#include "ImportConflictsComponent.h"

#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer
{
	namespace ImportConflictsComponentContants
	{
		constexpr int margin = 10;
		constexpr int editorBoxHeight = 26;
		constexpr int labelWidth = 136;
		constexpr std::initializer_list<Import::ContainerNameExistsOption> containerNameExistsOptions{Import::ContainerNameExistsOption::UseExisting,
			Import::ContainerNameExistsOption::CreateNew, Import::ContainerNameExistsOption::Replace};
		constexpr std::initializer_list<Import::ApplyTemplateOption> applyTemplateOptions{Import::ApplyTemplateOption::Always,
			Import::ApplyTemplateOption::NewObjectCreationOnly};
		const juce::String pastePropertiesToolTip = "Paste properties are only available when connected to Wwise 2022+";
	} // namespace ImportConflictsComponentContants

	ImportConflictsComponent::ImportConflictsComponent(juce::ValueTree appState)
		: applicationState(appState)
		, containerNameExists(applicationState, IDs::containerNameExists, nullptr)
		, applyTemplate(applicationState, IDs::applyTemplate, nullptr)
	{
		using namespace ImportConflictsComponentContants;

		auto featureSupport = applicationState.getChildWithName(IDs::featureSupport);
		applyTemplateFeatureEnabled.referTo(featureSupport, IDs::applyTemplateFeatureEnabled, nullptr);

		containerNameExistsLabel.setText("If Sound Name Exists", juce::dontSendNotification);
		containerNameExistsLabel.setBorderSize(juce::BorderSize(0));
		containerNameExistsLabel.setMinimumHorizontalScale(1.0f);
		containerNameExistsLabel.setJustificationType(juce::Justification::right);

		applyTemplateLabel.setText("Apply Template", juce::dontSendNotification);
		applyTemplateLabel.setBorderSize(juce::BorderSize(0));
		applyTemplateLabel.setMinimumHorizontalScale(1.0f);
		applyTemplateLabel.setJustificationType(juce::Justification::right);

		for(auto& containerNameExistsOption : containerNameExistsOptions)
			containerNameExistsComboBox.addItem(ImportHelper::containerNameExistsOptionToReadableString(containerNameExistsOption), static_cast<int>(containerNameExistsOption));

		for(auto& applyTemplateOption : applyTemplateOptions)
			applyTemplateComboBox.addItem(ImportHelper::applyTemplateOptionToReadableString(applyTemplateOption), static_cast<int>(applyTemplateOption));

		containerNameExistsComboBox.getSelectedIdAsValue().referTo(containerNameExists.getPropertyAsValue());
		applyTemplateComboBox.getSelectedIdAsValue().referTo(applyTemplate.getPropertyAsValue());

		auto containerNameExistsComboBoxOnChange = [this]
		{
			const char* tooltip = "";
			switch(static_cast<Import::ContainerNameExistsOption>(containerNameExistsComboBox.getSelectedId()))
			{
			case Import::ContainerNameExistsOption::UseExisting:
				tooltip = "Retains the existing sounds but adds the transferred files as source audio "
						  "files. The new files do not become the main audio sources for the existing "
						  "objects unless they have identical names to the existing sources, and "
						  "therefore overwrite them.";
				break;

			case Import::ContainerNameExistsOption::CreateNew:
				tooltip = "Creates new sounds with increments appended to the names, for example "
						  "SoundName_01. The existing sounds are not affected, unless the existing audio "
						  "sources have the same names as the newly transferred files. In that case, the "
						  "new sources overwrite the existing ones.";
				break;

			case Import::ContainerNameExistsOption::Replace:
				tooltip = "Deletes and recreates the sounds, with the transferred files as sources.";
				break;

			default:
				jassertfalse;
				break;
			}

			containerNameExistsComboBox.setTooltip(tooltip);
		};

		containerNameExistsComboBoxOnChange();
		containerNameExistsComboBox.onChange = containerNameExistsComboBoxOnChange;

		addAndMakeVisible(containerNameExistsLabel);
		addAndMakeVisible(applyTemplateLabel);
		addAndMakeVisible(containerNameExistsComboBox);
		addAndMakeVisible(applyTemplateComboBox);

		refreshComponent();

		applicationState.addListener(this);
	}

	ImportConflictsComponent::~ImportConflictsComponent()
	{
		applicationState.removeListener(this);
	}

	void ImportConflictsComponent::resized()
	{
		using namespace ImportConflictsComponentContants;

		auto area = getLocalBounds();

		auto containerNameSection = area.removeFromTop(editorBoxHeight);
		{
			containerNameExistsLabel.setBounds(containerNameSection.removeFromLeft(labelWidth));
			containerNameSection.removeFromLeft(margin);

			containerNameExistsComboBox.setBounds(containerNameSection);
		}

		area.removeFromTop(margin);

		auto applyTemplateSection = area.removeFromTop(editorBoxHeight);
		{
			applyTemplateLabel.setBounds(applyTemplateSection.removeFromLeft(labelWidth));
			applyTemplateSection.removeFromLeft(margin);

			applyTemplateComboBox.setBounds(applyTemplateSection);
		}
	}

	void ImportConflictsComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		if(treeWhosePropertyHasChanged.getType() == IDs::featureSupport && property == IDs::applyTemplateFeatureEnabled)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportConflictsComponent::handleAsyncUpdate()
	{
		refreshComponent();
	}

	void ImportConflictsComponent::refreshComponent()
	{
		using namespace ImportConflictsComponentContants;

		applyTemplateLabel.setEnabled(applyTemplateFeatureEnabled);
		applyTemplateComboBox.setEnabled(applyTemplateFeatureEnabled);

		applyTemplateLabel.setTooltip(applyTemplateFeatureEnabled ? "" : pastePropertiesToolTip);
		applyTemplateComboBox.setTooltip(applyTemplateFeatureEnabled ? "" : pastePropertiesToolTip);
	}
} // namespace AK::WwiseTransfer
