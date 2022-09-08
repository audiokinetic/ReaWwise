#include "ReaperContext.h"

#include "Helpers/WwiseHelper.h"
#include "Model/Wwise.h"

#include <algorithm>
#include <cstdlib>
#include <map>
#include <memory>
#include <optional>

namespace AK::ReaWwise
{
	namespace ReaperContextConstants
	{
		constexpr int defaultBufferSize = 4096;
		const juce::String stateSizeKey = "stateSize";
		const juce::String stateKey = "state";
		const juce::String applicationKey = "ReaWwise";
	} // namespace ReaperContextConstants

	ReaperContext::ReaperContext(reaper_plugin_info_t* pluginInfo)
		: pluginInfo(pluginInfo)
		, defaultRenderDirectory(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("REAPER Media"))
		, defaultRenderPattern("untitled")
	{
		getMainHwnd = decltype(GetMainHwnd)(pluginInfo->GetFunc("GetMainHwnd"));
		showConsoleMsg = decltype(ShowConsoleMsg)(pluginInfo->GetFunc("ShowConsoleMsg"));
		addExtensionsMainMenu = decltype(AddExtensionsMainMenu)(pluginInfo->GetFunc("AddExtensionsMainMenu"));
		enumProjects = decltype(EnumProjects)(pluginInfo->GetFunc("EnumProjects"));
		getSetProjectInfo_String = decltype(GetSetProjectInfo_String)(pluginInfo->GetFunc("GetSetProjectInfo_String"));
		getProjectName = decltype(GetProjectName)(pluginInfo->GetFunc("GetProjectName"));
		resolveRenderPattern = decltype(ResolveRenderPattern)(pluginInfo->GetFunc("ResolveRenderPattern"));
		enumProjectMarkers2 = decltype(EnumProjectMarkers2)(pluginInfo->GetFunc("EnumProjectMarkers2"));
		enumRegionRenderMatrix = decltype(EnumRegionRenderMatrix)(pluginInfo->GetFunc("EnumRegionRenderMatrix"));
		getParentTrack = decltype(GetParentTrack)(pluginInfo->GetFunc("GetParentTrack"));
		getTrackName = decltype(GetTrackName)(pluginInfo->GetFunc("GetTrackName"));
		main_OnCommand = decltype(Main_OnCommand)(pluginInfo->GetFunc("Main_OnCommand"));
		getProjectPathEx = decltype(GetProjectPathEx)(pluginInfo->GetFunc("GetProjectPathEx"));
		showMessageBox = decltype(ShowMessageBox)(pluginInfo->GetFunc("ShowMessageBox"));
		getProjExtState = decltype(GetProjExtState)(pluginInfo->GetFunc("GetProjExtState"));
		setProjExtState = decltype(SetProjExtState)(pluginInfo->GetFunc("SetProjExtState"));
		markProjectDirty = decltype(MarkProjectDirty)(pluginInfo->GetFunc("MarkProjectDirty"));
	}

	juce::String ReaperContext::getSessionName()
	{
		auto projectInfo = getProjectInfo();
		return projectInfo.projectPath;
	}

