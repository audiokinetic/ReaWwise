#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer::IDs
{
	inline const juce::Identifier application = "application";
	inline const juce::Identifier waapiConnected = "waapiConnected";
	inline const juce::Identifier projectPath = "projectPath";
	inline const juce::Identifier projectId = "projectId";

	inline const juce::Identifier originalsFolder = "originalsFolder";
	inline const juce::Identifier originalsSubfolder = "originalsSubfolder";
	inline const juce::Identifier originalsSubfolderValid = "originalsSubfolderValid";
	inline const juce::Identifier originalsSubfolderErrorMessage = "originalsSubfolderErrorMessage";
	inline const juce::Identifier languageSubfolder = "languageSubfolder";

	inline const juce::Identifier importDestination = "importDestination";
	inline const juce::Identifier importDestinationType = "importDestinationType";
	inline const juce::Identifier importDestinationValid = "importDestinationValid";
	inline const juce::Identifier importDestinationErrorMessage = "importDestinationErrorMessage";

	inline const juce::Identifier previewItems = "previewItems";
	inline const juce::Identifier previewItem = "previewItem";

	inline const juce::Identifier selectedRow = "selectedRow";
	inline const juce::Identifier hierarchyMapping = "hierarchyMapping";
	inline const juce::Identifier hierarchyMappingNode = "hierarchyMappingNode";

	inline const juce::Identifier objectType = "objectType";
	inline const juce::Identifier objectTypeValid = "objectTypeValid";
	inline const juce::Identifier objectTypeErrorMessage = "objectTypeErrorMessage";
	inline const juce::Identifier objectPath = "objectPath";
	inline const juce::Identifier objectName = "objectName";
	inline const juce::Identifier objectNameValid = "objectNameValid";
	inline const juce::Identifier objectNameErrorMessage = "objectNameErrorMessage";
	inline const juce::Identifier objectStatus = "objectStatus";
	inline const juce::Identifier objectLanguage = "objectLanguage";

	inline const juce::Identifier audioFilePath = "audioFilePath";
	inline const juce::Identifier renderFilePath = "renderFilePath";
	inline const juce::Identifier wavStatus = "wavStatus";

	inline const juce::Identifier propertyTemplatePath = "propertyTemplatePath";
	inline const juce::Identifier propertyTemplatePathType = "propertyTemplatePathType";
	inline const juce::Identifier propertyTemplatePathValid = "propertyTemplatePathValid";
	inline const juce::Identifier propertyTemplatePathErrorMessage = "propertyTemplatePathErrorMessage";
	inline const juce::Identifier propertyTemplatePathEnabled = "propertyTemplatePathEnabled";

	inline const juce::Identifier containerNameExists = "containerNameExists";
	inline const juce::Identifier audioFileNameExists = "audioFileNameExists";
	inline const juce::Identifier applyTemplate = "applyTemplate";

	inline const juce::Identifier version = "version";
	inline const juce::Identifier year = "year";
	inline const juce::Identifier major = "major";
	inline const juce::Identifier minor = "minor";
	inline const juce::Identifier build = "build";

	inline const juce::Identifier featureSupport = "featureSupport";
	inline const juce::Identifier selectObjectsOnImportCommand = "selectObjectsOnImportCommand";
	inline const juce::Identifier applyTemplateFeatureEnabled = "applyTemplateFeatureEnabled";
	inline const juce::Identifier undoGroupFeatureEnabled = "undoGroupFeatureEnabled";
	inline const juce::Identifier waqlEnabled = "waqlEnabled";
	inline const juce::Identifier originalsFolderLookupEnabled = "originalsFolderLookupEnabled";

	inline const juce::Identifier wwiseObjectsChanged = "wwiseObjectsChanged";

	inline const juce::Identifier previewLoading = "previewLoading";

	inline const juce::Identifier sessionName = "sessionName";

	inline const juce::Identifier collapsedUI = "collapsedUI";

	inline const juce::Identifier languages = "languages";
	inline const juce::Identifier language = "language";
	inline const juce::Identifier languageName = "languageName";
} // namespace AK::WwiseTransfer::IDs
