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

#pragma once

#include "Core/DawContext.h"
#include "IReaperPlugin.h"
#include "Model/Import.h"

namespace AK::ReaWwise
{
	class ReaperContext
		: public WwiseTransfer::DawContext
	{
	public:
		ReaperContext(IReaperPlugin& pluginInfo);
		~ReaperContext() override;

		bool sessionChanged() override;
		juce::String getSessionName() override;
		bool saveState(juce::ValueTree applicationState) override;
		juce::ValueTree retrieveState() override;
		void renderItems() override;
		std::vector<WwiseTransfer::Import::PreviewItem> getItemsForPreview(const WwiseTransfer::Import::Options& options) override;
		std::vector<WwiseTransfer::Import::Item> getItemsForImport(const WwiseTransfer::Import::Options& options) override;

	private:
		struct ProjectInfo
		{
			ReaProject* projectReference{};
			juce::String projectName;
			juce::String projectPath;
		};

		struct StateInfo
		{
			int projectStateCount{0};
			double renderSource{0.0};
			double renderBounds{0.0};
			juce::String renderFile;
			juce::String renderPattern;
		};

		struct ProjectStringBufferResult
		{
			bool status{false};
			std::vector<char> buffer;
		};

		std::vector<juce::String> getItemListFromRenderPattern(ReaProject* project, const juce::String& pattern, bool suppressIllegalPaths = true);
		ProjectInfo getProjectInfo() const;
		juce::String getRenderPattern(const ProjectInfo& projectInfo) const;
		std::vector<juce::String> getOriginalSubfolders(const ProjectInfo& projectInfo, const juce::String& originalsSubfolder);
		std::vector<juce::String> getRenderTargets();
		juce::String getProjectString(ReaProject* proj, const char* key) const;
		ProjectStringBufferResult getProjectStringBuffer(ReaProject* proj, const char* key) const;

		juce::CriticalSection apiAccess;
		IReaperPlugin& reaperPlugin;
		StateInfo stateInfo;
	};
} // namespace AK::ReaWwise