	bool ReaperContext::saveState(juce::ValueTree applicationState)
	{
		using namespace ReaperContextConstants;

		auto projectInfo = getProjectInfo();

		const auto applicationStateString = applicationState.toXmlString();
		const auto applicationStateStringSize = juce::String(applicationStateString.getNumBytesAsUTF8());
		if(setProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateSizeKey.toUTF8(), applicationStateStringSize.toUTF8()) &&
			setProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateKey.toUTF8(), applicationStateString.toUTF8()))
		{
			markProjectDirty(projectInfo.projectReference);
			return true;
		}

		return false;
	}

	juce::ValueTree ReaperContext::retrieveState()
	{
		using namespace ReaperContextConstants;

		auto projectInfo = getProjectInfo();

		std::string buffer(defaultBufferSize, '\0');
		getProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateSizeKey.toUTF8(), &buffer[0], buffer.size());

		const auto stateSize = std::strtoll(&buffer[0], nullptr, 10);
		if(stateSize == 0)
			return {};

		buffer.resize(stateSize);
		if(getProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateKey.toUTF8(), &buffer[0], buffer.size()))
			return juce::ValueTree::fromXml(buffer);

		return {};
	}

	int ReaperContext::callerVersion()
	{
		return pluginInfo->caller_version;
	}

	int ReaperContext::registerFunction(const char* command, void* function)
	{
		return pluginInfo->Register(command, function);
	}

	std::vector<juce::String> ReaperContext::splitDoubleNullTerminatedString(const char* buffer)
	{
		std::vector<juce::String> result;

		for(const char* current = buffer; current && *current; current += result.back().length() + 1)
		{
			result.emplace_back(current);
		}

		return result;
	}

	bool ReaperContext::isValid()
	{
		if(getMainHwnd &&
			showConsoleMsg &&
			addExtensionsMainMenu &&
			enumProjects &&
			getSetProjectInfo_String &&
			getProjectName &&
			resolveRenderPattern &&
			enumProjectMarkers2 &&
			enumRegionRenderMatrix &&
			getParentTrack &&
			getTrackName &&
			main_OnCommand &&
			getProjectPathEx &&
			showMessageBox &&
			getProjExtState &&
			setProjExtState &&
			markProjectDirty)
			return true;

		return false;
	}

	void ReaperContext::renderItems()
	{
		main_OnCommand(42230, 0);
	}

	std::vector<WwiseTransfer::Import::Item> ReaperContext::getItemsForImport(const WwiseTransfer::Import::Options& options)
	{
		std::vector<WwiseTransfer::Import::Item> importItems;

		auto importItemsForPreview = getItemsForPreview(options);
		if(importItemsForPreview.size() == 0)
			return importItems;

		auto projectInfo = getProjectInfo();

		// In the following section, we retrieve render stats from reaper. We need to estimate the size of the buffer so that
		// no heap corruption occurs when reaper tries to write to the buffer we give it. Unfortunately, there is no way
		// for us to know the exact size of this buffer should be. And, it seems like reaper blindly writes to this buffer
		// as we can not send it a buffer size.

		// Assuming that volume levels are in [-1000, 1000], this is the max length for an entry for one render would be:
		// FILE:{audioFilePath};PEAK:-0000.000000;LRA:-0000.000000;LUFSMMAX:-0000.000000;LUFSSMAX:-0000.000000;LUFSI:-0000.000000;
		// This is roughly 105 chars + the audioFileName
		// Lets use 150 for extra chars just to leave some room for error.
		const int extra = 150;

		juce::String renderDirectory = getProjectString(projectInfo.projectReference, "RENDER_FILE");

		int totalBufferSize = (renderDirectory.length() + projectInfo.projectPath.length() + extra) * static_cast<int>(importItemsForPreview.size());
		for(const auto& item : importItemsForPreview)
		{
			totalBufferSize += item.audioFilePath.length();
		}

		juce::StringArray temp;
		juce::String renderStats = getProjectString(projectInfo.projectReference, "RENDER_STATS", totalBufferSize);
		temp.addTokens(renderStats, ";", "");

		juce::StringArray finalRenderPaths;

		// Not all items in here are paths. Paths are prefixed with "FILE:"
		for(auto item : temp)
		{
			if(item.startsWith("FILE:"))
				finalRenderPaths.add(item.trimCharactersAtStart("FILE:"));
		}

		for(int i = 0; i < importItemsForPreview.size(); ++i)
		{
			importItems.push_back({importItemsForPreview[i].path, importItemsForPreview[i].originalsSubFolder, importItemsForPreview[i].audioFilePath, finalRenderPaths[i]});
		}

		return importItems;
	}

	std::vector<WwiseTransfer::Import::PreviewItem> ReaperContext::getItemsForPreview(const WwiseTransfer::Import::Options& options)
	{
		auto projectInfo = getProjectInfo();

		auto renderDirectoryPath = getProjectString(projectInfo.projectReference, "RENDER_FILE");
		auto renderPattern = getProjectString(projectInfo.projectReference, "RENDER_PATTERN");

		// There are several scenarios where the render pattern could be empty
		// 1. When the project hasn't been saved (reaper uses "untitled")
		// 2. When the project has been saved (reaper uses the project name)
		if(renderPattern.isEmpty())
		{
			if(projectInfo.projectPath.isEmpty())
			{
				renderPattern = defaultRenderPattern;
			}
			else
			{
				renderPattern = projectInfo.projectName;
			}
		}

		juce::File renderDirectory;
		if(juce::File::isAbsolutePath(renderDirectoryPath))
		{
			renderDirectory = juce::File(renderDirectoryPath);
		}
		else if(projectInfo.projectPath.isNotEmpty())
		{
			renderDirectory = juce::File(projectInfo.projectPath).getParentDirectory().getChildFile(renderDirectoryPath);
		}
		else
		{
			// If the project wasnt saved, reaper uses a general render directory
			renderDirectory = defaultRenderDirectory.getChildFile(renderDirectoryPath);
		}

		const auto originalsSubfolderPathPart = options.originalsSubfolder.isNotEmpty() ? options.originalsSubfolder + juce::File::getSeparatorString() : "";
		const auto resolvedRenderPaths = getItemListFromRenderPattern(projectInfo.projectReference, renderPattern, false);
		const auto resolvedRenderPathsWithOriginalsSubfolder = getItemListFromRenderPattern(projectInfo.projectReference, originalsSubfolderPathPart + renderPattern, false);
		if(resolvedRenderPaths.size() != resolvedRenderPathsWithOriginalsSubfolder.size())
		{
			juce::Logger::writeToLog("Reaper: Mismatch between resolvedRenderPaths and resolvedRenderPathsWithOriginalsSubfolder");
			return {};
		}

		const auto resolvePattern = options.importDestination + options.hierarchyMappingPath;
		std::vector<juce::String> resolvedObjectPaths = getItemListFromRenderPattern(projectInfo.projectReference, resolvePattern, false);
		if(resolvedObjectPaths.size() != resolvedRenderPaths.size() || resolvedObjectPaths.size() != resolvedRenderPathsWithOriginalsSubfolder.size())
		{
			juce::Logger::writeToLog("Reaper: Mismatch between resolvedObjectPaths, resolvedRenderPaths and resolvedRenderPathsWithOriginalsSubfolder");
			return {};
		}

		std::vector<WwiseTransfer::Import::PreviewItem> importItems;
		for(int i = 0; i < resolvedObjectPaths.size(); ++i)
		{
			const auto objectPath = resolvedObjectPaths[i].upToLastOccurrenceOf(".", false, false);
			const auto resolvedRenderPath = renderDirectory.getChildFile(resolvedRenderPaths[i]).getFullPathName();
			const auto resolvedRenderPathWithOriginalsSubfolder = renderDirectory.getChildFile(resolvedRenderPathsWithOriginalsSubfolder[i]);
			const auto parentDirectory = resolvedRenderPathWithOriginalsSubfolder.getParentDirectory().getRelativePathFrom(renderDirectory);
			const auto originalsSubfolder = parentDirectory == "." ? "" : parentDirectory;
			importItems.push_back({objectPath, originalsSubfolder, resolvedRenderPath});
		}

		return importItems;
	}

	juce::String ReaperContext::getProjectString(ReaProject* project, const char* key, int bufferSize)
	{
		// getSetProjectInfo_String is unsafe since we cannot specify the size of the buffer that it must fill.
		// There are a few cases where we are unable to predict the size required, so we use a large value and hope for the best.
		std::string buffer((std::max)(bufferSize, ReaperContextConstants::defaultBufferSize), '\0');
		if(!getSetProjectInfo_String(project, key, &buffer[0], false))
			return {};

		return buffer;
	}

	ProjectInfo ReaperContext::getProjectInfo()
	{
		std::string buffer(ReaperContextConstants::defaultBufferSize, '\0');

		// The buffer sent to enumProjects will contain the project path.
		auto projectReference = enumProjects(-1, &buffer[0], buffer.size());
		if(!projectReference || buffer.empty())
			return {};

		const juce::File projectFile(buffer);

		// REAPER requires that a project file ends with the .rpp (case insensitive) file extension.
		if(projectFile.getFileExtension().compareIgnoreCase(".rpp") != 0)
			return {};

		return {
			projectReference,
			projectFile.getFileNameWithoutExtension(),
			projectFile.getFullPathName()
		};
	}

	std::vector<juce::String> ReaperContext::getItemListFromRenderPattern(ReaProject* project, const juce::String& pattern, bool suppressIllegalPaths)
	{
		int bufferLength = resolveRenderPattern(project, suppressIllegalPaths ? "" : nullptr, pattern.toUTF8(), nullptr, 0);
		std::string buffer(bufferLength, '\0');

		resolveRenderPattern(project, suppressIllegalPaths ? "" : nullptr, pattern.toUTF8(), &buffer[0], bufferLength);
		return splitDoubleNullTerminatedString(&buffer[0]);
	}
} // namespace AK::ReaWwise
