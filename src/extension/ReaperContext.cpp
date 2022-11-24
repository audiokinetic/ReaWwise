/*----------------------------------------------------------------------------------------

Copyright (c) 2023 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#include "ReaperContext.h"

#include "Helpers/StringHelper.h"
#include "Helpers/WwiseHelper.h"
#include "Model/Wwise.h"

#include <regex>

namespace AK::ReaWwise
{
	namespace ReaperContextConstants
	{
		constexpr int defaultBufferSize = 4 * 1024;
		constexpr int largeBufferSize = 4 * 1024 * 1024;
		const juce::String stateSizeKey = "stateSize";
		const juce::String stateKey = "state";
		const juce::String applicationKey = "ReaWwise";
		const juce::String defaultRenderPattern = "untitled";
	} // namespace ReaperContextConstants

	enum ReaperCommands
	{
		Render = 42230
	};

	ReaperContext::ReaperContext(IReaperPlugin& reaperPlugin)
		: reaperPlugin(reaperPlugin)
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
		if(reaperPlugin.setProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateSizeKey.toUTF8(), applicationStateStringSize.toUTF8()) &&
			reaperPlugin.setProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateKey.toUTF8(), applicationStateString.toUTF8()))
		{
			reaperPlugin.markProjectDirty(projectInfo.projectReference);
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
		reaperPlugin.getProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateSizeKey.toUTF8(), &buffer[0], buffer.size());

		const auto stateSize = std::strtoll(&buffer[0], nullptr, 10);
		if(stateSize == 0)
			return {};

		buffer.resize(stateSize);
		if(reaperPlugin.getProjExtState(projectInfo.projectReference, applicationKey.toUTF8(), stateKey.toUTF8(), &buffer[0], buffer.size()))
			return juce::ValueTree::fromXml(buffer);

		return {};
	}

	void ReaperContext::renderItems()
	{
		reaperPlugin.main_OnCommand(ReaperCommands::Render, 0);
	}

	std::vector<juce::String> ReaperContext::getRenderTargets()
	{
		auto projectInfo = getProjectInfo();

		std::vector<juce::String> renderTargets;

		auto result = getProjectStringBuffer(projectInfo.projectReference, "RENDER_TARGETS_EX");

		if(result.status)
			renderTargets = WwiseTransfer::StringHelper::splitDoubleNullTerminatedString(result.buffer);
		else
		{
			// For REAPER < 6.69
			auto renderTargetsString = getProjectString(projectInfo.projectReference, "RENDER_TARGETS");

			juce::StringArray renderTargetsStringArray;
			renderTargetsStringArray.addTokens(renderTargetsString, ";", "");
			renderTargetsStringArray.removeEmptyStrings();

			renderTargets = std::vector<juce::String>(renderTargetsStringArray.strings.begin(), renderTargetsStringArray.strings.end());
		}

		return renderTargets;
	}

	juce::String ReaperContext::getProjectString(ReaProject* proj, const char* key) const
	{
		auto result = getProjectStringBuffer(proj, key);

		if(result.buffer.size() > 0)
			return WwiseTransfer::StringHelper::utf8EncodedCharArrayToString(result.buffer);

		return {};
	}

	ReaperContext::ProjectStringBufferResult ReaperContext::getProjectStringBuffer(ReaProject* proj, const char* key) const
	{
		ProjectStringBufferResult result;

		if(reaperPlugin.supportsReallocCommands())
		{
			// For REAPER 6.68+
			char buffer[ReaperContextConstants::defaultBufferSize];
			char* bufferPtr = buffer;

			int bufferSize = (int)sizeof(buffer);

			int token = reaperPlugin.reallocCmdRegisterBuf(&bufferPtr, &bufferSize);

			result.status = reaperPlugin.getSetProjectInfo_String(proj, key, bufferPtr, false);

			if(result.status)
				result.buffer.assign(bufferPtr, bufferPtr + bufferSize);

			reaperPlugin.reallocCmdClear(token);
		}
		else
		{
			static std::vector<char> buffer(ReaperContextConstants::largeBufferSize);
			std::fill(buffer.begin(), buffer.end(), '\0');

			result.status = reaperPlugin.getSetProjectInfo_String(proj, key, &buffer[0], false);

			if(result.status)
				result.buffer = buffer;
		}

		return result;
	}

	std::vector<WwiseTransfer::Import::Item> ReaperContext::getItemsForImport(const WwiseTransfer::Import::Options& options)
	{
		juce::ScopedLock lock{apiAccess};

		std::vector<WwiseTransfer::Import::Item> importItems;

		auto importItemsForPreview = getItemsForPreview(options);
		if(importItemsForPreview.size() == 0)
			return importItems;

		auto projectInfo = getProjectInfo();

		juce::String renderStats = getProjectString(projectInfo.projectReference, "RENDER_STATS");

		static juce::String fileToken("FILE:");
		static juce::String delimiter(';' + fileToken);

		if(renderStats.isNotEmpty())
		{
			// To ease parsing, append ";FILE:" to the end of renderStats
			if(renderStats.endsWithChar(';'))
				renderStats << fileToken;
			else
				renderStats << delimiter;

			int endPosition, startPosition = renderStats.indexOf(fileToken) + fileToken.length();

			if(startPosition != -1) // If we don't find the first "FILE:", exit since we are receiving something unexpected
			{
				static std::regex regex("(.+?);[A-Z]+");

				while((endPosition = renderStats.indexOf(startPosition, delimiter)) != -1)
				{
					auto finalRenderPath = renderStats.substring(startPosition, endPosition).toStdString();

					std::smatch results;
					if(std::regex_search(finalRenderPath, results, regex))
						finalRenderPath = results[1];

					const auto& importItemForPreview = importItemsForPreview[importItems.size()];

					importItems.push_back({
						importItemForPreview.path,
						importItemForPreview.originalsSubFolder,
						importItemForPreview.audioFilePath,
						finalRenderPath,
					});

					startPosition = endPosition + delimiter.length();
				}
			}
		}

		return importItems;
	}

	std::vector<juce::String> ReaperContext::getOriginalSubfolders(const ProjectInfo& projectInfo, const juce::String& originalsSubfolder)
	{
		// The originals subfolder is a combination of what the user inputs in the originals subfolder input field
		// Combined with anything in the render file path after the render folder

		// To get the resolved file paths relative to the render directory, we can simply subtract the parent paths in resolvedDummyRenderPattern from
		// the file paths in resolvedRenderPattern. Other approaches require us to know the render directory which is difficult to figure out and
		// requires alot of logic on our end.

		const auto dummyRenderPattern = juce::File::getSeparatorString();
		const auto resolvedDummyRenderPattern = getItemListFromRenderPattern(projectInfo.projectReference, dummyRenderPattern, true);

		const auto renderPattern = getRenderPattern(projectInfo);
		const auto resolvedRenderPattern = getItemListFromRenderPattern(projectInfo.projectReference, renderPattern, true);

		const auto originalsSubfolderRenderPattern = originalsSubfolder + juce::File::getSeparatorString();
		const auto resolvedOriginalsSubfolder = getItemListFromRenderPattern(projectInfo.projectReference, originalsSubfolderRenderPattern, true);

		if(resolvedDummyRenderPattern.size() != resolvedRenderPattern.size() && resolvedDummyRenderPattern.size() != resolvedOriginalsSubfolder.size())
		{
			juce::Logger::writeToLog("Reaper: Mismatch between resolvedDummyRenderPattern, resolvedRenderPattern and resolvedOriginalsSubfolder");
			return {};
		}

		std::vector<juce::String> finalOriginalsSubfolders;
		for(int i = 0; i < resolvedDummyRenderPattern.size(); ++i)
		{
			auto renderDirectory = juce::File(resolvedDummyRenderPattern[i]).getParentDirectory();
			auto relativeResolvedRenderPattern = juce::File(resolvedRenderPattern[i]).getRelativePathFrom(renderDirectory);
			auto originalsSubfolderFile = juce::File(resolvedOriginalsSubfolder[i]).getParentDirectory().getChildFile(relativeResolvedRenderPattern);

			juce::String originalsSubfolder = "";
			if(originalsSubfolderFile.getParentDirectory() != renderDirectory)
				originalsSubfolder = originalsSubfolderFile.getRelativePathFrom(renderDirectory).upToLastOccurrenceOf(juce::File::getSeparatorString(), false, true);

			finalOriginalsSubfolders.push_back(originalsSubfolder);
		}

		return finalOriginalsSubfolders;
	}

	std::vector<WwiseTransfer::Import::PreviewItem> ReaperContext::getItemsForPreview(const WwiseTransfer::Import::Options& options)
	{
		juce::ScopedLock lock{apiAccess};

		auto projectInfo = getProjectInfo();

		auto renderTargets = getRenderTargets();
		auto resolvedOriginalsSubfolder = getOriginalSubfolders(projectInfo, options.originalsSubfolder);

		const auto objectPathsPattern = options.importDestination + options.hierarchyMappingPath;
		std::vector<juce::String> resolvedObjectPaths = getItemListFromRenderPattern(projectInfo.projectReference, objectPathsPattern, false);

		if(renderTargets.size() != resolvedOriginalsSubfolder.size() || renderTargets.size() != resolvedObjectPaths.size())
		{
			juce::Logger::writeToLog("Reaper: Mismatch between renderTargets, resolvedObjectPaths and resolvedOriginalsSubfolder");
			return {};
		}

		std::vector<WwiseTransfer::Import::PreviewItem> importItems;
		for(int i = 0; i < resolvedObjectPaths.size(); ++i)
		{
			const auto& objectPath = resolvedObjectPaths[i].upToLastOccurrenceOf(".", false, false);

			importItems.push_back({objectPath, resolvedOriginalsSubfolder[i], renderTargets[i]});
		}

		return importItems;
	}

	ReaperContext::ProjectInfo ReaperContext::getProjectInfo() const
	{
		std::string buffer(ReaperContextConstants::defaultBufferSize, '\0');

		// The buffer sent to enumProjects will contain the project path.
		auto projectReference = reaperPlugin.enumProjects(-1, &buffer[0], buffer.size());
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
		auto renderPattern = getProjectString(projectInfo.projectReference, "RENDER_PATTERN");
		if(renderPattern.isNotEmpty())
			return renderPattern;

		if(projectInfo.projectPath.isEmpty())
			return ReaperContextConstants::defaultRenderPattern;

		return projectInfo.projectName;
	}

	bool ReaperContext::sessionChanged()
	{
		auto sessionChanged = false;

		auto projectInfo = getProjectInfo();

		auto projectStateCount = reaperPlugin.getProjectStateChangeCount(projectInfo.projectReference);
		auto renderSource = reaperPlugin.getSetProjectInfo(projectInfo.projectReference, "RENDER_SETTINGS", 0, false);
		auto renderBounds = reaperPlugin.getSetProjectInfo(projectInfo.projectReference, "RENDER_BOUNDSFLAG", 0, false);
		auto renderFile = getProjectString(projectInfo.projectReference, "RENDER_FILE");
		auto renderPattern = getProjectString(projectInfo.projectReference, "RENDER_PATTERN");

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
		const int bufferLength = reaperPlugin.resolveRenderPattern(project, suppressIllegalPaths ? "" : nullptr, pattern.toUTF8(), nullptr, 0);

		if(bufferLength == 0)
			return {};

		std::vector<char> buffer(bufferLength, '\0');
		const int newBufferLength = reaperPlugin.resolveRenderPattern(project, suppressIllegalPaths ? "" : nullptr, pattern.toUTF8(), &buffer[0], bufferLength);
		if(newBufferLength > bufferLength)
		{
			// It is possible the resolved render pattern changes between the two calls to resolveRenderPattern.
			// For example, a track that is set to render can be unmuted between the two calls which will result in a bigger buffer needed.
			// In that case we just return nothing and the next call will be good.
			juce::Logger::writeToLog("Reaper: Mismatch between calls to resolveRenderPattern");
			return {};
		}

		return WwiseTransfer::StringHelper::splitDoubleNullTerminatedString(buffer);
	}
} // namespace AK::ReaWwise
