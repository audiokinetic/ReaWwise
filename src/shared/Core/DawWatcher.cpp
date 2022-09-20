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
		: applicationState(appState)
		, hierarchyMapping(appState.getChildWithName(IDs::hierarchyMapping))
		, previewItems(appState.getChildWithName(IDs::previewItems))
		, importDestination(appState, IDs::importDestination, nullptr)
		, originalsSubfolder(appState, IDs::originalsSubfolder, nullptr)
		, containerNameExists(appState, IDs::containerNameExists, nullptr)
		, previewLoading(appState, IDs::previewLoading, nullptr)
		, sessionName(appState, IDs::sessionName, nullptr)
		, projectPath(appState, IDs::projectPath, nullptr)
		, originalsFolder(appState, IDs::originalsFolder, nullptr)
		, languageSubfolder(appState, IDs::languageSubfolder, nullptr)
		, waapiConnected(appState, IDs::waapiConnected, nullptr)
		, dawContext(dawContext)
		, waapiClient(waapiClient)
		, lastImportItemsHash(0)
		, refreshInterval(refreshInterval)
		, previewOptionsChanged(false)
	{
		auto featureSupport = appState.getChildWithName(IDs::featureSupport);
		waqlEnabled.referTo(featureSupport, IDs::waqlEnabled, nullptr);

		applicationState.addListener(this);
	}

	DawWatcher::~DawWatcher()
	{
		applicationState.removeListener(this);
	}

	void DawWatcher::start()
	{
		startTimer(refreshInterval);
	}

	void DawWatcher::stop()
	{
		stopTimer();
	}

	void DawWatcher::timerCallback()
	{
		sessionName = dawContext.getSessionName();

		if(dawContext.sessionChanged())
		{
			triggerAsyncUpdate();
		}
	}

	void DawWatcher::handleAsyncUpdate()
	{
		const auto hierarchyMappingPath = ImportHelper::hierarchyMappingToPath(ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping));
		const auto importItems = dawContext.getItemsForPreview({importDestination, originalsSubfolder, hierarchyMappingPath});

		const auto importItemsHash = ImportHelper::importPreviewItemsToHash(importItems);

		if(importItemsHash != lastImportItemsHash || previewOptionsChanged)
		{
			previewOptionsChanged = false;

			std::set<juce::String> objectPaths;
			std::unordered_map<juce::String, juce::ValueTree> pathToValueTreeMapping;

			juce::ValueTree rootNode(IDs::previewItems);

			// Build tree based on import items and their ancestors
			for(const auto& importItem : importItems)
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

						child = ImportHelper::previewItemNodeToValueTree(pathWithoutType, {name, type, Import::ObjectStatus::New, "", Import::WavStatus::Unknown});

						currentNode.appendChild(child, nullptr);
						pathToValueTreeMapping[pathWithoutType] = child;
					}

					currentNode = child;
				}

				auto pathWithoutType = WwiseHelper::pathToPathWithoutObjectTypes(importItem.path);
				objectPaths.insert(pathWithoutType);

				auto name = WwiseHelper::pathToObjectName(importItem.path);
				auto type = WwiseHelper::pathToObjectType(importItem.path);

				auto originalsWav = languageSubfolder + juce::File::getSeparatorChar() +
				                    (importItem.originalsSubFolder.isNotEmpty() ? importItem.originalsSubFolder + juce::File::getSeparatorChar() : "") +
				                    juce::File(importItem.audioFilePath).getFileName();

				auto wavStatus = Import::WavStatus::Unknown;

				if(originalsFolder.get().isNotEmpty())
				{
					auto absoluteWavPath = juce::File(originalsFolder).getChildFile(originalsWav);

					if(absoluteWavPath.exists())
						wavStatus = Import::WavStatus::Replaced;
					else
						wavStatus = Import::WavStatus::New;
				}

				auto child = ImportHelper::previewItemNodeToValueTree(pathWithoutType, {name, type, Import::ObjectStatus::New, originalsWav, wavStatus});

				currentNode.appendChild(child, nullptr);
				pathToValueTreeMapping[pathWithoutType] = child;
			}

			auto onGetObjectAncestorsAndDescendants = [this, pathToValueTreeMapping, rootNode](const Waapi::Response<Waapi::ObjectResponseSet>& response)
			{
				if(response.status)
				{
					// Update original tree with information from existing objects
					for(const auto& existingObject : response.result)
					{
						auto it = pathToValueTreeMapping.find(existingObject.path);

						if(it != pathToValueTreeMapping.end())
						{
							auto previewItem = ImportHelper::valueTreeToPreviewItemNode(it->second);

							if(existingObject.type != Wwise::ObjectType::Sound || containerNameExists == Import::ContainerNameExistsOption::UseExisting)
								previewItem.objectStatus = Import::ObjectStatus::NoChange;
							else if(containerNameExists == Import::ContainerNameExistsOption::Replace)
								previewItem.objectStatus = Import::ObjectStatus::Replaced;
							else if(containerNameExists == Import::ContainerNameExistsOption::CreateNew)
								previewItem.objectStatus = Import::ObjectStatus::NewRenamed;

							if(previewItem.type == Wwise::ObjectType::Unknown)
							{
								previewItem.type = existingObject.type;
							}

							juce::ValueTree localCopy = it->second;
							localCopy.copyPropertiesFrom(ImportHelper::previewItemNodeToValueTree(existingObject.path, previewItem), nullptr);
						}
					}
				}

				if(!previewItems.isEquivalentTo(rootNode))
					previewItems.copyPropertiesAndChildrenFrom(rootNode, nullptr);

				previewLoading = false;
			};

			if(waapiConnected.get() && importDestination.get().isNotEmpty())
			{
				previewLoading = true;

				if(waqlEnabled)
					waapiClient.getObjectAncestorsAndDescendantsAsync(importDestination, onGetObjectAncestorsAndDescendants);
				else
					waapiClient.getObjectAncestorsAndDescendantsLegacyAsync(importDestination, onGetObjectAncestorsAndDescendants);
			}
			else
			{
				Waapi::Response<Waapi::ObjectResponseSet> emptyResponse;
				onGetObjectAncestorsAndDescendants(emptyResponse);
			}
		}
	}

	void DawWatcher::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		static std::initializer_list<juce::Identifier> properties{IDs::containerNameExists, IDs::projectPath, IDs::originalsFolder,
			IDs::wwiseObjectsChanged, IDs::waqlEnabled, IDs::languageSubfolder, IDs::originalsFolder, IDs::importDestination, IDs::originalsSubfolder, IDs::waapiConnected};

		if(treeWhosePropertyHasChanged == applicationState && std::find(properties.begin(), properties.end(), property) != properties.end() ||
			treeWhosePropertyHasChanged.getType() == IDs::hierarchyMappingNode)
		{
			// Used to notify the next update iteration that the preview may contain items that have changed that would not be reflected
			// in the importItems hash. Perhaps we can include these items in the hash in the future.
			previewOptionsChanged = true;

			if(property == IDs::wwiseObjectsChanged)
				applicationState.setPropertyExcludingListener(this, IDs::wwiseObjectsChanged, false, nullptr);

			triggerAsyncUpdate();
		}
	}

	void DawWatcher::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		juce::ignoreUnused(childWhichHasBeenAdded);

		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void DawWatcher::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		juce::ignoreUnused(childWhichHasBeenRemoved, indexFromWhichChildWasRemoved);

		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void DawWatcher::valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
	{
		juce::ignoreUnused(oldIndex, newIndex);

		if(parentTreeWhoseChildrenHaveMoved.getType() == IDs::hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}
} // namespace AK::WwiseTransfer
