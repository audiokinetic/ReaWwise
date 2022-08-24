#include "FeatureSupport.h"

#include "Helpers/WwiseHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer
{
	FeatureSupport::FeatureSupport(juce::ValueTree appState, WaapiClient& waapiClient)
		: applicationState(appState)
		, version(applicationState.getChildWithName(IDs::version))
		, waapiClient(waapiClient)
	{
		auto featureSupport = applicationState.getChildWithName(IDs::featureSupport);
		selectObjectsOnImportCommand.referTo(featureSupport, IDs::selectObjectsOnImportCommand, nullptr);
		applyTemplateFeatureEnabled.referTo(featureSupport, IDs::applyTemplateFeatureEnabled, nullptr);
		undoGroupFeatureEnabled.referTo(featureSupport, IDs::undoGroupFeatureEnabled, nullptr);
		waqlEnabled.referTo(featureSupport, IDs::waqlEnabled, nullptr);
		originalsFolderLookupEnabled.referTo(featureSupport, IDs::originalsFolderLookupEnabled, nullptr);

		applicationState.addListener(this);
	}

	FeatureSupport::~FeatureSupport()
	{
		applicationState.removeListener(this);
	}

	void FeatureSupport::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
		const juce::Identifier& property)
	{
		if(treeWhosePropertyHasChanged.getType() == IDs::application && property == IDs::waapiConnected)
		{
			auto onGetVersionAsync = [this](const auto& response)
			{
				using namespace FeatureSupportConstants;

				auto versionValueTree = WwiseHelper::versionToValueTree(response.result);

				version.copyPropertiesAndChildrenFrom(versionValueTree, nullptr);

				selectObjectsOnImportCommand = response.result >= v2022_1_0_0 ? FindInProjectExplorerSelectionChannel1 : FindInProjectExplorerSyncGroup1;
				applyTemplateFeatureEnabled = response.result >= v2022_1_0_0;
				undoGroupFeatureEnabled = response.result >= v2021_1_10_0;
				waqlEnabled = response.result >= v2021_1_0_0;
				originalsFolderLookupEnabled = response.result >= v2022_1_0_0;
			};

			waapiClient.getVersionAsync(onGetVersionAsync);
		}
	}
} // namespace AK::WwiseTransfer
