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

#include "Model/IDs.h"
#include "Model/Wwise.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <regex>

namespace AK::WwiseTransfer::WwiseHelper
{
	inline bool validateObjectTypeParentChildRelationShip(Wwise::ObjectType parent, Wwise::ObjectType child)
	{
		using namespace Wwise;

		switch(parent)
		{
		case ObjectType::BlendContainer:
		case ObjectType::RandomContainer:
		case ObjectType::SequenceContainer:
		case ObjectType::SwitchContainer:
		{
			static constexpr std::initializer_list<ObjectType> children{
				ObjectType::SwitchContainer,
				ObjectType::RandomContainer,
				ObjectType::SequenceContainer,
				ObjectType::BlendContainer,
				ObjectType::SoundSFX,
				ObjectType::SoundVoice,
			};

			return std::find(children.begin(), children.end(), child) != children.end();
		};
		case ObjectType::ActorMixer:
		case ObjectType::VirtualFolder:
		case ObjectType::WorkUnit:
		{
			static constexpr std::initializer_list<ObjectType> children{
				ObjectType::WorkUnit,
				ObjectType::VirtualFolder,
				ObjectType::ActorMixer,
				ObjectType::SwitchContainer,
				ObjectType::RandomContainer,
				ObjectType::SequenceContainer,
				ObjectType::BlendContainer,
				ObjectType::SoundSFX,
				ObjectType::SoundVoice,
			};

			return std::find(children.begin(), children.end(), child) != children.end();
		};
		case ObjectType::PhysicalFolder:
		{
			static constexpr std::initializer_list<ObjectType> children{
				ObjectType::PhysicalFolder,
				ObjectType::WorkUnit,
			};

			return std::find(children.begin(), children.end(), child) != children.end();
		};
		default:
			return false;
		}
	};

	inline juce::String objectTypeToReadableString(Wwise::ObjectType objectType, bool forDisplayPurpose = false)
	{
		using namespace Wwise;

		switch(objectType)
		{
		case ObjectType::ActorMixer:
			return !forDisplayPurpose ? "Actor-Mixer" : "Actor-Mixer / Property Container";
		case ObjectType::AudioFileSource:
			return "Audio File Source";
		case ObjectType::BlendContainer:
			return "Blend Container";
		case ObjectType::PhysicalFolder:
			return "Physical Folder";
		case ObjectType::RandomContainer:
			return "Random Container";
		case ObjectType::SequenceContainer:
			return "Sequence Container";
		case ObjectType::SoundSFX:
			return "Sound SFX";
		case ObjectType::SoundVoice:
			return "Sound Voice";
		case ObjectType::SwitchContainer:
			return "Switch Container";
		case ObjectType::VirtualFolder:
			return "Virtual Folder";
		case ObjectType::WorkUnit:
			return "Work Unit";
		case ObjectType::Sound:
			return "Sound";
		default:
			return "Unknown";
		}
	};

	inline Wwise::ObjectType stringToObjectType(const juce::String& objectTypeAsString)
	{
		using namespace Wwise;

		if(objectTypeAsString == "AudioFileSource" || objectTypeAsString == "Audio File Source")
			return ObjectType::AudioFileSource;
		else if(objectTypeAsString == "ActorMixer" || objectTypeAsString == "Actor Mixer" || objectTypeAsString == "Actor-Mixer" || objectTypeAsString == "Property Container" || objectTypeAsString == "Actor-Mixer / Property Container")
			return ObjectType::ActorMixer;
		else if(objectTypeAsString == "BlendContainer" || objectTypeAsString == "Blend Container")
			return ObjectType::BlendContainer;
		else if(objectTypeAsString == "Folder")
			return ObjectType::VirtualFolder;
		else if(objectTypeAsString == "RandomSequenceContainer" || objectTypeAsString == "Random Container")
			return ObjectType::RandomContainer;
		else if(objectTypeAsString == "SequenceContainer" || objectTypeAsString == "Sequence Container")
			return ObjectType::SequenceContainer;
		else if(objectTypeAsString == "Sound")
			return ObjectType::Sound;
		else if(objectTypeAsString == "SoundSFX" || objectTypeAsString == "Sound SFX")
			return ObjectType::SoundSFX;
		else if(objectTypeAsString == "Sound Voice")
			return ObjectType::SoundVoice;
		else if(objectTypeAsString == "SwitchContainer" || objectTypeAsString == "Switch Container")
			return ObjectType::SwitchContainer;
		else if(objectTypeAsString == "WorkUnit" || objectTypeAsString == "Work Unit")
			return ObjectType::WorkUnit;
		else if(objectTypeAsString == "Virtual Folder")
			return ObjectType::VirtualFolder;
		else if(objectTypeAsString == "Physical Folder")
			return ObjectType::PhysicalFolder;
		else
			return ObjectType::Unknown;
	};

