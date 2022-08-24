#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace AK::WwiseTransfer
{
	class WildcardSelector
		: public juce::TextButton
	{
	public:
		WildcardSelector();

		std::function<void(const juce::String&)> onItemSelected;

	private:
		void showWildcardMenu();

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WildcardSelector);
	};
} // namespace AK::WwiseTransfer
