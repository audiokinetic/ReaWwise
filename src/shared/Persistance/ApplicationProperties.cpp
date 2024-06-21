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

		const juce::String enableCrossMachineTransferName = "enableCrossMachineTransfer";
		constexpr bool enableCrossMachineTransferValue = false;
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
		juce::String val = getUserSettings()->getValue(waapiIpPropertyName, waapiIpDefaultPropertyValue);
		if(val.isEmpty() || !getIsCrossMachineTransferEnabled())
			return waapiIpDefaultPropertyValue;
		return val;
	}

	void ApplicationProperties::setWaapiIp(const juce::String& ip)
	{
		using namespace ApplicationPropertyConstants;
		if(ip.isEmpty())
			return;
		getUserSettings()->setValue(waapiIpPropertyName, ip);
	}

	int ApplicationProperties::getWaapiPort()
	{
		using namespace ApplicationPropertyConstants;
		int val = getUserSettings()->getIntValue(waapiPortPropertyName, waapiPortDefaultPropertyValue);
		if(val <= 0)
			return waapiPortDefaultPropertyValue;
		return val;
	}

	void ApplicationProperties::setWaapiPort(int port)
	{
		using namespace ApplicationPropertyConstants;
		if(port <= 0)
			return;
		getUserSettings()->setValue(waapiPortPropertyName, port);
	}

	int ApplicationProperties::getPreviewRefreshInterval()
	{
		using namespace ApplicationPropertyConstants;
		return getUserSettings()->getIntValue(previewRefreshIntervalPropertyName, previewRefreshIntervalValue);
	}

	juce::StringArray ApplicationProperties::getRecentHierarchyMappingPresets()
	{
		using namespace ApplicationPropertyConstants;
		auto recentHierarchyMappingPresetsAsString =
			getUserSettings()->getValue(recentHierarchyMappingPresetsPropertyName, recentHierarchyMappingPresetsPropertyValue);

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
		if(recentHierarchyMappingPresets.contains(path))
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
	bool ApplicationProperties::getIsCrossMachineTransferEnabled()
	{
		using namespace ApplicationPropertyConstants;
		return getUserSettings()->getBoolValue(enableCrossMachineTransferName, enableCrossMachineTransferValue);
	}
	void ApplicationProperties::setIsCrossMachineTransferEnabled(bool value)
	{
		using namespace ApplicationPropertyConstants;
		getUserSettings()->setValue(enableCrossMachineTransferName, value);
	}
} // namespace AK::WwiseTransfer
