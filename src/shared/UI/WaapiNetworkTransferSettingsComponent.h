/*----------------------------------------------------------------------------------------

Copyright (c) 2024 AUDIOKINETIC Inc.

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
#include "Persistance/ApplicationProperties.h"
#include "Theme/CustomLookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer
{
	class WaapiNetworkTransferSettingsComponent : public juce::Component
	{
	public:
		WaapiNetworkTransferSettingsComponent(const juce::String& applicationName, ApplicationProperties& applicationProperties, WaapiClientWatcher& waapiCW);
		~WaapiNetworkTransferSettingsComponent() override;

		void resized() override;
		void paint(juce::Graphics& g) override;

	private:
		CustomLookAndFeel lookAndFeel;

		juce::Label enableCrossMachineTransferLabel;
		juce::ToggleButton enableCrossMachineTransferButton;

		juce::Label ipAddressLabel;
		juce::TextEditor ipAddressTextEditor;

		juce::Label portLabel;
		juce::TextEditor portTextEditor;

		ApplicationProperties& applicationProperties;
		WaapiClientWatcher& waapiClientWatcher;

		void setToCrossMachineTransfer(bool isCrossMachineTransfer, bool onInit = false);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaapiNetworkTransferSettingsComponent)
	};
} // namespace AK::WwiseTransfer
