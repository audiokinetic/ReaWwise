#pragma once

#include "Core/WaapiClient.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class WwiseProjectSupport
		: juce::ValueTree::Listener
	{
	public:
		WwiseProjectSupport(juce::ValueTree appState, WaapiClient& waapiClient);
		~WwiseProjectSupport();

	private:
		WaapiClient& waapiClient;

		juce::ValueTree applicationState;
		juce::ValueTree hierarchyMapping;
		juce::ValueTree languages;

		juce::CachedValue<bool> waapiConnected;
		juce::CachedValue<juce::String> projectPath;
		juce::CachedValue<juce::String> projectId;
		juce::CachedValue<bool> originalsFolderLookupEnabled;
		juce::CachedValue<juce::String> originalsFolder;
		juce::CachedValue<juce::String> languageSubfolder;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;

		void loadProjectInfo();
		void loadProjectLanguages();
		void loadOriginalsFolder();
		void updateLanguageSubpath();
	};
} // namespace AK::WwiseTransfer