	inline juce::String buildObjectPathNode(Wwise::ObjectType objectType, const juce::String& name)
	{
		return "\\<" + objectTypeToReadableString(objectType) + ">" + name;
	}

	inline juce::String pathToPathWithoutObjectTypes(const juce::String& objectPath)
	{
		static std::regex pattern("<.+?>");
		auto result = std::regex_replace(objectPath.toStdString(), pattern, "");
		return juce::String(result);
	}

	inline std::vector<juce::String> pathToPathParts(const juce::String& objectPath)
	{
		juce::StringArray parts;
		parts.addTokens(objectPath.trimCharactersAtStart("\\"), "\\", "");

		std::vector<juce::String> objectPathParts;
		for(auto& part : parts)
		{
			objectPathParts.emplace_back(part);
		}

		return objectPathParts;
	}

	inline std::vector<juce::String> pathToAncestorPaths(const juce::String& objectPath)
	{
		std::vector<juce::String> ancestors;

		auto parts = pathToPathParts(objectPath);

		juce::String current;
		for(int i = 0; i < parts.size() - 1; ++i)
		{
			current << "\\" << parts[i];
			ancestors.emplace_back(current);
		}

		return ancestors;
	}

	inline juce::String pathToObjectName(const juce::String& objectPath)
	{
		auto lastIndexGreaterThan = objectPath.lastIndexOf(">");
		auto lastIndexOfPathSeperator = objectPath.lastIndexOf("\\");

		if(lastIndexGreaterThan > lastIndexOfPathSeperator)
			return objectPath.substring(lastIndexGreaterThan + 1);
		else
			return objectPath.substring(lastIndexOfPathSeperator + 1);
	}

	inline Wwise::ObjectType pathToObjectType(const juce::String& objectPath)
	{
		if(objectPath == "\\Actor-Mixer Hierarchy" || objectPath == "\\Containers")
			return Wwise::ObjectType::ActorMixer;

		auto lastIndexLessThan = objectPath.lastIndexOf("<");
		auto lastIndexGreaterThan = objectPath.lastIndexOf(">");

		return WwiseHelper::stringToObjectType(objectPath.substring(lastIndexLessThan + 1, lastIndexGreaterThan));
	}

	inline juce::ValueTree versionToValueTree(const Wwise::Version& version)
	{
		juce::ValueTree valueTree(IDs::version);

		valueTree.setProperty(IDs::year, version.year, nullptr);
		valueTree.setProperty(IDs::major, version.major, nullptr);
		valueTree.setProperty(IDs::minor, version.minor, nullptr);
		valueTree.setProperty(IDs::build, version.build, nullptr);

		return valueTree;
	}

	inline Wwise::Version valueTreeToVersion(juce::ValueTree valueTree)
	{
		return {
			valueTree[IDs::year],
			valueTree[IDs::major],
			valueTree[IDs::minor],
			valueTree[IDs::build],
		};
	}

	inline juce::ValueTree languagesToValueTree(const std::vector<juce::String>& languages)
	{
		juce::ValueTree languagesValueTree(IDs::languages);

		for(const auto& language : languages)
		{
			juce::ValueTree languageValueTree(IDs::language);
			languageValueTree.setProperty(IDs::languageName, language, nullptr);

			languagesValueTree.appendChild(languageValueTree, nullptr);
		}

		return languagesValueTree;
	}

	inline std::vector<juce::String> valueTreeToLanguages(juce::ValueTree valueTree)
	{
		std::vector<juce::String> languages;

		for(int i = 0; i < valueTree.getNumChildren(); ++i)
		{
			languages.emplace_back(valueTree.getChild(i)[IDs::languageName].toString());
		}

		return languages;
	}

	inline juce::String getCommonAncestor(const juce::String& firstPath, const juce::String& secondPath)
	{
		auto firstAsParts = WwiseHelper::pathToPathParts(firstPath);
		auto lastAsParts = WwiseHelper::pathToPathParts(secondPath);

		juce::String commonAncestorPath;
		for(int i = 0; i < firstAsParts.size() && i < lastAsParts.size(); ++i)
		{
			if(firstAsParts[i] == lastAsParts[i])
				commonAncestorPath << "\\" << firstAsParts[i];
			else
				break;
		}

		return commonAncestorPath;
	}

	inline bool isPathComplete(const juce::String path)
	{
		for(int i = 1; i < path.length(); ++i)
		{
			if(path[i] == '\\' && (path[i - 1] == '\\' || path[i - 1] == '>'))
				return false;

			if(i == path.length() - 1 && (path[i] == '>' || path[i] == '\\'))
				return false;
		}

		return true;
	}
} // namespace AK::WwiseTransfer::WwiseHelper
