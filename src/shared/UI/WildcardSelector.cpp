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

#include "WildcardSelector.h"

#include <unordered_map>
#include <vector>

namespace AK::WwiseTransfer
{
	namespace WildcardSelectorConstants
	{
		const std::unordered_map<juce::String, std::vector<juce::String>> wildcards =
			{
				{"Project Settings", {"$project", "$title", "$author", "$track", "$trackslashes",
										 "$tracknumber", "$folders", "$folders[X]", "$parent", "$marker",
										 "$marker(name)", "$marker(name)[s]", "$region", "$region(name)", "$region(name)[s]",
										 "$regionnumber", "$tempo", "$timesignature", "$fx", "$fx[X]"}},
				{"Project Order", {"$filenumber", "$filenumber[N]", "$filecount", "$note", "$note[X]",
									  "$natural", "$namenumber", "$timelineorder", "$timelineorder[N]", "$timelineorder_track",
									  "$timelineorder_track[N]"}},
				{"Media Item Information", {"$item", "$itemnumber", "$itemnotes", "$takemarker"}},
				{"Position/Length", {"$start", "$end", "$length", "$startbeats", "$endbeats",
										"$lengthbeats", "$starttc", "$endtc", "$startframes", "$endframes",
										"$lengthframes", "$startsecods", "$endseconds", "$lengthseconds"}},
				{"Output Format", {"$format", "$samplerate", "$sampleratek", "$bitdepth"}},
				{"Date/Time", {"$date", "$datetime", "$year", "$year2", "$month"
																		"$monthname",
								  "$day", "$dayname", "$hour", "$hour12",
								  "$ampm", "$minute", "$second"}},
				{"Computer Information", {"$user", "$computer"}}};
	}

	WildcardSelector::WildcardSelector()
		: juce::TextButton("Wildcards")
	{
		onClick = [this]
		{
			showWildcardMenu();
		};
	}

	void WildcardSelector::showWildcardMenu()
	{
		using namespace WildcardSelectorConstants;

		auto opts = juce::PopupMenu::Options()
		                .withTargetComponent(this);

		juce::PopupMenu wildcardsMenu;
		wildcardsMenu.setLookAndFeel(&getLookAndFeel());

		for(auto categoryIter = wildcards.begin(); categoryIter != wildcards.end(); ++categoryIter)
		{
			juce::PopupMenu subMenu;

			for(auto wildcardListIter = categoryIter->second.begin(); wildcardListIter != categoryIter->second.end(); ++wildcardListIter)
			{
				auto action = [this, selectedItem = *wildcardListIter]
				{
					if(onItemSelected)
						onItemSelected(selectedItem);
				};

				subMenu.addItem(*wildcardListIter, action);
			}

			wildcardsMenu.addSubMenu(categoryIter->first, subMenu);
		}

		wildcardsMenu.showMenuAsync(opts);
	}
} // namespace AK::WwiseTransfer
