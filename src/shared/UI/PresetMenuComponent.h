#pragma once

#include "CustomDrawableButton.h"
#include "Persistance/ApplicationProperties.h"

#include <memory>

namespace AK::WwiseTransfer
{
	class PresetMenuComponent
		: public CustomDrawableButton
	{
	public:
		PresetMenuComponent(juce::ValueTree appState, ApplicationProperties& applicationProperties, const juce::String& applicationName);

	private:
		juce::File presetFolder;
		std::unique_ptr<juce::FileChooser> fileChooser;

		juce::ValueTree hierarchyMapping;

		ApplicationProperties& applicationProperties;

		void showMenu();
		void loadPreset(const juce::File& filename);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PresetMenuComponent)
	};
} // namespace AK::WwiseTransfer
