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

#include "WaapiNetworkTransferSettingsComponent.h"

#include "AK/AkWwiseSDKVersion.h"
#include "BinaryData.h"
#include "Theme/CustomLookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>

#define AK_WWISE_VST_VERSION_FULL AK_WWISESDK_VERSIONNAME_SHORT "." AK_WWISESDK_NUM2STRING(AK_WWISESDK_VERSION_BUILD)

namespace AK::WwiseTransfer
{
	namespace CrossMachineTransferComponentConstants
	{
		constexpr int editorBoxHeight = 26;
		constexpr int labelWidth = 220;
		constexpr int margin = 10;
		constexpr int width = 370;
		constexpr int height = 100;
	} // namespace CrossMachineTransferComponentConstants

	WaapiNetworkTransferSettingsComponent::WaapiNetworkTransferSettingsComponent(const juce::String& applicationName,
		ApplicationProperties& appProps,
		WaapiClientWatcher& waapiCW)
		: applicationProperties(appProps)
		, waapiClientWatcher(waapiCW)
	{
		using namespace CrossMachineTransferComponentConstants;

		setSize(width, height);
		setLookAndFeel(&lookAndFeel);

		ipAddressLabel.setText("Waapi IP Address", juce::dontSendNotification);
		ipAddressLabel.setBorderSize(juce::BorderSize(0));
		ipAddressLabel.setMinimumHorizontalScale(1.0f);
		ipAddressLabel.setJustificationType(juce::Justification::left);

		ipAddressTextEditor.setFont(CustomLookAndFeelConstants::smallFontSize);
		ipAddressTextEditor.setText(applicationProperties.getWaapiIp());
		ipAddressTextEditor.onFocusLost = [this]
		{
			applicationProperties.setWaapiIp(ipAddressTextEditor.getText());
			waapiClientWatcher.changeParameters(applicationProperties.getWaapiIp(), applicationProperties.getWaapiPort());
		};

		addAndMakeVisible(ipAddressLabel);
		addAndMakeVisible(ipAddressTextEditor);

		portLabel.setText("Waapi Port", juce::dontSendNotification);
		portLabel.setBorderSize(juce::BorderSize(0));
		portLabel.setMinimumHorizontalScale(1.0f);
		portLabel.setJustificationType(juce::Justification::left);

		portTextEditor.setFont(CustomLookAndFeelConstants::smallFontSize);
		portTextEditor.setText(juce::String(applicationProperties.getWaapiPort()));
		portTextEditor.onFocusLost = [this]
		{
			applicationProperties.setWaapiPort(portTextEditor.getTextValue().getValue());
			waapiClientWatcher.changeParameters(applicationProperties.getWaapiIp(), applicationProperties.getWaapiPort());
		};

		addAndMakeVisible(portLabel);
		addAndMakeVisible(portTextEditor);

		enableCrossMachineTransferLabel.setText("Enable Cross Machine Transfer", juce::dontSendNotification);
		enableCrossMachineTransferLabel.setBorderSize(juce::BorderSize(0));
		enableCrossMachineTransferLabel.setMinimumHorizontalScale(1.0f);
		enableCrossMachineTransferLabel.setJustificationType(juce::Justification::left);

		enableCrossMachineTransferButton.setToggleState(applicationProperties.getIsCrossMachineTransferEnabled(), juce::dontSendNotification);
		setToCrossMachineTransfer(applicationProperties.getIsCrossMachineTransferEnabled(), true);
		enableCrossMachineTransferButton.onClick = [this]()
		{
			setToCrossMachineTransfer(!applicationProperties.getIsCrossMachineTransferEnabled());
		};

		addAndMakeVisible(enableCrossMachineTransferLabel);
		addAndMakeVisible(enableCrossMachineTransferButton);
	}

	WaapiNetworkTransferSettingsComponent::~WaapiNetworkTransferSettingsComponent()
	{
		setLookAndFeel(nullptr);
	}

	void WaapiNetworkTransferSettingsComponent::resized()
	{
		using namespace CrossMachineTransferComponentConstants;

		auto area = getLocalBounds();
		area.reduce(margin, margin);

		auto crossMachineTransferEnableSection = area.removeFromTop(editorBoxHeight);
		{
			enableCrossMachineTransferLabel.setBounds(crossMachineTransferEnableSection.removeFromLeft(labelWidth));
			crossMachineTransferEnableSection.removeFromLeft(margin);

			enableCrossMachineTransferButton.setBounds(crossMachineTransferEnableSection);
		}

		auto ipAddressSection = area.removeFromTop(editorBoxHeight);
		{
			ipAddressLabel.setBounds(ipAddressSection.removeFromLeft(labelWidth));
			ipAddressSection.removeFromLeft(margin);

			ipAddressTextEditor.setBounds(ipAddressSection);
		}

		auto portSection = area.removeFromTop(editorBoxHeight);
		{
			portLabel.setBounds(portSection.removeFromLeft(labelWidth));
			portSection.removeFromLeft(margin);

			portTextEditor.setBounds(portSection);
		}
	}

	void WaapiNetworkTransferSettingsComponent::paint(juce::Graphics& g)
	{
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	}

	void WaapiNetworkTransferSettingsComponent::setToCrossMachineTransfer(bool isCrossMachineTransfer, bool onInit)
	{
		applicationProperties.setIsCrossMachineTransferEnabled(isCrossMachineTransfer);
		enableCrossMachineTransferButton.setToggleState(isCrossMachineTransfer, juce::dontSendNotification);
		ipAddressTextEditor.setEnabled(isCrossMachineTransfer);
		ipAddressTextEditor.setText(applicationProperties.getWaapiIp(), false);
		if(!onInit)
			waapiClientWatcher.changeParameters(applicationProperties.getWaapiIp(), applicationProperties.getWaapiPort());
	}
} // namespace AK::WwiseTransfer
