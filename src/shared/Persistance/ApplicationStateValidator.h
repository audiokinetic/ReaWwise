#pragma once

#include "Core/DawContext.h"
#include "Model/Wwise.h"

#include <juce_data_structures/juce_data_structures.h>

namespace AK::WwiseTransfer::ApplicationState
{
	class Validator
		: juce::ValueTree::Listener
	{
	public:
		Validator(juce::ValueTree appState);
		~Validator();

	private:
		juce::ValueTree applicationState;

		void valueTreePropertyChanged(juce::ValueTree& valueTree, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) override;
		void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int indexOfChild) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex) override;

		bool validateImportDestination(const juce::String& importDestination, Wwise::ObjectType objectType);
		bool validateOriginalsSubfolder(const juce::String& originalsFolder, const juce::String& languageSubfolder, const juce::String& originalsSubfolder);
		void validatePropertyTemplatePath(juce::ValueTree hierarchyMappingNode);
		void validateObjectName(juce::ValueTree hierarchyMappingNode);
		void validateHierarchyMapping(juce::ValueTree hierarchyMapping);
	};
} // namespace AK::WwiseTransfer::ApplicationState
