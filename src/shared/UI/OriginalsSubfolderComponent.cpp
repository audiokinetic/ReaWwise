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

#include "OriginalsSubfolderComponent.h"

#include "BinaryData.h"
#include "Persistance/ApplicationState.h"

namespace
{
	void setImageFromRawData(juce::Image& image, const void* rawData, size_t numBytes)
	{
		// Create a Drawable from the SVG data.
		auto drawable = juce::Drawable::createFromImageData(rawData, numBytes);

		if (drawable)
		{
			int width = drawable->getWidth();
			int height = drawable->getHeight();

			// Create a transparent ARGB image to draw on.
			image = juce::Image(juce::Image::ARGB, width, height, true);

			// Get a graphics context for the image and draw the drawable into it.
			juce::Graphics g(image);
			drawable->drawWithin(g, juce::Rectangle<float>(0, 0, width, height),
				juce::RectanglePlacement::centred, 1.0f);
		}
	}

	const char* kConnectionStatusConnectedTooltipMessage = "Connection Status: Connected";
	const char* kConnectionStatusDisconnectedTooltipMessage = "Connection Status: Disconnected";
}

namespace AK::WwiseTransfer
{
	namespace WwiseOriginalsComponentConstants
	{
		constexpr int margin = 10;
		constexpr int spacing = 4;
		constexpr int editorBoxHeight = 26;
		constexpr int labelWidth = 122;
		constexpr int wildcardsButtonWidth = 80;
		constexpr int smallButtonWidth = 26;
	} // namespace WwiseOriginalsComponentConstants

	OriginalsSubfolderComponent::OriginalsSubfolderComponent(juce::ValueTree appState, const juce::String& applicationName, ApplicationProperties& applicationProperties, WaapiClientWatcher& waapiCW)
		: applicationState(appState)
		, projectPath(applicationState, IDs::projectPath, nullptr)
		, originalsSubfolder(applicationState, IDs::originalsSubfolder, nullptr)
		, fileBrowserButton("FileBrowserButton", juce::Drawable::createFromImageData(BinaryData::General_FolderWithTriangle_Normal_svg, BinaryData::General_FolderWithTriangle_Normal_svgSize))
		, aboutButton("AboutButton", juce::Drawable::createFromImageData(BinaryData::Dialog_Help_Active_png, BinaryData::Dialog_Help_Active_pngSize))
		, crossMachineTransferSettingsButton("crossMachineTransferSettingsButton", juce::Drawable::createFromImageData(BinaryData::Dialog_Settings_Active_png, BinaryData::Dialog_Settings_Active_pngSize))
		, originalsFolder(applicationState, IDs::originalsFolder, nullptr)
		, languageSubfolder(applicationState, IDs::languageSubfolder, nullptr)
		, aboutComponent(applicationName)
		, crossMachineTransferSettingsComponent(applicationName, applicationProperties, waapiCW)
	{
		projectPathLabel.setText("Project Path", juce::dontSendNotification);
		projectPathLabel.setBorderSize(juce::BorderSize(0));
		projectPathLabel.setMinimumHorizontalScale(1.0f);
		projectPathLabel.setJustificationType(juce::Justification::right);

		originalsSubfolderLabel.setText("Originals Subfolder", juce::dontSendNotification);
		originalsSubfolderLabel.setBorderSize(juce::BorderSize(0));
		originalsSubfolderLabel.setMinimumHorizontalScale(1.0f);
		originalsSubfolderLabel.setJustificationType(juce::Justification::right);

		projectPathEditor.setFont(CustomLookAndFeelConstants::smallFontSize);
		projectPathEditor.getValueToTruncate().referTo(projectPath.getPropertyAsValue());
		projectPathEditor.setReadOnly(true);
		projectPathEditor.setMouseClickGrabsKeyboardFocus(false);
		projectPathEditor.setMouseCursor(juce::MouseCursor::NormalCursor);

		setImageFromRawData(connectedStatusImage, BinaryData::Success_svg, BinaryData::Success_svgSize);
		setImageFromRawData(disconnectedStatusImage, BinaryData::Error_svg, BinaryData::Error_svgSize);
		connectionStatusIcon.setImage(disconnectedStatusImage);
		connectionStatusIcon.setTooltip(kConnectionStatusDisconnectedTooltipMessage);

		originalsSubfolderEditor.setFont(CustomLookAndFeelConstants::smallFontSize);
		originalsSubfolderEditor.getTextValue().referTo(originalsSubfolder.getPropertyAsValue());
		originalsSubfolderEditor.getValidationValue().referTo(applicationState.getPropertyAsValue(IDs::originalsSubfolderValid, nullptr));
		originalsSubfolderEditor.getErrorMessageValue().referTo(applicationState.getPropertyAsValue(IDs::originalsSubfolderErrorMessage, nullptr));

		aboutButton.setTooltip("About ReaWwise");
		aboutButton.onClick = [this]
		{
			showAboutWindow();
		};

		crossMachineTransferSettingsButton.setTooltip("Cross Machine Transfer Settings");
		crossMachineTransferSettingsButton.onClick = [this]
		{
			showCrossMachineTransferSettingsWindow();
		};

		fileBrowserButton.onClick = [this]
		{
			selectOriginalsSubfoler();
		};

		wildcardSelector.onItemSelected = [this](const juce::String& wildcard)
		{
			onWildcardSelected(wildcard);
		};

		addAndMakeVisible(projectPathLabel);
		addAndMakeVisible(projectPathEditor);
		addAndMakeVisible(originalsSubfolderLabel);
		addAndMakeVisible(originalsSubfolderEditor);
		addAndMakeVisible(fileBrowserButton);
		addAndMakeVisible(wildcardSelector);
		addAndMakeVisible(aboutButton);
		addAndMakeVisible(crossMachineTransferSettingsButton);
		addAndMakeVisible(connectionStatusIcon);

		refreshComponent();

		applicationState.addListener(this);
	}

