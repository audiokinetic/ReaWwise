/*----------------------------------------------------------------------------------------

Copyright (c) 2023 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#include "PresetMenuComponent.h"

#include "BinaryData.h"
#include "Helpers/PersistanceHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer
{
	namespace PresetMenuComponentConstants
	{
		constexpr auto loadFlags = juce::FileBrowserComponent::openMode |
		                           juce::FileBrowserComponent::canSelectFiles;
		constexpr auto saveFlags = juce::FileBrowserComponent::saveMode |
		                           juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting;
		constexpr auto fileFilter = "*.xml";
	} // namespace PresetMenuComponentConstants

	PresetMenuComponent::PresetMenuComponent(juce::ValueTree appState, ApplicationProperties& applicationProperties, const juce::String& applicationName)
		: CustomDrawableButton("PresetMenuButton", juce::Drawable::createFromImageData(BinaryData::General_FolderWithTriangle_Normal_svg, BinaryData::General_FolderWithTriangle_Normal_svgSize))
		, hierarchyMapping(appState.getChildWithName(IDs::hierarchyMapping))
		, presetFolder(juce::File::getSpecialLocation(juce::File::SpecialLocationType::userApplicationDataDirectory).getChildFile(applicationName).getChildFile("Presets"))
		, applicationProperties(applicationProperties)
	{
		if(!presetFolder.exists())
			presetFolder.createDirectory();

		setTooltip("Manage Presets");

		onClick = [this]
		{
			showMenu();
		};
	}

	void PresetMenuComponent::showMenu()
	{
		using namespace PresetMenuComponentConstants;

		juce::PopupMenu presetMenu;
		presetMenu.setLookAndFeel(&getLookAndFeel());

		auto onSavePreset = [this]
		{
			fileChooser = std::make_unique<juce::FileChooser>("Save preset...", presetFolder, fileFilter);

			auto onFileChosen = [this](const juce::FileChooser& chooser)
			{
				auto file = chooser.getResult();

				auto createResult = file.create();

				if(createResult)
				{
					const auto presetData = PersistanceHelper::hierarchyMappingToPresetData(hierarchyMapping);
					file.replaceWithText(presetData);
					applicationProperties.addRecentHierarchyMappingPreset(file.getFullPathName());
				}
				// An empty path indicates that the file is invalid. This is the behavior for when the user clicks cancel in the file chooser
				// https://docs.juce.com/master/classFile.html#a38506545c1402119b5fef7bcf6289fa9
				else if(!file.getFullPathName().isEmpty())
				{
					juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Save Preset", createResult.getErrorMessage());
				}
			};

			fileChooser->launchAsync(saveFlags, onFileChosen);
		};

		presetMenu.addItem("Save Preset...", onSavePreset);

		auto onLoadPreset = [this]
		{
			fileChooser = std::make_unique<juce::FileChooser>("Select preset...", presetFolder, fileFilter);

			auto onFileChosen = [this](const juce::FileChooser& chooser)
			{
				auto presetFile = chooser.getResult();
				if(presetFile.exists())
					loadPreset(presetFile);
			};

			fileChooser->launchAsync(loadFlags, onFileChosen);
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

		const auto opts = juce::PopupMenu::Options().withTargetComponent(this);
		presetMenu.showMenuAsync(opts);
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
			applicationProperties.removeRecentHierarchyMappingPreset(presetFile.getFullPathName());
		}
	}

} // namespace AK::WwiseTransfer
