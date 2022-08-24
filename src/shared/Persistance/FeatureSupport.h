#pragma once

#include "Core/WaapiClient.h"
#include "Model/Wwise.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	namespace FeatureSupportConstants
	{
		const juce::String FindInProjectExplorerSelectionChannel1 = "FindInProjectExplorerSelectionChannel1";
		const juce::String FindInProjectExplorerSyncGroup1 = "FindInProjectExplorerSyncGroup1";
		const Wwise::Version v2022_1_0_0 = {2022, 1, 0, 0};
		const Wwise::Version v2021_1_0_0 = {2021, 1, 0, 0};
		const Wwise::Version v2021_1_10_0 = {2021, 1, 10, 0};
	} // namespace FeatureSupportConstants

	class FeatureSupport
		: juce::ValueTree::Listener
	{
	public:
		FeatureSupport(juce::ValueTree appState, WaapiClient& waapiClient);
		~FeatureSupport();

	private:
		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
			const juce::Identifier& property) override;

		WaapiClient& waapiClient;

		juce::ValueTree applicationState;
		juce::ValueTree version;

		juce::ValueTree featureSupport;
		juce::CachedValue<juce::String> selectObjectsOnImportCommand;
		juce::CachedValue<bool> applyTemplateFeatureEnabled;
		juce::CachedValue<bool> undoGroupFeatureEnabled;
		juce::CachedValue<bool> waqlEnabled;
		juce::CachedValue<bool> originalsFolderLookupEnabled;
	};
} // namespace AK::WwiseTransfer
