#pragma once

#include "Core/DawContext.h"
#include "WaapiClient.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class DawWatcher
		: private juce::Thread
		, private juce::ValueTree::Listener
	{
	public:
		DawWatcher(juce::ValueTree appState, WaapiClient& waapiClient, DawContext& dawContext, int refreshInterval);
		~DawWatcher();

		void start();
		void stop();

		juce::ValueTree hierarchyMapping;
		juce::CachedValue<juce::String> importDestination;
		juce::CachedValue<juce::String> originalsSubfolder;
		juce::CachedValue<Import::ContainerNameExistsOption> containerNameExists;
		juce::CachedValue<bool> wwiseObjectsChanged;
		juce::CachedValue<bool> previewLoading;
		juce::CachedValue<juce::String> sessionName;
		juce::CachedValue<bool> waqlEnabled;
		juce::CachedValue<juce::String> projectPath;
		juce::CachedValue<juce::String> originalsFolder;
		juce::CachedValue<juce::String> languageSubfolder;

		juce::ValueTree previewItems;

		unsigned int lastImportItemsFromDawHash;
		Import::ContainerNameExistsOption lastContainerNameExists;
		juce::String lastProjectPath;

	private:
		juce::ValueTree applicationState;

		std::atomic<bool> previewOptionsChanged;

		DawContext& dawContext;
		WaapiClient& waapiClient;
		int refreshInterval;

		void run() override;
		void setPreviewLoading(bool isPreviewLoading);
		void updateSessionName();
		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
	};
} // namespace AK::WwiseTransfer
