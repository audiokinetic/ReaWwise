/*----------------------------------------------------------------------------------------

Copyright (c) 2026 AUDIOKINETIC Inc.

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
#include "Model/Wwise.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	namespace FeatureSupportConstants
	{
		const juce::String FindInProjectExplorerSelectionChannel1 = "FindInProjectExplorerSelectionChannel1";

		const Wwise::Version v2026_1_0_0 = {2026, 1, 0, 0};
		const Wwise::Version v2025_1_0_0 = {2025, 1, 0, 0};
		const Wwise::Version v2024_1_0_0 = {2024, 1, 0, 0};
		const Wwise::Version v2023_1_0_0 = {2023, 1, 0, 0};

		const Wwise::Version minimum_supported_version = v2024_1_0_0;
	} // namespace FeatureSupportConstants

	class FeatureSupport
		: juce::ValueTree::Listener
	{
	public:
		FeatureSupport(juce::ValueTree appState, WaapiClient& waapiClient);
		~FeatureSupport();

	private:
		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
			const juce::Identifier& property) override;

		WaapiClient& waapiClient;

		juce::ValueTree applicationState;
		juce::ValueTree version;

		juce::ValueTree featureSupport;
		juce::CachedValue<bool> newObjectNamesEnabled;
		juce::CachedValue<bool> isSegmentUniversal;
		juce::CachedValue<bool> isUsingInteractiveMusicHierarchy;
	};
} // namespace AK::WwiseTransfer
