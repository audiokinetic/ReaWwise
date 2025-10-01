/*----------------------------------------------------------------------------------------

Copyright (c) 2026 AUDIOKINETIC Inc.

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
#include "Model/Import.h"

#include <unordered_map>

namespace AK::WwiseTransfer
{
	class PersistenceSupport
		: juce::ValueTree::Listener
	{
	public:
		PersistenceSupport(juce::ValueTree appState, DawContext& dawContext);
		virtual ~PersistenceSupport();

	private:
		DawContext& dawContext;
		juce::ValueTree applicationState;

		std::unordered_map<juce::String, juce::ValueTree> stateCache;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded) override;
		void valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int indexOfChild) override;
		void valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex) override;

		void saveState();
	};
} // namespace AK::WwiseTransfer
