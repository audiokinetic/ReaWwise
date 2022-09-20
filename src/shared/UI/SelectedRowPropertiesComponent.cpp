#include "SelectedRowPropertiesComponent.h"

#include "BinaryData.h"
#include "Helpers/ImportHelper.h"
#include "Helpers/WwiseHelper.h"
#include "Model/IDs.h"
#include "Model/Wwise.h"
#include "Theme/CustomLookAndFeel.h"

namespace AK::WwiseTransfer
{
	namespace SelectedRowPropertiesComponentConstants
	{
		constexpr int margin = 10;
		constexpr int spacing = 4;
		constexpr int bottomMargin = 14;
		constexpr int labelHeight = 26;
		constexpr int labelWidth = 170;
		constexpr int wildcardsButtonWidth = 100;
		constexpr int syncButtonWidth = 36;
		constexpr int propertyTemplateToggleButtonWidth = 22;
		constexpr int lineThickness = 2;
		constexpr std::initializer_list<Wwise::ObjectType> objectTypes{Wwise::ObjectType::ActorMixer, Wwise::ObjectType::BlendContainer,
			Wwise::ObjectType::PhysicalFolder, Wwise::ObjectType::RandomContainer, Wwise::ObjectType::SequenceContainer, Wwise::ObjectType::SoundSFX,
			Wwise::ObjectType::SoundVoice, Wwise::ObjectType::SwitchContainer, Wwise::ObjectType::VirtualFolder, Wwise::ObjectType::WorkUnit};
		const juce::String pastePropertiesToolTip = "Paste properties are only available when connected to Wwise 2022+";
	} // namespace SelectedRowPropertiesComponentConstants

	SelectedRowPropertiesComponent::SelectedRowPropertiesComponent(juce::ValueTree appState, WaapiClient& waapiClient)
		: applicationState(appState)
		, hierarchyMapping(applicationState.getChildWithName(IDs::hierarchyMapping))
		, languages(applicationState.getChildWithName(IDs::languages))
		, selectedRow(hierarchyMapping, IDs::selectedRow, nullptr)
		, emptyHierarchyMappingNode(ImportHelper::hierarchyMappingNodeToValueTree(Import::HierarchyMappingNode("", Wwise::ObjectType::Unknown)))
		, waapiClient(waapiClient)
		, propertyTemplatePathSyncButton("PropertyTemplatePathSyncButton", juce::Drawable::createFromImageData(BinaryData::General_GetFromWwise_Normal_svg, BinaryData::General_GetFromWwise_Normal_svgSize))
	{
		using namespace SelectedRowPropertiesComponentConstants;

		setText("Selected Row Properties");

		auto featureSupport = applicationState.getChildWithName(IDs::featureSupport);
		applyTemplateFeatureEnabled.referTo(featureSupport, IDs::applyTemplateFeatureEnabled, nullptr);

		objectTypeLabel.setText("Object Type", juce::dontSendNotification);
		objectTypeLabel.setBorderSize(juce::BorderSize(0));
		objectTypeLabel.setMinimumHorizontalScale(1.0f);
		objectTypeLabel.setJustificationType(juce::Justification::right);

		objectNameLabel.setText("Object Name", juce::dontSendNotification);
		objectNameLabel.setBorderSize(juce::BorderSize(0));
		objectNameLabel.setMinimumHorizontalScale(1.0f);
		objectNameLabel.setJustificationType(juce::Justification::right);

		propertyTemplatePathLabel.setText("Use Property Template", juce::dontSendNotification);
		propertyTemplatePathLabel.setBorderSize(juce::BorderSize(0));
		propertyTemplatePathLabel.setMinimumHorizontalScale(1.0f);
		propertyTemplatePathLabel.setJustificationType(juce::Justification::right);

		propertyTemplatePathSyncButton.setTooltip("Sync with selected object in Wwise");

		objectNameEditor.setFont(CustomLookAndFeelConstants::smallFontSize);
		propertyTemplatePathEditor.setFont(CustomLookAndFeelConstants::smallFontSize);

		objectTypeComboBox.setTextWhenNothingSelected("Select Object Type");

		for(auto& objectType : objectTypes)
			objectTypeComboBox.addItem(WwiseHelper::objectTypeToReadableString(objectType), static_cast<int>(objectType));

		objectLanguageComboBox.setTextWhenNothingSelected("Language");
		objectLanguageComboBox.setTextWhenNoChoicesAvailable("No Languages loaded");

		addAndMakeVisible(objectTypeLabel);
		addAndMakeVisible(objectNameLabel);
		addAndMakeVisible(propertyTemplatePathLabel);
		addAndMakeVisible(objectTypeComboBox);
		addAndMakeVisible(objectNameEditor);
		addAndMakeVisible(propertyTemplatePathEditor);
		addAndMakeVisible(propertyTemplateToggleButton);
		addAndMakeVisible(wildcardSelector);
		addAndMakeVisible(propertyTemplatePathSyncButton);
		addChildComponent(objectLanguageComboBox);

		refreshComponent();

		objectTypeComboBox.onChange = [this]
		{
			objectType = static_cast<Wwise::ObjectType>(objectTypeComboBox.getSelectedId());
		};

		wildcardSelector.onItemSelected = [this](const juce::String& wildcard)
		{
			objectName = objectName + wildcard;
		};

		propertyTemplatePathSyncButton.onClick = [this]
		{
			updatePropertyTemplatePath();
		};

		objectLanguageComboBox.onChange = [this]
		{
			objectLanguage = objectLanguageComboBox.getText();
		};

		applicationState.addListener(this);
	}

