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

	struct RenderInfo
	{
		juce::String resolvedRenderPath;
		juce::String resolvedRenderPathWithOriginalsSubfolder;
		juce::String finalRenderPath;
	};

	struct TimeRange
	{
		double start{};
		double end{};

		bool overlaps(const TimeRange& other)
		{
			return start <= other.end && end >= other.start;
		}
	};

	struct ReaperMarker
	{
		const char* name;
		bool isRegion;
		TimeRange timeRange;
		int index;
	};

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

		auto applicationStateString = applicationState.toXmlString();
		auto applicationStateStringSize = juce::String(applicationStateString.getNumBytesAsUTF8());

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

		juce::ValueTree state;

		auto projectInfo = getProjectInfo();

		std::string buffer(ReaperContextConstants::defaultBufferSize, '\0');
		getProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateSizeKey.toUTF8(), &buffer[0], buffer.size());

		auto stateSize = std::strtoll(&buffer[0], nullptr, 10);

		if(stateSize != 0)
		{
			buffer.resize(stateSize);

			if(getProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateKey.toUTF8(), &buffer[0], buffer.size()))
			{
				auto valueTree = juce::ValueTree::fromXml(buffer);
				state = valueTree;
			}
		}

		return state;
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

	void ReaperContext::renderImportItems()
	{
		main_OnCommand(42230, 0);
	}

	std::vector<WwiseTransfer::Import::Item> ReaperContext::getImportItems(WwiseTransfer::Import::Options options)
	{
		using namespace ReaperContextConstants;

		auto projectInfo = getProjectInfo();

		auto renderDirectoryPath = getProjectString(projectInfo.projectReference, "RENDER_FILE", defaultBufferSize);
		auto renderPattern = getProjectString(projectInfo.projectReference, "RENDER_PATTERN", defaultBufferSize);

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

		auto renderDirectoryPathPart = renderDirectoryPath.isNotEmpty() ? renderDirectoryPath + juce::File::getSeparatorString() : "";
		auto originalsSubfolderPathPart = options.originalsSubfolder.isNotEmpty() ? options.originalsSubfolder + juce::File::getSeparatorString() : "";

		auto resolvedRenderPaths = getItemListFromRenderPattern(projectInfo.projectReference, renderDirectoryPathPart + renderPattern);
		auto resolvedRenderPathsWithOriginalsSubfolder = getItemListFromRenderPattern(projectInfo.projectReference, renderDirectoryPathPart + originalsSubfolderPathPart + renderPattern);

		if(resolvedRenderPaths.size() != resolvedRenderPathsWithOriginalsSubfolder.size())
		{
			juce::Logger::writeToLog("Reaper: Mismatch between resolvedRenderPaths and resolvedRenderPathsWithOriginalsSubfolder");
			return {};
		}

		// Estimate the size of render stats so we can pass reaper the propper buffer size
		int totalBufferSize = 0;
		int extra = 150; // Each path may come with aditional render statistics (peak information).
		for(const auto& resolvedRenderPath : resolvedRenderPaths)
		{
			totalBufferSize += resolvedRenderPath.length() + extra;
		}

		// Get list of files that where rendered during the last render run using the current render settings and render pattern
		// These file names may differ from the resolved render paths due to silent renaming.
		juce::StringArray finalRenderPaths;

		if(totalBufferSize > 0)
		{
			juce::String renderStats = getProjectString(projectInfo.projectReference, "RENDER_STATS", totalBufferSize);

			juce::StringArray temp;
			temp.addTokens(renderStats, ";", "");

			// Not all items in here are paths. Paths are prefixed with "FILE:"
			for(auto item : temp)
			{
				if(item.startsWith("FILE:"))
					finalRenderPaths.add(item.trimCharactersAtStart("FILE:"));
			}
		}

		std::vector<WwiseTransfer::Import::Item> importItems;

		auto resolvePattern = options.importDestination + options.hierarchyMappingPath;

		std::vector<juce::String> resolvedObjectPaths = getItemListFromRenderPattern(projectInfo.projectReference, resolvePattern, false);

		if(resolvedObjectPaths.size() != resolvedRenderPaths.size() || resolvedObjectPaths.size() != resolvedRenderPathsWithOriginalsSubfolder.size())
		{
			juce::Logger::writeToLog("Reaper: Mismatch between resolvedObjectPaths, resolvedRenderPaths and resolvedRenderPathsWithOriginalsSubfolder");
			return {};
		}

		for(int i = 0; i < resolvedObjectPaths.size(); ++i)
		{
			juce::String objectPath(resolvedObjectPaths[i].upToLastOccurrenceOf(".", false, false));

			juce::File parentDirectory = juce::File(resolvedRenderPathsWithOriginalsSubfolder[i])
			                                 .getParentDirectory();

			juce::String currentOriginalsSubfolder = parentDirectory == renderDirectory ? "" : parentDirectory.getRelativePathFrom(renderDirectory);

			auto objectName = WwiseTransfer::WwiseHelper::pathToObjectName(objectPath);

			importItems.emplace_back(objectName,
				WwiseTransfer::Wwise::ObjectType::SoundSFX,
				objectPath,
				currentOriginalsSubfolder,
				resolvedRenderPaths[i],
				finalRenderPaths[i]);
		}

		return importItems;
	}

	juce::String ReaperContext::getProjectString(ReaProject* project, const char* key, int bufferSize)
	{
		std::string buffer(bufferSize, '\0');

		if(getSetProjectInfo_String(project, key, &buffer[0], false))
		{
			auto end = buffer.find('\0');
			buffer.resize(end);
			return buffer;
		}

		return juce::String();
	}

	ProjectInfo ReaperContext::getProjectInfo()
	{
		std::string buffer(ReaperContextConstants::defaultBufferSize, '\0');

		ReaProject* projectReference;

		// The buffer sent to enumProjects will contain the project path. It is a requirement that a project file ends with the .rpp (case insensitive) file extension.
		while((projectReference = enumProjects(-1, &buffer[0], buffer.size())) && !juce::String(buffer).endsWithIgnoreCase(".rpp"))
		{
			if(buffer.size() > 10 << 20 || buffer.empty())
			{
				jassertfalse;

				return {};
			}
			else
			{
				try
				{
					buffer.resize(2 * buffer.size());
				}
				catch(std::bad_alloc&)
				{
					jassertfalse;

					return {};
				}
			}
		}

		juce::File projectFile(buffer);

		return {
			projectReference,
			projectFile.getFileNameWithoutExtension(),
			projectFile.getFullPathName()};
	}

	std::vector<juce::String> ReaperContext::getItemListFromRenderPattern(ReaProject* project, const juce::String& pattern, bool suppressIllegalPaths)
	{
		int bufferLength = resolveRenderPattern(project, suppressIllegalPaths ? "" : nullptr, pattern.toUTF8(), nullptr, 0);
		std::string buffer(bufferLength, '\0');

		resolveRenderPattern(project, suppressIllegalPaths ? "" : nullptr, pattern.toUTF8(), &buffer[0], bufferLength);
		return splitDoubleNullTerminatedString(&buffer[0]);
	}
} // namespace AK::ReaWwise
