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

#include "Helpers/WwiseHelper.h"
#include "Model/Import.h"

#include <catch2/catch_all.hpp>
#include <catch2/catch_test_macros.hpp>
#include <juce_core/juce_core.h>
#include <memory>

namespace AK::WwiseTransfer::Test
{
	std::unordered_map<Wwise::ObjectType, juce::String> objectTypeStringMap{
		{Wwise::ObjectType::ActorMixer, "Actor-Mixer"},
		{Wwise::ObjectType::AudioFileSource, "Audio File Source"},
		{Wwise::ObjectType::BlendContainer, "Blend Container"},
		{Wwise::ObjectType::PhysicalFolder, "Physical Folder"},
		{Wwise::ObjectType::RandomContainer, "Random Container"},
		{Wwise::ObjectType::SequenceContainer, "Sequence Container"},
		{Wwise::ObjectType::SoundSFX, "Sound SFX"},
		{Wwise::ObjectType::SoundVoice, "Sound Voice"},
		{Wwise::ObjectType::SwitchContainer, "Switch Container"},
		{Wwise::ObjectType::VirtualFolder, "Virtual Folder"},
		{Wwise::ObjectType::WorkUnit, "Work Unit"},
		{Wwise::ObjectType::Sound, "Sound"},
		{Wwise::ObjectType::Unknown, "Unknown"},
	};

	std::vector<Wwise::ObjectType> objectTypes{
		Wwise::ObjectType::ActorMixer,
		Wwise::ObjectType::AudioFileSource,
		Wwise::ObjectType::BlendContainer,
		Wwise::ObjectType::PhysicalFolder,
		Wwise::ObjectType::RandomContainer,
		Wwise::ObjectType::SequenceContainer,
		Wwise::ObjectType::SoundSFX,
		Wwise::ObjectType::SoundVoice,
		Wwise::ObjectType::SwitchContainer,
		Wwise::ObjectType::VirtualFolder,
		Wwise::ObjectType::WorkUnit,
		Wwise::ObjectType::Sound,
		Wwise::ObjectType::Unknown,
	};
} // namespace AK::WwiseTransfer::Test
