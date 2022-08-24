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
				importItemRequests.emplace_back(Waapi::ImportItemRequest{importItem.path, importItem.originalsSubFolder, importItem.renderFilePath});

				auto pathWithoutObjectTypes = WwiseHelper::pathToPathWithoutObjectTypes(importItem.path);
				objectsInExtension.insert(pathWithoutObjectTypes);

				for(auto ancestorPath : WwiseHelper::pathToAncestorPaths(pathWithoutObjectTypes))
				{
					objectsInExtension.insert(ancestorPath);
				}
			}

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
							summary.objects[object.path].type = object.type;
							summary.objects[object.path].newlyCreated = false;
						}
					}
				};

				if(options.undoGroupFeatureEnabled)
					waapiClient.beginUndoGroup();

				juce::String objectLanguage = ImportTaskContants::defaultObjectLanguage;
				if(options.hierarchyMappingNodeList.back().type == Wwise::ObjectType::SoundVoice)
					objectLanguage = options.hierarchyMappingNodeList.back().language;

				auto importResponse = waapiClient.import(importItemRequests, options.containerNameExistsOption, objectLanguage);

				if(importResponse.status)
				{
					// Anything that is returned here was newly created
					for(const auto& object : importResponse.result)
					{
						summary.objectsCreated++;

						summary.objects[object.path].type = object.type;
						summary.objects[object.path].newlyCreated = true;

						// Audio file sources represent imported wav files
						if(object.type == Wwise::ObjectType::AudioFileSource)
						{
							summary.audioFilesImported++;

							summary.objects[object.path].originalWavFilePath = object.originalWavFilePath;
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

								if(it != depthToTemplatePropertyPathMap.end() && (options.applyTemplateOption == Import::ApplyTemplateOption::Always || object.newlyCreated))
								{
									propertyTemplatePathToObjectMapping[it->second].emplace_back(objectPath);
								}
							}

							for(const auto& [source, targets] : propertyTemplatePathToObjectMapping)
							{
								auto response = waapiClient.pasteProperties({source, targets});

								if(!response.status)
								{
									summary.errorMessage << juce::NewLine() << response.errorMessage;
								}
								else
								{
									for(const auto& target : targets)
									{
										summary.objectTemplatesApplied++;

										summary.objects[target].propertyTemplatePath = source;
									}
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
					summary.errorMessage << juce::NewLine() << importResponse.errorMessage;
				}
			}
			else
			{
				summary.errorMessage << juce::NewLine() << existingObjectsResponse.errorMessage;
			}

			if(options.undoGroupFeatureEnabled)
				waapiClient.endUndoGroup("Import and Apply Paste Properties");

			auto onCallAsync = [this, summary = summary]
			{
				callback(summary);
			};

			juce::MessageManager::callAsync(onCallAsync);
		}

	private:
		WaapiClient& waapiClient;
		Import::Task::Options options;
		Callback callback;
	};
} // namespace AK::WwiseTransfer
