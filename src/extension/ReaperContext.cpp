#include "ReaperContext.h"

#include "Helpers/WwiseHelper.h"
#include "Model/Wwise.h"

namespace AK::ReaWwise
{
	namespace ReaperContextConstants
	{
		constexpr int defaultBufferSize = 4096;
		const juce::String stateSizeKey = "stateSize";
		const juce::String stateKey = "state";
		const juce::String applicationKey = "ReaWwise";
		const juce::String defaultRenderPattern = "untitled";
	} // namespace ReaperContextConstants

	ReaperContext::ReaperContext(ReaperPluginInterface& pluginInfo)
		: reaperPluginInterface(pluginInfo)
		, defaultRenderDirectory(juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("REAPER Media"))
	{
	}

	ReaperContext::~ReaperContext()
	{
	}

	juce::String ReaperContext::getSessionName()
	{
		juce::ScopedLock lock{apiAccess};

		auto projectInfo = getProjectInfo();
		return projectInfo.projectPath;
	}

	bool ReaperContext::saveState(juce::ValueTree applicationState)
	{
		using namespace ReaperContextConstants;

		juce::ScopedLock lock{apiAccess};

		auto projectInfo = getProjectInfo();

		const auto applicationStateString = applicationState.toXmlString();
		const auto applicationStateStringSize = juce::String(applicationStateString.getNumBytesAsUTF8());
		if(reaperPluginInterface.setProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateSizeKey.toUTF8(), applicationStateStringSize.toUTF8()) &&
			reaperPluginInterface.setProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateKey.toUTF8(), applicationStateString.toUTF8()))
		{
			reaperPluginInterface.markProjectDirty(projectInfo.projectReference);
			return true;
		}

		return false;
	}

	juce::ValueTree ReaperContext::retrieveState()
	{
		using namespace ReaperContextConstants;

		juce::ScopedLock lock{apiAccess};

		auto projectInfo = getProjectInfo();

		std::string buffer(defaultBufferSize, '\0');
		reaperPluginInterface.getProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateSizeKey.toUTF8(), &buffer[0], buffer.size());

		const auto stateSize = std::strtoll(&buffer[0], nullptr, 10);
		if(stateSize == 0)
			return {};

		buffer.resize(stateSize);
		if(reaperPluginInterface.getProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateKey.toUTF8(), &buffer[0], buffer.size()))
			return juce::ValueTree::fromXml(buffer);

		return {};
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

	void ReaperContext::renderItems()
	{
		reaperPluginInterface.main_OnCommand(42230, 0);
	}

	std::vector<WwiseTransfer::Import::Item> ReaperContext::getItemsForImport(const WwiseTransfer::Import::Options& options)
	{
		juce::ScopedLock lock{apiAccess};

		std::vector<WwiseTransfer::Import::Item> importItems;

		auto importItemsForPreview = getItemsForPreview(options);
		if(importItemsForPreview.size() == 0)
			return importItems;

		auto projectInfo = getProjectInfo();
		const auto renderDirectory = getRenderDirectory(projectInfo);

		juce::StringArray temp;
		juce::String renderStats = reaperPluginInterface.getProjectString(projectInfo.projectReference, "RENDER_STATS");
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
		juce::ScopedLock lock{apiAccess};

		auto projectInfo = getProjectInfo();

		const auto renderDirectory = getRenderDirectory(projectInfo);
		const auto originalsSubfolderPathPart = options.originalsSubfolder + juce::File::getSeparatorString();

		const auto resolvedOriginalsSubfolder = getItemListFromRenderPattern(projectInfo.projectReference, originalsSubfolderPathPart, false);

		juce::StringArray renderTargets;
		juce::String renderTargetsString = reaperPluginInterface.getProjectString(projectInfo.projectReference, "RENDER_TARGETS");
		renderTargets.addTokens(renderTargetsString, ";", "");

		if(renderTargets.size() != resolvedOriginalsSubfolder.size())
		{
			juce::Logger::writeToLog("Reaper: Mismatch between renderTargets and resolvedOriginalsSubfolder");
			return {};
		}

		const auto objectPathsPattern = options.importDestination + options.hierarchyMappingPath;
		std::vector<juce::String> resolvedObjectPaths = getItemListFromRenderPattern(projectInfo.projectReference, objectPathsPattern, false);

		if(resolvedObjectPaths.size() != renderTargets.size() || resolvedObjectPaths.size() != resolvedOriginalsSubfolder.size())
		{
			juce::Logger::writeToLog("Reaper: Mismatch between resolvedObjectPaths, renderTargets and resolvedOriginalsSubfolder");
			return {};
		}

		std::vector<WwiseTransfer::Import::PreviewItem> importItems;
		for(int i = 0; i < resolvedObjectPaths.size(); ++i)
		{
			const auto& objectPath = resolvedObjectPaths[i].upToLastOccurrenceOf(".", false, false);
			const auto& renderTarget = renderTargets[i];

			// Get the parent of the render target, as a relative path against the render directory
			// We want to preserve this hierarchy in the wwise originals folder
			juce::String relativeParentDir;
			if(juce::File(renderTarget).getParentDirectory() != renderDirectory)
				relativeParentDir = juce::File(renderTarget).getRelativePathFrom(renderDirectory).upToLastOccurrenceOf(juce::File::getSeparatorString(), false, true);

			// Reaper may append a differentiator at the end of a path during pattern resolving. We don't care about it.
			auto originalsSubfolder = resolvedOriginalsSubfolder[i].upToLastOccurrenceOf(juce::File::getSeparatorString(), false, true);

			// The originalsSubfolder is a combination of what the user inputs in the gui (resolvedOriginalsSubfolder) and the directory structure under the render directory.
			if(relativeParentDir.isNotEmpty())
			{
				if(originalsSubfolder.isNotEmpty())
					originalsSubfolder << juce::File::getSeparatorString();

				originalsSubfolder << relativeParentDir;
			}

			importItems.push_back({objectPath, originalsSubfolder, renderTarget});
		}

		return importItems;
	}

	ReaperContext::ProjectInfo ReaperContext::getProjectInfo() const
	{
		std::string buffer(ReaperContextConstants::defaultBufferSize, '\0');

		// The buffer sent to enumProjects will contain the project path.
		auto projectReference = reaperPluginInterface.enumProjects(-1, &buffer[0], buffer.size());
		if(!projectReference || buffer.empty())
			return {};

		const juce::File projectFile(buffer);

		// REAPER requires that a project file ends with the .rpp (case insensitive) file extension.
		if(projectFile.getFileExtension().compareIgnoreCase(".rpp") != 0)
			return {};

		return {
			projectReference,
			projectFile.getFileNameWithoutExtension(),
			projectFile.getFullPathName()};
	}

	juce::String ReaperContext::getRenderPattern(const ReaperContext::ProjectInfo& projectInfo) const
	{
		// There are several scenarios where the render pattern could be empty
		// 1. When the project hasn't been saved (reaper uses "untitled")
		// 2. When the project has been saved (reaper uses the project name)
		auto renderPattern = reaperPluginInterface.getProjectString(projectInfo.projectReference, "RENDER_PATTERN");
		if(renderPattern.isNotEmpty())
			return renderPattern;

		if(projectInfo.projectPath.isEmpty())
			return ReaperContextConstants::defaultRenderPattern;

		return projectInfo.projectName;
	}

	juce::File ReaperContext::getRenderDirectory(const ReaperContext::ProjectInfo& projectInfo) const
	{
		auto renderDirectoryPath = reaperPluginInterface.getProjectString(projectInfo.projectReference, "RENDER_FILE");
		if(juce::File::isAbsolutePath(renderDirectoryPath))
			return juce::File(renderDirectoryPath);

		if(projectInfo.projectPath.isNotEmpty())
			return juce::File(projectInfo.projectPath).getParentDirectory().getChildFile(renderDirectoryPath);

		// If the project wasnt saved, reaper uses a general render directory
		return defaultRenderDirectory.getChildFile(renderDirectoryPath);
	}

	bool ReaperContext::sessionChanged()
	{
		auto sessionChanged = false;

		auto projectInfo = getProjectInfo();

		auto projectStateCount = reaperPluginInterface.getProjectStateChangeCount(projectInfo.projectReference);
		auto renderSource = reaperPluginInterface.getSetProjectInfo(projectInfo.projectReference, "RENDER_SETTINGS", 0, false);
		auto renderBounds = reaperPluginInterface.getSetProjectInfo(projectInfo.projectReference, "RENDER_BOUNDSFLAG", 0, false);
		auto renderFile = reaperPluginInterface.getProjectString(projectInfo.projectReference, "RENDER_FILE");
		auto renderPattern = reaperPluginInterface.getProjectString(projectInfo.projectReference, "RENDER_PATTERN");

		if(projectStateCount != stateInfo.projectStateCount ||
			renderSource != stateInfo.renderSource ||
			renderBounds != stateInfo.renderBounds ||
			renderFile != stateInfo.renderFile ||
			renderPattern != stateInfo.renderPattern)
		{
			sessionChanged = true;
		}

		stateInfo = {
			projectStateCount,
			renderSource,
			renderBounds,
			renderFile,
			renderPattern};

		return sessionChanged;
	}

	std::vector<juce::String> ReaperContext::getItemListFromRenderPattern(ReaProject* project, const juce::String& pattern, bool suppressIllegalPaths)
	{
		const int bufferLength = reaperPluginInterface.resolveRenderPattern(project, suppressIllegalPaths ? "" : nullptr, pattern.toUTF8(), nullptr, 0);
		std::string buffer(bufferLength, '\0');

		const int newBufferLength = reaperPluginInterface.resolveRenderPattern(project, suppressIllegalPaths ? "" : nullptr, pattern.toUTF8(), &buffer[0], bufferLength);
		if(newBufferLength > bufferLength)
		{
			// It is possible the resolved render pattern changes between the two calls to resolveRenderPattern.
			// For example, a track that is set to render can be unmuted between the two calls which will result in a bigger buffer needed.
			// In that case we just return nothing and the next call will be good.
			juce::Logger::writeToLog("Reaper: Mismatch between calls to resolveRenderPattern");
			return {};
		}

		return splitDoubleNullTerminatedString(&buffer[0]);
	}
} // namespace AK::ReaWwise
