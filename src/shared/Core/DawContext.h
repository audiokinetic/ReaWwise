#pragma once

#include "Model/Import.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

namespace AK::WwiseTransfer
{
	class DawContext
	{
	public:
		virtual ~DawContext() = default;

		virtual juce::String getSessionName() = 0;
		virtual bool saveState(juce::ValueTree applicationState) = 0;
		virtual juce::ValueTree retrieveState() = 0;
		virtual void renderImportItems() = 0;
		virtual std::vector<Import::Item> getImportItems(Import::Options options) = 0;
	};
} // namespace AK::WwiseTransfer
