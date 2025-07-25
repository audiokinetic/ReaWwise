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

#include <juce_gui_basics/juce_gui_basics.h>

namespace AK::WwiseTransfer::Wwise
{
	enum class ImportObjectType
	{
		SFX,
		Voice,
		Music
	};

	enum class ImportOperation
	{
		CreateNew,
		UseExisting,
		ReplaceExisting
	};

	enum class ObjectType
	{
		Unknown,
		ActorMixer,
		AudioFileSource,
		BlendContainer,
		PhysicalFolder,
		RandomContainer,
		SequenceContainer,
		SoundSFX,
		SoundVoice,
		SwitchContainer,
		VirtualFolder,
		WorkUnit,
		Sound
	};

	struct Version
	{
		int year{};
		int major{};
		int minor{};
		int build{};

		bool operator<(const Version& other) const
		{
			return year < other.year ||
			       (year == other.year && major < other.major) ||
			       (year == other.year && major == other.major && minor < other.minor) ||
			       (year == other.year && major == other.major && minor == other.minor && build < other.build);
		}

		bool operator>(const Version& other) const
		{
			return other < *this;
		}

		bool operator<=(const Version& other) const
		{
			return !(*this > other);
		}

		bool operator>=(const Version& other) const
		{
			return !(*this < other);
		}

		bool operator==(const Version& other) const
		{
			return year == other.year && major == other.major && minor == other.minor && build == other.build;
		}

		bool operator!=(const Version& other) const
		{
			return !(*this == other);
		}

		friend juce::String& operator<<(juce::String& out, const Version& v)
		{
			out << v.year << "." << v.major << "." << v.minor << "." << v.build;
			return out;
		}
	};
} // namespace AK::WwiseTransfer::Wwise

template <>
struct juce::VariantConverter<AK::WwiseTransfer::Wwise::ObjectType>
{
	static AK::WwiseTransfer::Wwise::ObjectType fromVar(const juce::var v)
	{
		return static_cast<AK::WwiseTransfer::Wwise::ObjectType>(int(v));
	}

	static juce::var toVar(AK::WwiseTransfer::Wwise::ObjectType objectType)
	{
		return int(objectType);
	}
};
