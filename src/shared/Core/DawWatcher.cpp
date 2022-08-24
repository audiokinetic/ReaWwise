#include "DawWatcher.h"

#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"
#include "Model/Import.h"
#include "Model/Waapi.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <optional>
#include <unordered_map>

namespace AK::WwiseTransfer
{
	struct PreviewOptions
	{
		Import::ContainerNameExistsOption containerNameExists;
		juce::String projectPath;
		juce::String originalsFolder;
		bool waqlEnabled;
		juce::String languageSubfolder;
	};

	DawWatcher::DawWatcher(juce::ValueTree appState, WaapiClient& waapiClient, DawContext& dawContext, int refreshInterval)
		: juce::Thread("DawWatcher")
		, applicationState(appState)
		, hierarchyMapping(appState.getChildWithName(IDs::hierarchyMapping))
		, importDestination(appState, IDs::importDestination, nullptr)
		, originalsSubfolder(appState, IDs::originalsSubfolder, nullptr)
		, containerNameExists(appState, IDs::containerNameExists, nullptr)
		, wwiseObjectsChanged(appState, IDs::wwiseObjectsChanged, nullptr)
		, previewLoading(appState, IDs::previewLoading, nullptr)
		, dawContext(dawContext)
		, waapiClient(waapiClient)
		, previewItems(appState.getChildWithName(IDs::previewItems))
		, refreshInterval(refreshInterval)
		, lastImportItemsFromDawHash(0)
		, sessionName(appState, IDs::sessionName, nullptr)
		, projectPath(appState, IDs::projectPath, nullptr)
		, originalsFolder(appState, IDs::originalsFolder, nullptr)
		, languageSubfolder(appState, IDs::languageSubfolder, nullptr)
	{
		auto featureSupport = appState.getChildWithName(IDs::featureSupport);
		waqlEnabled.referTo(featureSupport, IDs::sessionName, nullptr);

		applicationState.addListener(this);
	}

	DawWatcher::~DawWatcher()
	{
		applicationState.removeListener(this);
	}

	void DawWatcher::start()
	{
		startThread();
	}

	void DawWatcher::stop()
	{
		stopThread(-1);
	}

