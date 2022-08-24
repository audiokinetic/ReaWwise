#pragma once

#include "FeatureSupport.h"
#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"
#include "Model/Import.h"
#include "Model/Wwise.h"

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer::ApplicationState
{
	inline juce::ValueTree create()
	{
		auto applicationState = juce::ValueTree(IDs::application);
		applicationState.setProperty(IDs::projectPath, "", nullptr);
		applicationState.setProperty(IDs::projectId, "", nullptr);
		applicationState.setProperty(IDs::originalsSubfolder, "", nullptr);
		applicationState.setProperty(IDs::originalsSubfolderValid, true, nullptr);
		applicationState.setProperty(IDs::originalsSubfolderErrorMessage, "", nullptr);
		applicationState.setProperty(IDs::originalsFolder, "", nullptr);
		applicationState.setProperty(IDs::languageSubfolder, "SFX", nullptr);

		applicationState.setProperty(IDs::importDestination, "\\Actor-Mixer Hierarchy\\Default Work Unit", nullptr);
		applicationState.setProperty(IDs::importDestinationValid, true, nullptr);
		applicationState.setProperty(IDs::importDestinationErrorMessage, "", nullptr);
		applicationState.setProperty(IDs::importDestinationType, juce::VariantConverter<Wwise::ObjectType>::toVar(Wwise::ObjectType::WorkUnit), nullptr);

		applicationState.setProperty(IDs::containerNameExists, juce::VariantConverter<Import::ContainerNameExistsOption>::toVar(Import::ContainerNameExistsOption::UseExisting), nullptr);
		applicationState.setProperty(IDs::audioFileNameExists, juce::VariantConverter<Import::AudioFilenameExistsOption>::toVar(Import::AudioFilenameExistsOption::Replace), nullptr);
		applicationState.setProperty(IDs::applyTemplate, juce::VariantConverter<Import::ApplyTemplateOption>::toVar(Import::ApplyTemplateOption::NewObjectCreationOnly), nullptr);

		applicationState.setProperty(IDs::wwiseObjectsChanged, false, nullptr);

		auto version = juce::ValueTree(IDs::version);
		version.setProperty(IDs::year, 0, nullptr);
		version.setProperty(IDs::major, 0, nullptr);
		version.setProperty(IDs::minor, 0, nullptr);
		version.setProperty(IDs::build, 0, nullptr);
		applicationState.appendChild(version, nullptr);

		auto featureSupport = juce::ValueTree(IDs::featureSupport);

		featureSupport.setProperty(IDs::selectObjectsOnImportCommand, FeatureSupportConstants::FindInProjectExplorerSelectionChannel1, nullptr);
		featureSupport.setProperty(IDs::applyTemplateFeatureEnabled, false, nullptr);
		featureSupport.setProperty(IDs::undoGroupFeatureEnabled, false, nullptr);
		featureSupport.setProperty(IDs::waqlEnabled, false, nullptr);
		featureSupport.setProperty(IDs::originalsFolderLookupEnabled, false, nullptr);

		applicationState.appendChild(featureSupport, nullptr);

		auto previewItems = juce::ValueTree(IDs::previewItems);
		applicationState.appendChild(previewItems, nullptr);

		auto physicalFolder = Import::HierarchyMappingNode("$project", Wwise::ObjectType::PhysicalFolder);
		auto soundSfx = Import::HierarchyMappingNode("$region", Wwise::ObjectType::SoundSFX);

		auto hierarchyMapping = juce::ValueTree(IDs::hierarchyMapping);
		hierarchyMapping.setProperty(IDs::selectedRow, 0, nullptr);
		hierarchyMapping.appendChild(ImportHelper::hierarchyMappingNodeToValueTree(physicalFolder), nullptr);
		hierarchyMapping.appendChild(ImportHelper::hierarchyMappingNodeToValueTree(soundSfx), nullptr);

		applicationState.appendChild(hierarchyMapping, nullptr);

		applicationState.setProperty(IDs::previewLoading, false, nullptr);

		applicationState.setProperty(IDs::sessionName, "", nullptr);

		applicationState.setProperty(IDs::collapsedUI, false, nullptr);

		auto languages = juce::ValueTree(IDs::languages);
		applicationState.appendChild(languages, nullptr);

		// Returning by value because juce::ValueTree uses value semantics. No internal copies are made when passing the tree around.
		return applicationState;
	};
} // namespace AK::WwiseTransfer::ApplicationState