	SelectedRowPropertiesComponent::~SelectedRowPropertiesComponent()
	{
		applicationState.removeListener(this);
	}

	void SelectedRowPropertiesComponent::refreshComponent()
	{
		hierarchyMappingNode = selectedRow >= 0 ? hierarchyMapping.getChild(selectedRow) : emptyHierarchyMappingNode;

		objectName.referTo(hierarchyMappingNode, IDs::objectName, nullptr);
		objectNameValid.referTo(hierarchyMappingNode, IDs::objectNameValid, nullptr);
		objectNameErrorMessage.referTo(hierarchyMappingNode, IDs::objectNameErrorMessage, nullptr);
		objectType.referTo(hierarchyMappingNode, IDs::objectType, nullptr);

		objectLanguage.referTo(hierarchyMappingNode, IDs::objectLanguage, nullptr);
		propertyTemplatePath.referTo(hierarchyMappingNode, IDs::propertyTemplatePath, nullptr);
		propertyTemplatePathEnabled.referTo(hierarchyMappingNode, IDs::propertyTemplatePathEnabled, nullptr);
		propertyTemplatePathValid.referTo(hierarchyMappingNode, IDs::propertyTemplatePathValid, nullptr);
		propertyTemplatePathType.referTo(hierarchyMappingNode, IDs::propertyTemplatePathType, nullptr);

		// Bind components that support cached value binding
		objectNameEditor.getTextValue().referTo(objectName.getPropertyAsValue());
		objectNameEditor.getValidationValue().referTo(objectNameValid.getPropertyAsValue());
		objectNameEditor.getErrorMessageValue().referTo(objectNameErrorMessage.getPropertyAsValue());
		objectTypeComboBox.getSelectedIdAsValue().referTo(objectType.getPropertyAsValue());
		propertyTemplatePathEditor.getTextValue().referTo(propertyTemplatePath.getPropertyAsValue());
		propertyTemplatePathEditor.getValidationValue().referTo(propertyTemplatePathValid.getPropertyAsValue());
		propertyTemplatePathEditor.getErrorMessageValue().referTo(hierarchyMappingNode.getPropertyAsValue(IDs::propertyTemplatePathErrorMessage, nullptr));
		propertyTemplateToggleButton.getToggleStateValue().referTo(propertyTemplatePathEnabled.getPropertyAsValue());

		updatePropertyTemplateSection();
		objectLanguageComboBox.setVisible(objectType == Wwise::ObjectType::SoundVoice);
		objectLanguageComboBox.clear();

		auto languageList = WwiseHelper::valueTreeToLanguages(languages);
		for(int i = 0; i < languageList.size(); ++i)
		{
			objectLanguageComboBox.addItem(languageList[i], i + 1);
			if(languageList[i] == objectLanguage)
				objectLanguageComboBox.setSelectedId(i + 1);
		}

		if(objectLanguage.get().isNotEmpty() && objectLanguageComboBox.getSelectedId() == 0)
			objectLanguageComboBox.setText(objectLanguage.get());
	}

	void SelectedRowPropertiesComponent::updatePropertyTemplatePath()
	{
		// TODO: show loading state
		auto onGetSelectedObject = [this](const Waapi::Response<Waapi::ObjectResponse>& response)
		{
			if(response.result.path.isEmpty())
				return juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Property Template Path", "No object is selected in Wwise. Please select one and try again.");

			propertyTemplatePath = response.result.path;
			propertyTemplatePathType = response.result.type;

			// TODO: end loading state
		};

		waapiClient.getSelectedObjectAsync(onGetSelectedObject);
	}

	void SelectedRowPropertiesComponent::updatePropertyTemplateSection()
	{
		using namespace SelectedRowPropertiesComponentConstants;

		auto isEnabled = propertyTemplatePathEnabled && applyTemplateFeatureEnabled;

		propertyTemplatePathSyncButton.setEnabled(isEnabled);
		propertyTemplatePathEditor.setEnabled(isEnabled);
		propertyTemplatePathEditor.setAlpha(isEnabled ? 1 : 0.5f);
		propertyTemplatePathEditor.setMouseCursor(isEnabled ? juce::MouseCursor::IBeamCursor : juce::MouseCursor::NormalCursor);

		// Not using isEnabled because these fields depend on propertyTemplatePathEnabled.
		propertyTemplateToggleButton.setEnabled(applyTemplateFeatureEnabled);
		propertyTemplatePathLabel.setEnabled(applyTemplateFeatureEnabled);

		propertyTemplatePathEditor.setTooltip(applyTemplateFeatureEnabled ? "" : pastePropertiesToolTip);
		propertyTemplatePathSyncButton.setTooltip(applyTemplateFeatureEnabled ? "Sync with selected object in Wwise" : pastePropertiesToolTip);
		propertyTemplateToggleButton.setTooltip(applyTemplateFeatureEnabled ? "" : pastePropertiesToolTip);
		propertyTemplatePathLabel.setTooltip(applyTemplateFeatureEnabled ? "" : pastePropertiesToolTip);
	}

