#include "OriginalsSubfolderComponent.h"

#include "BinaryData.h"
#include "Persistance/ApplicationState.h"

namespace AK::WwiseTransfer
{
	namespace WwiseOriginalsComponentConstants
	{
		constexpr int margin = 10;
		constexpr int spacing = 4;
		constexpr int editorBoxHeight = 26;
		constexpr int labelWidth = 120;
		constexpr int wildcardsButtonWidth = 80;
		constexpr int smallButtonWidth = 26;
	} // namespace WwiseOriginalsComponentConstants

	OriginalsSubfolderComponent::OriginalsSubfolderComponent(juce::ValueTree appState, const juce::String& applicationName)
		: applicationState(appState)
		, projectPath(applicationState, IDs::projectPath, nullptr)
		, originalsSubfolder(applicationState, IDs::originalsSubfolder, nullptr)
		, fileBrowserButton("FileBrowserButton", juce::Drawable::createFromImageData(BinaryData::General_FolderWithTriangle_Normal_svg, BinaryData::General_FolderWithTriangle_Normal_svgSize))
		, aboutButton("AboutButton", juce::Drawable::createFromImageData(BinaryData::Dialog_Help_Active_png, BinaryData::Dialog_Help_Active_pngSize))
		, originalsFolder(applicationState, IDs::originalsFolder, nullptr)
		, languageSubfolder(applicationState, IDs::languageSubfolder, nullptr)
		, aboutComponent(applicationName)
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

		originalsSubfolderEditor.setFont(CustomLookAndFeelConstants::smallFontSize);
		originalsSubfolderEditor.getTextValue().referTo(originalsSubfolder.getPropertyAsValue());
		originalsSubfolderEditor.getValidationValue().referTo(applicationState.getPropertyAsValue(IDs::originalsSubfolderValid, nullptr));
		originalsSubfolderEditor.getErrorMessageValue().referTo(applicationState.getPropertyAsValue(IDs::originalsSubfolderErrorMessage, nullptr));

		aboutButton.setTooltip("About ReaWwise");
		aboutButton.onClick = [this]
		{
			showAboutWindow();
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

		if(treeType == IDs::application && property == IDs::projectPath || treeType == IDs::application && property == IDs::originalsFolder)
		{
			triggerAsyncUpdate();
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
} // namespace AK::WwiseTransfer
