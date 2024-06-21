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

#include <juce_data_structures/juce_data_structures.h>

namespace AK::WwiseTransfer
{
	class ApplicationProperties : juce::ApplicationProperties
	{
	public:
		explicit ApplicationProperties(const juce::String& applicationName);

		juce::String getWaapiIp();
		void setWaapiIp(const juce::String& ip);
		int getWaapiPort();
		void setWaapiPort(int port);
		int getPreviewRefreshInterval();
		juce::StringArray getRecentHierarchyMappingPresets();
		void addRecentHierarchyMappingPreset(const juce::String& path);
		void removeRecentHierarchyMappingPreset(const juce::String& path);
		void clearRecentHierarchyMappingPresets();
		double getScaleFactorOverride();
		bool getShowSilentIncrementWarning();
		void setShowSilentIncrementWarning(bool value);
		bool getIsCrossMachineTransferEnabled();
		void setIsCrossMachineTransferEnabled(bool value);

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ApplicationProperties)
	};
} // namespace AK::WwiseTransfer
