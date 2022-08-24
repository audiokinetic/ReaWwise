#include "PresetMenuComponent.h"

#include "BinaryData.h"
#include "Helpers/PersistanceHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer
{
	PresetMenuComponent::PresetMenuComponent(juce::ValueTree appState, ApplicationProperties& applicationProperties, const juce::String& applicationName)
		: CustomDrawableButton("PresetMenuButton", juce::Drawable::createFromImageData(BinaryData::General_FolderWithTriangle_Normal_svg, BinaryData::General_FolderWithTriangle_Normal_svgSize))
		, hierarchyMapping(appState.getChildWithName(IDs::hierarchyMapping))
		, presetFolder(juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile(applicationName).getChildFile("Presets"))
		, applicationProperties(applicationProperties)
	{
		if(!presetFolder.exists())
			presetFolder.createDirectory();

		onClick = [this]
		{
			showMenu();
		};
	}

	void PresetMenuComponent::showMenu()
	{
		auto opts = juce::PopupMenu::Options()
		                .withTargetComponent(this);

		juce::PopupMenu presetMenu;
		presetMenu.setLookAndFeel(&getLookAndFeel());

		auto onSavePreset = [this]
		{
			savePreset();
		};

		presetMenu.addItem("Save Preset...", onSavePreset);

		auto onLoadPreset = [this]
		{
			loadPresetWithFilePicker();
		};

		presetMenu.addItem("Load Preset...", onLoadPreset);

		auto recentPresets = applicationProperties.getRecentHierarchyMappingPresets();

		juce::PopupMenu recentMenu;

		for(const auto& recentPreset : recentPresets)
		{
			auto onRecentPresetSelected = [this, recentPreset = recentPreset]
			{
				loadPreset(recentPreset);
			};

			recentMenu.addItem(recentPreset, onRecentPresetSelected);
		}

		recentMenu.addSeparator();

		auto onClearRecentPresets = [this]
		{
			applicationProperties.clearRecentHierarchyMappingPresets();
		};

		recentMenu.addItem("Clear Recent Presets", onClearRecentPresets);

		presetMenu.addSubMenu("Recent Preset", recentMenu, !recentPresets.isEmpty());

		presetMenu.showMenuAsync(opts);
	}

	void PresetMenuComponent::loadPresetWithFilePicker()
	{
		fileChooser = std::make_unique<juce::FileChooser>("Select preset...", presetFolder, "*.xml");

		auto onFileChosen = [this](const juce::FileChooser& chooser)
		{
			auto presetFile = chooser.getResult();

			if(presetFile.exists())
				loadPreset(presetFile);
		};

		fileChooser->launchAsync(juce::FileBrowserComponent::openMode, onFileChosen);
	}

	void PresetMenuComponent::loadPreset(const juce::File& presetFile)
	{
		auto presetData = presetFile.loadFileAsString();
		auto hierarchyMappingTemp = PersistanceHelper::presetDataToHierarchyMapping(presetData);

		if(hierarchyMappingTemp.isValid())
		{
			hierarchyMapping.copyPropertiesAndChildrenFrom(hierarchyMappingTemp, nullptr);
			applicationProperties.addRecentHierarchyMappingPreset(presetFile.getFullPathName());
		}
		else
		{
			juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Load Preset", "Unable to load preset! The preset file is corrupt.");
		}
	}

	void PresetMenuComponent::savePreset()
	{
		fileChooser = std::make_unique<juce::FileChooser>("Save preset...", presetFolder, "*.xml");

		auto onFileChosen = [this](const juce::FileChooser& chooser)
		{
			auto presetData = PersistanceHelper::hierarchyMappingToPresetData(hierarchyMapping);

			auto file = chooser.getResult();

			file.create();

			file.replaceWithText(presetData);

			applicationProperties.addRecentHierarchyMappingPreset(file.getFullPathName());
		};

		fileChooser->launchAsync(juce::FileBrowserComponent::saveMode, onFileChosen);
	}
} // namespace AK::WwiseTransfer
