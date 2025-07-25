/*----------------------------------------------------------------------------------------

Copyright (c) 2025 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

#pragma once

#include "Core/DawContext.h"
#include "Core/WaapiClient.h"
#include "Model/Wwise.h"

#include <juce_data_structures/juce_data_structures.h>

namespace AK::WwiseTransfer::ApplicationState
{
	class Validator
		: juce::ValueTree::Listener
	{
	public:
		Validator(juce::ValueTree appState, WaapiClient& waapiClient);
		~Validator();

	private:
		juce::ValueTree applicationState;
		WaapiClient& waapiClient;

		void valueTreePropertyChanged(juce::ValueTree& valueTree, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parent, juce::ValueTree& child) override;
		void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int indexOfChild) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex) override;

		bool validateImportDestination(const juce::String& importDestination) const;
		bool validateOriginalsSubfolder(const juce::String& originalsFolder, const juce::String& languageSubfolder, const juce::String& originalsSubfolder);
		void validatePropertyTemplatePath(juce::ValueTree hierarchyMappingNode);
		void validateObjectName(juce::ValueTree hierarchyMappingNode);
		void validateHierarchyMapping(Wwise::ObjectType importDestinationType, juce::ValueTree hierarchyMapping);

		bool pathStartsWithAllowedPrefix(const juce::String& path) const;
	};
} // namespace AK::WwiseTransfer::ApplicationState