	OriginalsSubfolderComponent::~OriginalsSubfolderComponent()
	{
		applicationState.removeListener(this);
	}

	void OriginalsSubfolderComponent::resized()
	{
		using namespace WwiseOriginalsComponentConstants;

		auto area = getLocalBounds();

		auto projectPathSection = area.removeFromTop(editorBoxHeight);
		{
			projectPathLabel.setBounds(projectPathSection.removeFromLeft(labelWidth));
			projectPathSection.removeFromLeft(margin);

			aboutButton.setBounds(projectPathSection.removeFromRight(smallButtonWidth));
			projectPathSection.removeFromRight(spacing);

			crossMachineTransferSettingsButton.setBounds(projectPathSection.removeFromRight(smallButtonWidth));
			projectPathSection.removeFromRight(spacing);

			auto iconArea = projectPathSection.removeFromRight(smallButtonWidth);
			connectionStatusIcon.setBounds(iconArea.reduced(4, 4)); // Make it a bit smaller for a better look.
			projectPathSection.removeFromRight(spacing);

			projectPathEditor.setBounds(projectPathSection);
		}

		area.removeFromTop(margin);

		auto originalsSubfolderSection = area.removeFromTop(editorBoxHeight);
		{
			originalsSubfolderLabel.setBounds(originalsSubfolderSection.removeFromLeft(labelWidth));

			wildcardSelector.setBounds(originalsSubfolderSection.removeFromRight(wildcardsButtonWidth));
			originalsSubfolderSection.removeFromRight(spacing);

			fileBrowserButton.setBounds(originalsSubfolderSection.removeFromRight(smallButtonWidth));
			originalsSubfolderSection.removeFromRight(spacing);

			originalsSubfolderSection.removeFromLeft(margin);
			originalsSubfolderEditor.setBounds(originalsSubfolderSection);
		}
	}

	void OriginalsSubfolderComponent::refreshComponent()
	{
		auto projectPathEmpty = projectPath.get().isEmpty();
		auto originalsFolderEmpty = originalsFolder.get().isEmpty();

		projectPathLabel.setEnabled(!projectPathEmpty);
		projectPathEditor.setAlpha(!projectPathEmpty ? 1 : 0.5f);

		fileBrowserButton.setEnabled(!projectPathEmpty && !originalsFolderEmpty);
		fileBrowserButton.setTooltip(originalsFolderEmpty ? "File browser is only available when connected to Wwise 2022+" : "Browse");
	}

	void OriginalsSubfolderComponent::selectOriginalsSubfoler()
	{
		fileChooser = std::make_unique<juce::FileChooser>("Please select a subfolder relative to the Wwise originals folder...", juce::File(originalsFolder + languageSubfolder), "*");

		auto folderChooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories;

		fileChooser->launchAsync(folderChooserFlags, [this](const juce::FileChooser& chooser)
			{
				juce::File relativeFolder(chooser.getResult());

				originalsSubfolder = relativeFolder.getRelativePathFrom(juce::File(originalsFolder + languageSubfolder));
			});
	}

	void OriginalsSubfolderComponent::onWildcardSelected(const juce::String& wildcard)
	{
		originalsSubfolder = originalsSubfolderEditor.getText() + wildcard;
	}

	void OriginalsSubfolderComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		auto treeType = treeWhosePropertyHasChanged.getType();

		if(treeType == IDs::application)
		{
			if (property == IDs::projectPath || treeType == IDs::application && property == IDs::originalsFolder)
			{
				triggerAsyncUpdate();
			}
			else if (property == IDs::waapiConnected)
			{
				bool waapiConnected = applicationState.getProperty(IDs::waapiConnected);
				if (waapiConnected)
				{
					connectionStatusIcon.setImage(connectedStatusImage);
					connectionStatusIcon.setTooltip(kConnectionStatusConnectedTooltipMessage);
				}
				else
				{
					connectionStatusIcon.setImage(disconnectedStatusImage);
					connectionStatusIcon.setTooltip(kConnectionStatusDisconnectedTooltipMessage);
				}
			}
		}
		
	}

	void OriginalsSubfolderComponent::handleAsyncUpdate()
	{
		refreshComponent();
	}
	void OriginalsSubfolderComponent::showAboutWindow()
	{
		juce::DialogWindow::LaunchOptions options;
		options.dialogTitle = "About ReaWwise";
		options.useNativeTitleBar = true;
		options.resizable = false;
		options.componentToCentreAround = this->getParentComponent()->getParentComponent();
		options.content = juce::OptionalScopedPointer<juce::Component>(&aboutComponent, false);

		options.launchAsync();
	}
	void OriginalsSubfolderComponent::showCrossMachineTransferSettingsWindow()
	{
		juce::DialogWindow::LaunchOptions options;
		options.dialogTitle = "WAAPI Network Transfer Settings";
		options.useNativeTitleBar = true;
		options.resizable = false;
		options.componentToCentreAround = this->getParentComponent()->getParentComponent();
		options.content = juce::OptionalScopedPointer<juce::Component>(&crossMachineTransferSettingsComponent, false);

		options.launchAsync();
	}
} // namespace AK::WwiseTransfer
