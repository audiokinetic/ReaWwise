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

#include "CustomDrawableButton.h"
#include "Persistance/ApplicationProperties.h"

#include <memory>

namespace AK::WwiseTransfer
{
	class PresetMenuComponent
		: public CustomDrawableButton
	{
	public:
		PresetMenuComponent(juce::ValueTree appState, ApplicationProperties& applicationProperties, const juce::String& applicationName);

	private:
		juce::File presetFolder;
		std::unique_ptr<juce::FileChooser> fileChooser;

		juce::ValueTree hierarchyMapping;

		ApplicationProperties& applicationProperties;

		void showMenu();
		void loadPreset(const juce::File& filename);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetMenuComponent)
	};
} // namespace AK::WwiseTransfer
