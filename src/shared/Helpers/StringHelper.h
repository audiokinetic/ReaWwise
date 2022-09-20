#pragma once

#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace AK::WwiseTransfer::StringHelper
{
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
				stringArray.push_back(juce::String(&tempBuffer[0], tempBuffer.size()));
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
