#pragma once

#include "Core/DawContext.h"
#include "WaapiClient.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class DawWatcher
		: private juce::Timer
		, public juce::AsyncUpdater
		, private juce::ValueTree::Listener
	{
	public:
		DawWatcher(juce::ValueTree appState, WaapiClient& waapiClient, DawContext& dawContext, int refreshInterval);
		~DawWatcher();

		void start();
		void stop();

	private:
		juce::ValueTree applicationState;
		juce::ValueTree hierarchyMapping;
		juce::ValueTree previewItems;

		juce::CachedValue<juce::String> importDestination;
		juce::CachedValue<juce::String> originalsSubfolder;
		juce::CachedValue<Import::ContainerNameExistsOption> containerNameExists;
		juce::CachedValue<bool> previewLoading;
		juce::CachedValue<juce::String> sessionName;
		juce::CachedValue<bool> waqlEnabled;
		juce::CachedValue<juce::String> projectPath;
		juce::CachedValue<juce::String> originalsFolder;
		juce::CachedValue<juce::String> languageSubfolder;
		juce::CachedValue<bool> waapiConnected;

		DawContext& dawContext;
		WaapiClient& waapiClient;

		unsigned int lastImportItemsHash;
		int refreshInterval;
		bool previewOptionsChanged;

		void timerCallback() override;
		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;
		void handleAsyncUpdate() override;
	};
} // namespace AK::WwiseTransfer
