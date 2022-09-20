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
		{Wwise::ObjectType::Unknown, "Unknown"}};

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
		Wwise::ObjectType::Unknown};
} // namespace AK::WwiseTransfer::Test
