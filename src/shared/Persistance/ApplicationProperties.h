#pragma once

#include <juce_data_structures/juce_data_structures.h>

namespace AK::WwiseTransfer
{
	class ApplicationProperties : juce::ApplicationProperties
	{
	public:
		explicit ApplicationProperties(const juce::String& applicationName);

		juce::String getWaapiIp();
		int getWaapiPort();
		int getPreviewRefreshInterval();
		juce::StringArray getRecentHierarchyMappingPresets();
		void addRecentHierarchyMappingPreset(const juce::String& path);
		void removeRecentHierarchyMappingPreset(const juce::String& path);
		void clearRecentHierarchyMappingPresets();
		double getScaleFactorOverride();
		bool getShowSilentIncrementWarning();
		void setShowSilentIncrementWarning(bool value);

	private:
		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ApplicationProperties)
	};
} // namespace AK::WwiseTransfer