	void SelectedRowPropertiesComponent::paint(juce::Graphics& g)
	{
		juce::Font f(CustomLookAndFeelConstants::regularFontSize);

		const float textH = f.getHeight();
		const float indent = 3.0f;
		const float textEdgeGap = 4.0f;
		auto constant = 5.0f;

		auto x = indent;
		auto y = f.getAscent() - 3.0f;
		auto w = juce::jmax(0.0f, (float)getWidth() - x * 2.0f);
		auto h = juce::jmax(0.0f, (float)getHeight() - y - indent);
		constant = juce::jmin(constant, w * 0.5f, h * 0.5f);
		auto constant2 = 2.0f * constant;

		auto textW = getText().isEmpty() ? 0
		                                 : juce::jlimit(0.0f,
											   juce::jmax(0.0f, w - constant2 - textEdgeGap * 2),
											   (float)f.getStringWidth(getText()) + textEdgeGap * 2.0f);
		auto textX = constant + textEdgeGap;

		juce::Path p;
		p.startNewSubPath(0, y);
		p.lineTo(constant2, y);

		p.startNewSubPath(x + textX + textW, y);
		p.lineTo(x + w, y);

		auto alpha = 1.0f;

		g.setColour(findColour(juce::GroupComponent::outlineColourId)
						.withMultipliedAlpha(alpha));

		g.strokePath(p, juce::PathStrokeType(2.0f));

		g.setColour(findColour(juce::GroupComponent::textColourId)
						.withMultipliedAlpha(alpha));

		g.setFont(f);

		g.drawText(getText(),
			juce::roundToInt(x + textX), 0,
			juce::roundToInt(textW),
			juce::roundToInt(textH),
			juce::Justification::centred, true);
	}

	void SelectedRowPropertiesComponent::resized()
	{
		using namespace SelectedRowPropertiesComponentConstants;

		auto area = getLocalBounds();
		area.removeFromTop(labelHeight);

		auto objectTypeArea = area.removeFromTop(labelHeight);
		{
			objectTypeLabel.setBounds(objectTypeArea.removeFromLeft(labelWidth));

			objectLanguageComboBox.setBounds(objectTypeArea.removeFromRight(wildcardsButtonWidth));

			objectTypeArea.removeFromRight(spacing);

			objectTypeArea.removeFromLeft(margin);
			objectTypeComboBox.setBounds(objectTypeArea);
		}

		area.removeFromTop(margin);

		auto objectNameArea = area.removeFromTop(labelHeight);
		{
			objectNameLabel.setBounds(objectNameArea.removeFromLeft(labelWidth));

			wildcardSelector.setBounds(objectNameArea.removeFromRight(wildcardsButtonWidth));
			objectNameArea.removeFromRight(spacing);

			objectNameArea.removeFromLeft(margin);
			objectNameEditor.setBounds(objectNameArea);
		}

		area.removeFromTop(margin);

		auto propertyTemplatePathArea = area.removeFromTop(labelHeight);
		{
			propertyTemplateToggleButton.setBounds(propertyTemplatePathArea.removeFromLeft(propertyTemplateToggleButtonWidth));
			propertyTemplatePathLabel.setBounds(propertyTemplatePathArea.removeFromLeft(labelWidth - propertyTemplateToggleButtonWidth));

			propertyTemplatePathArea.removeFromRight(wildcardsButtonWidth - syncButtonWidth);
			propertyTemplatePathSyncButton.setBounds(propertyTemplatePathArea.removeFromRight(syncButtonWidth));
			propertyTemplatePathArea.removeFromRight(spacing);

			propertyTemplatePathArea.removeFromLeft(margin);
			propertyTemplatePathEditor.setBounds(propertyTemplatePathArea);
		}
	}

	void SelectedRowPropertiesComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		auto treeType = treeWhosePropertyHasChanged.getType();

		if(treeType == IDs::hierarchyMapping && property == IDs::selectedRow ||
			treeType == IDs::hierarchyMappingNode && (property == IDs::objectType || property == IDs::propertyTemplatePathEnabled) ||
			treeType == IDs::featureSupport && property == IDs::applyTemplateFeatureEnabled)
		{
			triggerAsyncUpdate();
		}
	}

	void SelectedRowPropertiesComponent::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		auto treeType = parentTree.getType();

		if(treeType == IDs::hierarchyMapping || treeType == IDs::languages)
		{
			triggerAsyncUpdate();
		}
	}

	void SelectedRowPropertiesComponent::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void SelectedRowPropertiesComponent::valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
	{
		if(parentTreeWhoseChildrenHaveMoved.getType() == IDs::hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void SelectedRowPropertiesComponent::handleAsyncUpdate()
	{
		refreshComponent();
	}
} // namespace AK::WwiseTransfer
