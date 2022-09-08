#pragma once

#include "Core/DawContext.h"
#include "Model/Import.h"

#include <reaper_plugin_functions.h>

namespace AK::ReaWwise
{
	struct ProjectInfo
	{
		ReaProject* projectReference;
		juce::String projectName;
		juce::String projectPath;
	};

	class ReaperContext
		: public WwiseTransfer::DawContext
	{
	public:
		ReaperContext(reaper_plugin_info_t* pluginInfo);

		juce::String getSessionName() override;
		bool saveState(juce::ValueTree applicationState) override;
		juce::ValueTree retrieveState() override;
		void renderItems() override;
		std::vector<WwiseTransfer::Import::PreviewItem> getItemsForPreview(const WwiseTransfer::Import::Options& options) override;
		std::vector<WwiseTransfer::Import::Item> getItemsForImport(const WwiseTransfer::Import::Options& options) override;

		// Reaper api for public use
		bool isValid();
		int callerVersion();
		decltype(GetMainHwnd) getMainHwnd;
		decltype(ShowConsoleMsg) showConsoleMsg;
		decltype(AddExtensionsMainMenu) addExtensionsMainMenu;
		decltype(EnumProjects) enumProjects;
		decltype(GetSetProjectInfo_String) getSetProjectInfo_String;
		decltype(GetProjectName) getProjectName;
		decltype(ResolveRenderPattern) resolveRenderPattern;
		decltype(EnumProjectMarkers2) enumProjectMarkers2;
		decltype(EnumRegionRenderMatrix) enumRegionRenderMatrix;
		decltype(GetParentTrack) getParentTrack;
		decltype(GetTrackName) getTrackName;
		decltype(Main_OnCommand) main_OnCommand;
		decltype(GetProjectPathEx) getProjectPathEx;
		decltype(ShowMessageBox) showMessageBox;
		decltype(GetProjExtState) getProjExtState;
		decltype(SetProjExtState) setProjExtState;
		decltype(MarkProjectDirty) markProjectDirty;
		int registerFunction(const char* command, void* function);

	private:
		std::vector<juce::String> splitDoubleNullTerminatedString(const char*);
		std::vector<juce::String> getItemListFromRenderPattern(ReaProject* project, const juce::String& pattern, bool suppressIllegalPaths = true);
		juce::String getProjectString(ReaProject* project, const char* key, int bufferSize = 0);
		ProjectInfo getProjectInfo();

		juce::File defaultRenderDirectory;
		juce::String defaultRenderPattern;

		reaper_plugin_info_t* pluginInfo;
	};
} // namespace AK::ReaWwise
