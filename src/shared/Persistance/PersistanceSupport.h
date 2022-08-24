#pragma once

#include "Core/DawContext.h"
#include "Model/Import.h"

#include <unordered_map>

namespace AK::WwiseTransfer
{
	class PersistanceSupport
		: juce::ValueTree::Listener
	{
	public:
		PersistanceSupport(juce::ValueTree appState, DawContext& dawContext);
		virtual ~PersistanceSupport();

	private:
		DawContext& dawContext;
		juce::ValueTree applicationState;

		std::unordered_map<juce::String, juce::ValueTree> stateCache;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int indexOfChild) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex) override;

		void saveState();
	};
} // namespace AK::WwiseTransfer
