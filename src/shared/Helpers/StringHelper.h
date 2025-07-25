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

#include <algorithm>
#include <iostream>
#include <iterator>
#include <juce_gui_basics/juce_gui_basics.h>
#include <string>
#include <vector>

namespace AK::WwiseTransfer::StringHelper
{
	template <typename CharArray>
	juce::String utf8EncodedCharArrayToString(const CharArray& charArray)
	{
		return {juce::CharPointer_UTF8(&charArray[0]), std::size(charArray)};
	}

	inline std::vector<juce::String> splitDoubleNullTerminatedString(const std::vector<char>& buffer)
	{
		std::vector<juce::String> stringArray;

		std::vector<char> tempBuffer;
		for(const char character : buffer)
		{
			if(character != '\0')
				tempBuffer.push_back(character);
			else if(!tempBuffer.empty())
			{
				stringArray.push_back(utf8EncodedCharArrayToString(tempBuffer));
				tempBuffer.clear();
			}
			else
				break;
		}

		return stringArray;
	}

	inline std::vector<char> createDoubleNullTerminatedStringBuffer(const std::vector<juce::String>& strings)
	{
		std::vector<char> rv;

		if(constexpr bool minimizeAllocations = true)
		{
			std::size_t size = strings.empty() ? 2 : 1;
			std::for_each(strings.cbegin(), strings.cend(), [&size](const juce::String& string)
				{
					auto s = string.length();
					size += s + (s ? 1 : 0);
				});
			rv.reserve(size);
		}

		std::for_each(strings.cbegin(), strings.cend(), [&rv](const juce::String& s)
			{
				if(s.isEmpty())
					return;

				for(const auto& c : s)
					rv.push_back(c);

				rv.push_back('\0');
			});

		if(strings.empty())
			rv.push_back('\0');

		rv.push_back('\0');
		return rv;
	}
} // namespace AK::WwiseTransfer::StringHelper
