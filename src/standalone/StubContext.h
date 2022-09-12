#pragma once

#include "Core/DawContext.h"

#include <vector>

namespace AK::WwiseTransfer
{
	class StubContext
		: public DawContext
	{
	public:
		bool sessionChanged() override
		{
			return false;
		}

		std::vector<Import::PreviewItem> getItemsForPreview(const Import::Options& options) override
		{
			return std::vector<Import::PreviewItem>{{"\\A\\B\\C", juce::String(), juce::String()}};
		}

		std::vector<Import::Item> getItemsForImport(const Import::Options& options) override
		{
			return std::vector<Import::Item>{{"\\A\\B\\C", juce::String(), juce::String(), juce::String()}};
		}

		void renderItems() override
		{
		}

		// Inherited via DawContext
		juce::String getSessionName() override
		{
			return juce::String();
		}

		bool saveState(juce::ValueTree applicationState) override
		{
			return true;
		}

		juce::ValueTree retrieveState() override
		{
			return juce::ValueTree();
		}
	};
} // namespace AK::WwiseTransfer