	void DawWatcher::run()
	{
		while(!threadShouldExit())
		{
			updateSessionName();

			std::optional<Import::Options> importOptions;
			std::optional<PreviewOptions> previewOptions;

			{
				juce::MessageManager::Lock l;

				auto hierarchyMappingPath = ImportHelper::hierarchyMappingToPath(ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping));

				importOptions = Import::Options(importDestination, originalsSubfolder, hierarchyMappingPath);
				previewOptions = PreviewOptions{containerNameExists, projectPath, originalsFolder, waqlEnabled, languageSubfolder};
			}

			if(importOptions && previewOptions)
			{
				auto importItemsFromDaw = dawContext.getImportItems(*importOptions);

				auto currentImportItemsFromDawHash = ImportHelper::importItemsToHash(importItemsFromDaw);

				if(lastImportItemsFromDawHash != currentImportItemsFromDawHash || previewOptionsChanged)
				{
					setPreviewLoading(true);
					previewOptionsChanged.store(false);

					lastImportItemsFromDawHash = currentImportItemsFromDawHash;

					std::set<juce::String> objectPaths;
					std::unordered_map<juce::String, juce::ValueTree> pathToValueTreeMapping;

					juce::ValueTree rootNode(IDs::previewItems);

					// Build tree based on import items and their ancestors
					for(const auto& importItem : importItemsFromDaw)
					{
						auto currentNode = rootNode;

						for(const auto& ancestorPath : WwiseHelper::pathToAncestorPaths(importItem.path))
						{
							auto pathWithoutType = WwiseHelper::pathToPathWithoutObjectTypes(ancestorPath);
							auto child = currentNode.getChildWithName(pathWithoutType);

							if(!child.isValid())
							{
								objectPaths.insert(pathWithoutType);

								auto name = WwiseHelper::pathToObjectName(pathWithoutType);
								auto type = WwiseHelper::pathToObjectType(ancestorPath);

								child = ImportHelper::previewItemToValueTree(pathWithoutType, {name, type, Import::ObjectStatus::New, "", Import::WavStatus::Unknown});

								currentNode.appendChild(child, nullptr);
								pathToValueTreeMapping[pathWithoutType] = child;
							}

							currentNode = child;
						}

						auto pathWithoutType = WwiseHelper::pathToPathWithoutObjectTypes(importItem.path);
						objectPaths.insert(pathWithoutType);

						auto name = WwiseHelper::pathToObjectName(importItem.path);
						auto type = WwiseHelper::pathToObjectType(importItem.path);

						auto originalsWav = previewOptions->languageSubfolder + juce::File::getSeparatorChar() +
						                    (importItem.originalsSubFolder.isNotEmpty() ? importItem.originalsSubFolder + juce::File::getSeparatorChar() : "") +
						                    juce::File(importItem.audioFilePath).getFileName();

						auto wavStatus = Import::WavStatus::Unknown;

						if(previewOptions->originalsFolder.isNotEmpty())
						{
							auto absoluteWavPath = juce::File(previewOptions->originalsFolder).getChildFile(originalsWav);

							if(absoluteWavPath.exists())
								wavStatus = Import::WavStatus::Replaced;
							else
								wavStatus = Import::WavStatus::New;
						}

						auto child = ImportHelper::previewItemToValueTree(pathWithoutType, {name, type, Import::ObjectStatus::New, originalsWav, wavStatus});

						currentNode.appendChild(child, nullptr);
						pathToValueTreeMapping[pathWithoutType] = child;
					}

					Waapi::Response<Waapi::ObjectResponseSet> response;

					if(importOptions->importDestination.isNotEmpty())
					{
						if(previewOptions->waqlEnabled)
							response = waapiClient.getObjectAncestorsAndDescendants(importOptions->importDestination);
						else
							response = waapiClient.getObjectAncestorsAndDescendantsLegacy(importOptions->importDestination);
					}

					if(response.status)
					{
						auto pathToValueTreeMappingLocal = pathToValueTreeMapping;

						// Update original tree with information from existing objects
						for(const auto& existingObject : response.result)
						{
							auto it = pathToValueTreeMappingLocal.find(existingObject.path);

							if(it != pathToValueTreeMappingLocal.end())
							{
								auto previewItem = ImportHelper::valueTreeToPreviewItem(it->second);

								if(existingObject.type != Wwise::ObjectType::Sound || previewOptions->containerNameExists == Import::ContainerNameExistsOption::UseExisting)
									previewItem.objectStatus = Import::ObjectStatus::NoChange;
								else if(previewOptions->containerNameExists == Import::ContainerNameExistsOption::Replace)
									previewItem.objectStatus = Import::ObjectStatus::Replaced;
								else if(previewOptions->containerNameExists == Import::ContainerNameExistsOption::CreateNew)
									previewItem.objectStatus = Import::ObjectStatus::NewRenamed;

								if(previewItem.type == Wwise::ObjectType::Unknown)
								{
									previewItem.type = existingObject.type;
								}

								it->second.copyPropertiesFrom(ImportHelper::previewItemToValueTree(existingObject.path, previewItem), nullptr);
							}
						}
					}

					// Update preview tree. This will cause the preview to update itself if there is new content
					auto onCallAsync = [this, rootNode = rootNode]
					{
						if(!previewItems.isEquivalentTo(rootNode))
							previewItems.copyPropertiesAndChildrenFrom(rootNode, nullptr);
					};

					juce::MessageManager::callAsync(onCallAsync);

					setPreviewLoading(false);
				}
			}

			wait(refreshInterval);
		}
	}

	void DawWatcher::setPreviewLoading(bool isPreviewLoading)
	{
		auto onCallAsync = [this, isPreviewLoading]
		{
			previewLoading = isPreviewLoading;
		};

		juce::MessageManager::callAsync(onCallAsync);
	}

	void DawWatcher::updateSessionName()
	{
		auto name = dawContext.getSessionName();

		auto updateSessionName = [this, name = name]
		{
			sessionName = name;
		};

		juce::MessageManager::callAsync(updateSessionName);
	}

	void DawWatcher::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		static std::initializer_list<juce::Identifier> properties{IDs::containerNameExists, IDs::projectPath, IDs::originalsFolder,
			IDs::wwiseObjectsChanged, IDs::waqlEnabled, IDs::languageSubfolder};

		if(treeWhosePropertyHasChanged == applicationState && std::find(properties.begin(), properties.end(), property) != properties.end())
		{
			previewOptionsChanged.store(true);

			if(property == IDs::wwiseObjectsChanged)
				wwiseObjectsChanged = false;
		}
	}
} // namespace AK::WwiseTransfer
