#include "ImportDestinationComponent.h"

#include "BinaryData.h"
#include "Persistance/ApplicationState.h"
#include "Theme/CustomLookAndFeel.h"

namespace AK::WwiseTransfer
{
	namespace ImportDestinationComponentConstants
	{
		constexpr int margin = 10;
		constexpr int spacing = 4;
		constexpr int editorBoxHeight = 26;
		constexpr int labelWidth = 120;
		constexpr int syncButtonWidth = 36;
	} // namespace ImportDestinationComponentConstants

	ImportDestinationComponent::ImportDestinationComponent(juce::ValueTree appState, WaapiClient& waapiClient)
		: applicationState(appState)
		, waapiClient(waapiClient)
		, importDestination(applicationState, IDs::importDestination, nullptr)
		, importDestinationType(applicationState, IDs::importDestinationType, nullptr)
		, projectPath(applicationState, IDs::projectPath, nullptr)
		, updateImportDestinationButton("UpdateImportDestinationButton", juce::Drawable::createFromImageData(BinaryData::General_GetFromWwise_Normal_svg, BinaryData::General_GetFromWwise_Normal_svgSize))
	{
		importDestinationLabel.setText("Import Destination", juce::dontSendNotification);
		importDestinationLabel.setBorderSize(juce::BorderSize(0));
		importDestinationLabel.setMinimumHorizontalScale(1.0f);
		importDestinationLabel.setJustificationType(juce::Justification::right);

		importDestinationEditor.setFont(CustomLookAndFeelConstants::smallFontSize);
		importDestinationEditor.getTextValue().referTo(importDestination.getPropertyAsValue());
		importDestinationEditor.getValidationValue().referTo(applicationState.getPropertyAsValue(IDs::importDestinationValid, nullptr));
		importDestinationEditor.getErrorMessageValue().referTo(applicationState.getPropertyAsValue(IDs::importDestinationErrorMessage, nullptr));

		updateImportDestinationButton.setTooltip("Sync with selected object in Wwise");

		updateImportDestinationButton.onClick = [this]
		{
			updateImportDestination();
		};

		applicationState.addListener(this);

		addAndMakeVisible(importDestinationLabel);
		addAndMakeVisible(importDestinationEditor);
		addAndMakeVisible(updateImportDestinationButton);

		refreshComponent();
	}

	ImportDestinationComponent::~ImportDestinationComponent()
	{
		applicationState.removeListener(this);
	}

	void ImportDestinationComponent::resized()
	{
		using namespace ImportDestinationComponentConstants;

		auto area = getLocalBounds();

		auto ImportDestinationSection = area.removeFromTop(editorBoxHeight);
		{
			importDestinationLabel.setBounds(ImportDestinationSection.removeFromLeft(labelWidth));
			ImportDestinationSection.removeFromLeft(margin);

			updateImportDestinationButton.setBounds(ImportDestinationSection.removeFromRight(syncButtonWidth));
			ImportDestinationSection.removeFromRight(spacing);

			importDestinationEditor.setBounds(ImportDestinationSection);
		}
	}

	void ImportDestinationComponent::refreshComponent()
	{
		auto projectPathEmpty = projectPath.get().isEmpty();

		updateImportDestinationButton.setEnabled(!projectPathEmpty);
	}

	void ImportDestinationComponent::updateImportDestination()
	{
		auto onGetSelectedObject = [this](const Waapi::Response<Waapi::ObjectResponse>& response)
		{
			if(response.result.path.isEmpty())
				return juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Import Destination", "No object is selected in Wwise. Please select one and try again.");

			importDestination = response.result.path;
			importDestinationType = response.result.type;
		};

		waapiClient.getSelectedObjectAsync(onGetSelectedObject);
	}

	void ImportDestinationComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		triggerAsyncUpdate();
	}

	void ImportDestinationComponent::handleAsyncUpdate()
	{
		refreshComponent();
	}
} // namespace AK::WwiseTransfer
