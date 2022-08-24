#pragma once

#include "Core/DawContext.h"

#include <vector>

namespace AK::WwiseTransfer
{
	class StubContext
		: public DawContext
	{
	public:
		std::vector<Import::Item> getImportItems(Import::Options options) override
		{
			return std::vector<Import::Item>{Import::Item("test", Wwise::ObjectType::SoundSFX, "\\A\\B\\C", juce::String(), juce::String(), juce::String())};
		}

		void renderImportItems() override
		{
		}

		// Inherited via DawContext
		juce::String getSessionName() override
		{
			return juce::String();
		};

		bool saveState(juce::ValueTree applicationState) override
		{
			return true;
		};

		juce::ValueTree retrieveState() override
		{
			return juce::ValueTree();
		}
	};
} // namespace AK::WwiseTransfer
