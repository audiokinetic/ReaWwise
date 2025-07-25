/*----------------------------------------------------------------------------------------

Copyright (c) 2025 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

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
		juce::CachedValue<bool> additionalProjectInfoLookupEnabled;
		juce::CachedValue<juce::String> originalsFolder;
		juce::CachedValue<juce::String> languageSubfolder;
		juce::CachedValue<juce::String> referenceLanguage;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
		void valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override;

		void loadProjectInfo();

		void loadAdditionalProjectInfo();
		void resetAdditionalProjectInfo();

		void loadProjectLanguages();

		void updateLanguageSubpath();
		void updateLangugeForHierarchyMappingNodes();
	};
} // namespace AK::WwiseTransfer
