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

#include "AboutComponent.h"

#include "AK/AkWwiseSDKVersion.h"
#include "BinaryData.h"
#include "Theme/CustomLookAndFeel.h"

#include <juce_gui_basics/juce_gui_basics.h>

#define AK_WWISE_VST_VERSION_FULL AK_WWISESDK_VERSIONNAME_SHORT "." AK_WWISESDK_NUM2STRING(AK_WWISESDK_VERSION_BUILD)

namespace AK::WwiseTransfer
{
	namespace AboutComponentConstants
	{
		constexpr int titleHeight = 32;
		constexpr int labelHeight = 24;
		constexpr int linkHeight = 20;
		constexpr int margin = 10;
		constexpr int width = 300;
		constexpr int height = 254;
		constexpr int wwiseIconHeight = 100;
		const juce::String githubBaseUrl = "https://github.com/audiokinetic/";
		const juce::String releaseNoteUrlPart = "/releases/tag/";
		const juce::String licenseUrlPart = "/blob/main/License.txt";
		const juce::String documentationUrl = "https://www.audiokinetic.com/library/reawwise";
	} // namespace AboutComponentConstants

	AboutComponent::AboutComponent(const juce::String& applicationName)
		: copyRightSymbol(juce::CharPointer_UTF8("\xa9"))
		, wwiseIcon(juce::Drawable::createFromImageData(BinaryData::wwise_icon_svg, BinaryData::wwise_icon_svgSize))
		, tooltipWindow(this)

	{
		using namespace AboutComponentConstants;

		setSize(width, height);
		setLookAndFeel(&lookAndFeel);

		titleLabel.setFont(titleHeight);
		titleLabel.setText("ReaWwise", juce::dontSendNotification);
		titleLabel.setJustificationType(juce::Justification::centred);

		versionLabel.setText("Version " + juce::String(JUCE_APPLICATION_VERSION_STRING) + " " + juce::String(COMMIT_HASH), juce::dontSendNotification);
		versionLabel.setEnabled(false);
		versionLabel.setJustificationType(juce::Justification::centred);

		copyrightLabel.setText(juce::String(juce::CharPointer_UTF8("\xc2\xa9")) + " " + juce::String(YEAR) + " Audiokinetic Inc.", juce::dontSendNotification);
		copyrightLabel.setJustificationType(juce::Justification::centred);

		documentationLink.setButtonText("Documentation");
		documentationLink.setAlpha(0.5f);
		documentationLink.setURL(documentationUrl);
		documentationLink.setJustificationType(juce::Justification::centred);

		releaseNotesLink.setButtonText("Release Notes");
		releaseNotesLink.setAlpha(0.5f);
		releaseNotesLink.setURL(githubBaseUrl + applicationName + releaseNoteUrlPart + JUCE_APPLICATION_VERSION_STRING);
		releaseNotesLink.setJustificationType(juce::Justification::centred);

		licensingLink.setButtonText("License Info");
		licensingLink.setAlpha(0.5f);
		licensingLink.setURL(githubBaseUrl + applicationName + licenseUrlPart);
		licensingLink.setJustificationType(juce::Justification::centred);

		addAndMakeVisible(titleLabel);
		addAndMakeVisible(versionLabel);
		addAndMakeVisible(copyrightLabel);
		addAndMakeVisible(documentationLink);
		addAndMakeVisible(releaseNotesLink);
		addAndMakeVisible(licensingLink);
		addAndMakeVisible(*wwiseIcon);

		versionLabel.setTooltip("Wwise SDK: " + juce::String(AK_WWISE_VST_VERSION_FULL) + juce::newLine +
								"Branch: " + juce::String(BRANCH_NAME) + juce::newLine +
								"Build: " + juce::String(BUILD_NUMBER));
	}

	AboutComponent::~AboutComponent()
	{
		setLookAndFeel(nullptr);
	}

	void AboutComponent::resized()
	{
		using namespace AboutComponentConstants;

		auto area = getLocalBounds();
		area.reduce(margin, margin);

		titleLabel.setBounds(area.removeFromTop(titleHeight));
		versionLabel.setBounds(area.removeFromTop(labelHeight));

		area.removeFromTop(margin);

		wwiseIcon->setTransformToFit(area.removeFromTop(wwiseIconHeight).toFloat(), juce::RectanglePlacement::centred);

		area.removeFromTop(margin);

		copyrightLabel.setBounds(area.removeFromTop(labelHeight));

		area.removeFromTop(margin);

		juce::FlexBox fb;
		fb.flexDirection = juce::FlexBox::Direction::row;

		fb.items.add(juce::FlexItem(documentationLink).withHeight(linkHeight).withFlex(1));
		fb.items.add(juce::FlexItem(releaseNotesLink).withHeight(linkHeight).withFlex(1));
		fb.items.add(juce::FlexItem(licensingLink).withHeight(linkHeight).withFlex(1));

		fb.performLayout(area.removeFromTop(linkHeight));
	}

	void AboutComponent::paint(juce::Graphics& g)
	{
		g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
	}
} // namespace AK::WwiseTransfer
