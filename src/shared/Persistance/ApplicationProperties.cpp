#include "ApplicationProperties.h"

namespace AK::WwiseTransfer
{
	namespace ApplicationPropertyConstants
	{
		// Potentially grab from local install (future)
		const juce::String waapiIpPropertyName = "waapiIp";
		const juce::String waapiIpDefaultPropertyValue = "127.0.0.1";

		const juce::String waapiPortPropertyName = "waapiPort";
		constexpr int waapiPortDefaultPropertyValue = 8080;

		const juce::String previewRefreshIntervalPropertyName = "previewRefreshInterval";
		constexpr int previewRefreshIntervalValue = 100;

		const juce::String recentHierarchyMappingPresetsPropertyName = "recentHierarchyMappingPresets";
		const juce::String recentHierarchyMappingPresetsPropertyValue = "";

		const juce::String scaleFactorOverridePropertyName = "scaleFactorOverride";
		constexpr double scaleFactorOverridePropertyValue = 0;

		const juce::String showSilentIncrementWarningName = "showSilentIncrementWarning";
		constexpr bool showSilentIncrementWarningValue = true;
	} // namespace ApplicationPropertyConstants

	ApplicationProperties::ApplicationProperties(const juce::String& applicationName)
	{
		juce::PropertiesFile::Options opts;
		opts.applicationName = applicationName;
		opts.filenameSuffix = ".settings";
		opts.folderName = applicationName + "/Settings";
		opts.osxLibrarySubFolder = "Application Support";
		setStorageParameters(opts);

		// Create the settings file the first time. We might want to install a default one though
		using namespace ApplicationPropertyConstants;
		auto file = getUserSettings()->getFile();
		if(!file.exists())
		{
			getUserSettings()->setValue(waapiIpPropertyName, waapiIpDefaultPropertyValue);
			getUserSettings()->setValue(waapiPortPropertyName, waapiPortDefaultPropertyValue);
			getUserSettings()->setValue(previewRefreshIntervalPropertyName, previewRefreshIntervalValue);
			getUserSettings()->save();
		}
	}

	juce::String ApplicationProperties::getWaapiIp()
	{
		using namespace ApplicationPropertyConstants;
		return getUserSettings()->getValue(waapiIpPropertyName, waapiIpDefaultPropertyValue);
	}

	int ApplicationProperties::getWaapiPort()
	{
		using namespace ApplicationPropertyConstants;
		return getUserSettings()->getIntValue(waapiPortPropertyName, waapiPortDefaultPropertyValue);
	}

	int ApplicationProperties::getPreviewRefreshInterval()
	{
		using namespace ApplicationPropertyConstants;
		return getUserSettings()->getIntValue(previewRefreshIntervalPropertyName, previewRefreshIntervalValue);
	}

	juce::StringArray ApplicationProperties::getRecentHierarchyMappingPresets()
	{
		using namespace ApplicationPropertyConstants;
		auto recentHierarchyMappingPresetsAsString = getUserSettings()->getValue(recentHierarchyMappingPresetsPropertyName, recentHierarchyMappingPresetsPropertyValue);

		juce::StringArray recentHierarchyMappingPresets;
		recentHierarchyMappingPresets.addTokens(recentHierarchyMappingPresetsAsString, ";", "");

		return recentHierarchyMappingPresets;
	}

	void ApplicationProperties::addRecentHierarchyMappingPreset(const juce::String& path)
	{
		using namespace ApplicationPropertyConstants;

		auto recentHierarchyMappingPresets = getRecentHierarchyMappingPresets();
		if(recentHierarchyMappingPresets.contains(path))
			recentHierarchyMappingPresets.removeString(path);

		recentHierarchyMappingPresets.insert(0, path);

		const auto recentHierarchyMappingPresetsAsString = recentHierarchyMappingPresets.joinIntoString(";", 0, 10);
		getUserSettings()->setValue(recentHierarchyMappingPresetsPropertyName, recentHierarchyMappingPresetsAsString);
	}

	void ApplicationProperties::removeRecentHierarchyMappingPreset(const juce::String& path)
	{
		using namespace ApplicationPropertyConstants;

		auto recentHierarchyMappingPresets = getRecentHierarchyMappingPresets();
		if (recentHierarchyMappingPresets.contains(path))
			recentHierarchyMappingPresets.removeString(path);

		const auto recentHierarchyMappingPresetsAsString = recentHierarchyMappingPresets.joinIntoString(";", 0, 10);
		getUserSettings()->setValue(recentHierarchyMappingPresetsPropertyName, recentHierarchyMappingPresetsAsString);
	}

	void ApplicationProperties::clearRecentHierarchyMappingPresets()
	{
		using namespace ApplicationPropertyConstants;
		getUserSettings()->setValue(recentHierarchyMappingPresetsPropertyName, "");
	}

	double ApplicationProperties::getScaleFactorOverride()
	{
		using namespace ApplicationPropertyConstants;
		return getUserSettings()->getDoubleValue(scaleFactorOverridePropertyName, scaleFactorOverridePropertyValue);
	}

	bool ApplicationProperties::getShowSilentIncrementWarning()
	{
		using namespace ApplicationPropertyConstants;
		return getUserSettings()->getBoolValue(showSilentIncrementWarningName, showSilentIncrementWarningValue);
	}

	void ApplicationProperties::setShowSilentIncrementWarning(bool value)
	{
		using namespace ApplicationPropertyConstants;
		getUserSettings()->setValue(showSilentIncrementWarningName, value);
	}
} // namespace AK::WwiseTransfer
