/*----------------------------------------------------------------------------------------

Copyright (c) 2023 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#pragma once

#include "Helpers/ImportHelper.h"
#include "Model/Import.h"
#include "WaapiClient.h"

#include <vector>

namespace AK::WwiseTransfer
{
	namespace ImportTaskContants
	{
		const juce::String defaultObjectLanguage = "SFX";
	}

	template <class Callback>
	class ImportTask : public juce::ThreadWithProgressWindow
	{
	public:
		ImportTask(WaapiClient& waapiClient, const Import::Task::Options& options, Callback callback)
			: juce::ThreadWithProgressWindow("Importing...", true, false)
			, waapiClient(waapiClient)
			, options(options)
			, callback(callback)
		{
			// Set progress to -1 one to show infinite progress bar
			setProgress(-1);
		}

		void run() override
		{
			using namespace AK::WwiseAuthoringAPI;

			using ObjectPath = juce::String;
			using PropertyTemplatePath = juce::String;

			Import::Summary summary;

			auto containerNameExistsOptionAsString = ImportHelper::containerNameExistsOptionToString(options.containerNameExistsOption);

			std::vector<Waapi::ImportItemRequest> importItemRequests;

			// Holds all paths to objects defined in the extension (including ancestors)
			std::set<ObjectPath> objectsInExtension;

			for(const auto& importItem : options.importItems)
			{
				if(WwiseHelper::isPathComplete(importItem.path))
				{
					importItemRequests.emplace_back(Waapi::ImportItemRequest{importItem.path, importItem.originalsSubFolder, importItem.renderFilePath});

					auto pathWithoutObjectTypes = WwiseHelper::pathToPathWithoutObjectTypes(importItem.path);
					objectsInExtension.insert(pathWithoutObjectTypes);

					for(const auto& ancestorPath : WwiseHelper::pathToAncestorPaths(pathWithoutObjectTypes))
						objectsInExtension.insert(ancestorPath);
				}
				else
					juce::Logger::writeToLog("File with incomplete object path " + importItem.path + " will not be imported.");
			}

			if(!importItemRequests.empty())
			{
				// Will be eventullay compared to the results of the import to figure out what was newly created
				Waapi::Response<Waapi::ObjectResponseSet> existingObjectsResponse;

				if(options.waqlEnabled)
					existingObjectsResponse = waapiClient.getObjectAncestorsAndDescendants(options.importDestination);
				else
					existingObjectsResponse = waapiClient.getObjectAncestorsAndDescendantsLegacy(options.importDestination);

				if(existingObjectsResponse.status)
				{
					for(const auto& object : existingObjectsResponse.result)
					{
						// We only care about objects defined in the extension
						auto it = objectsInExtension.find(object.path);

						if(it != objectsInExtension.end())
						{
							// Create an entry in the summary data structure (It will eventually be used to produce the import summary page)
							// Ignore sounds that would be newly created. We need to get their paths from the import response because they may change.
							if(options.containerNameExistsOption != Import::ContainerNameExistsOption::CreateNew || object.type != Wwise::ObjectType::Sound)
							{
								auto& summaryObject = summary.objects[object.path];
								summaryObject.type = object.type;
								summaryObject.id = object.id;
							}
						}
					}

					const ScopedUndoGroup scopedundogroup(waapiClient, options.undoGroupFeatureEnabled);

					juce::String objectLanguage = ImportTaskContants::defaultObjectLanguage;
					if(options.hierarchyMappingNodeList.back().type == Wwise::ObjectType::SoundVoice)
						objectLanguage = options.hierarchyMappingNodeList.back().language;

					// Check if wav files will be replaced. Only works if we have the originals folder.
					// Basically checks to see if the audio file is already present in the originals folder.
					std::set<juce::String> existingAudioFiles;

					if(options.originalsFolder.isNotEmpty())
					{
						for(const auto& importItemRequest : importItemRequests)
						{
							// Build the final file path
							auto pathInWwise = options.originalsFolder + options.languageSubfolder + juce::File::getSeparatorString() +
							                   (importItemRequest.originalsSubFolder.isNotEmpty() ? importItemRequest.originalsSubFolder + juce::File::getSeparatorString() : "") +
							                   juce::File(importItemRequest.renderFilePath).getFileName();

							if(juce::File(pathInWwise).exists())
								existingAudioFiles.emplace(pathInWwise);

							for(const auto& existingObject : existingObjectsResponse.result)
							{
								if(existingObject.originalWavFilePath == pathInWwise)
								{
									auto& summaryObject = summary.objects[existingObject.path];
									summaryObject.id = existingObject.id;
									summaryObject.originalWavFilePath = pathInWwise;
									summaryObject.wavStatus = Import::WavStatus::Replaced;
									summaryObject.type = existingObject.type;
								}
							}
						}
					}

					auto importResponse = waapiClient.import(importItemRequests, options.containerNameExistsOption, objectLanguage);

					if(importResponse.status)
					{
						// Result will include newly created and existing (affected) objects
						for(const auto& object : importResponse.result)
						{
							// Check against existing objects to see if object was truely newly created
							auto it = summary.objects.find(object.path);

							if(it == summary.objects.end())
							{
								auto& summaryObject = summary.objects[object.path];
								summaryObject.objectStatus = Import::ObjectStatus::New;
								summaryObject.type = object.type;
								summaryObject.originalWavFilePath = object.originalWavFilePath;

								if(object.originalWavFilePath.isNotEmpty())
								{
									// Some objects are associated with an originalWavFilePath that may have been replaced.
									auto it = existingAudioFiles.find(object.originalWavFilePath);
									if(it != existingAudioFiles.end())
									{
										summary.objects[object.path].wavStatus = Import::WavStatus::Replaced;
									}
									else
									{
										summary.objects[object.path].wavStatus = Import::WavStatus::New;
									}
								}
							}
							// If the object was found but the id is different, it was replaced
							else if(it->second.id != object.id)
							{
								summary.objects[object.path].objectStatus = Import::ObjectStatus::Replaced;
							}
						}

						// Applying templates only works in custom hierarchy mapping mode
						if(options.applyTemplateFeatureEnabled)
						{
							// Will store node depth in relation to template property path
							std::map<int, juce::String> depthToTemplatePropertyPathMap;

							int importDestinationDepth = WwiseHelper::pathToPathParts(options.importDestination).size();

							for(int i = 0; i < options.hierarchyMappingNodeList.size(); ++i)
							{
								const auto& hierarchyMappingNode = options.hierarchyMappingNodeList[i];

								if(hierarchyMappingNode.propertyTemplatePath.isNotEmpty() &&
									hierarchyMappingNode.propertyTemplatePathEnabled &&
									hierarchyMappingNode.propertyTemplatePathValid)
								{
									depthToTemplatePropertyPathMap[importDestinationDepth + i + 1] = hierarchyMappingNode.propertyTemplatePath;
								}
							}

							if(depthToTemplatePropertyPathMap.size() > 0)
							{
								std::map<PropertyTemplatePath, std::vector<ObjectPath>> propertyTemplatePathToObjectMapping;

								// Go through all objects and if their depth matches a hierarchy node that has a template defined in it, add it to propertyTemplatePathToObjectMapping
								// We will eventually use this map to submit paste property requests in waaapi
								for(const auto& [objectPath, object] : summary.objects)
								{
									const int depth = WwiseHelper::pathToPathParts(objectPath).size();

									auto it = depthToTemplatePropertyPathMap.find(depth);

									if(it != depthToTemplatePropertyPathMap.end() && (options.applyTemplateOption == Import::ApplyTemplateOption::Always || object.objectStatus == Import::ObjectStatus::New))
										propertyTemplatePathToObjectMapping[it->second].emplace_back(objectPath);
								}

								for(const auto& [source, targets] : propertyTemplatePathToObjectMapping)
								{
									const auto response = waapiClient.pasteProperties({source, targets});
									if(!response.status)
									{
										summary.errors.push_back(response.error);
									}
									else
									{
										for(const auto& target : targets)
											summary.objects[target].propertyTemplatePath = source;
									}
								}
							}
						}

						if(importResponse.result.size() > 0)
						{
							std::vector<juce::String> importedObjectPaths;

							// Assumes that importResponse.result is sorted by path
							auto first = importResponse.result.begin()->path;
							auto last = importResponse.result.rbegin()->path;

							importedObjectPaths.emplace_back(WwiseHelper::getCommonAncestor(first, last));

							waapiClient.selectObjects(options.selectObjectsOnImportCommand, importedObjectPaths);
						}
					}
					else
					{
						summary.errors.push_back(importResponse.error);
					}
				}
				else
				{
					summary.errors.push_back(existingObjectsResponse.error);
				}
			}
			else
				juce::Logger::writeToLog("Import items list was empty. Nothing to import.");

			auto onCallAsync = [this, summary = summary]
			{
				callback(summary);
			};

			juce::MessageManager::callAsync(onCallAsync);
		}

	private:
		struct ScopedUndoGroup final
		{
			WaapiClient& waapiClient;
			bool enabled{false};

			ScopedUndoGroup(WaapiClient& waapiClient, bool enable)
				: waapiClient(waapiClient)
				, enabled(enable)
			{
				if(enable)
					waapiClient.beginUndoGroup();
			}

			~ScopedUndoGroup()
			{
				if(enabled)
					waapiClient.endUndoGroup("Import and Apply Paste Properties");
			}
		};

		WaapiClient& waapiClient;
		Import::Task::Options options;
		Callback callback;
	};
} // namespace AK::WwiseTransfer
