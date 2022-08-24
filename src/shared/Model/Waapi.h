#pragma once

#include "AK/WwiseAuthoringAPI/AkAutobahn/AkJson.h"
#include "Helpers/WwiseHelper.h"
#include "Model/Wwise.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <set>

namespace AK::WwiseTransfer::Waapi
{
	struct ImportItemRequest
	{
		juce::String path;
		juce::String originalsSubFolder;
		juce::String renderFilePath;
	};

	struct ProjectInfo
	{
		juce::String projectPath;
		juce::String projectId;
	};

	struct ObjectResponse
	{
		ObjectResponse(AK::WwiseAuthoringAPI::AkJson json)
			: id(json.HasKey("id") ? juce::String(json["id"].GetVariant().GetString()) : "")
			, name(json.HasKey("name") ? json["name"].GetVariant().GetString() : "")
			, type(WwiseHelper::stringToObjectType(json.HasKey("type") ? json["type"].GetVariant().GetString() : ""))
			, path(json.HasKey("path") ? json["path"].GetVariant().GetString() : "")
			, originalWavFilePath(json.HasKey("sound:originalWavFilePath") ? json["sound:originalWavFilePath"].GetVariant().GetString() : "")
		{
		}

		ObjectResponse() = default;

		bool operator==(const ObjectResponse& other) const
		{
			return path == other.path && id == other.id;
		}

		bool operator<(const ObjectResponse& other) const
		{
			return path + id < other.path + other.id;
		}

		juce::String id;
		juce::String name;
		Wwise::ObjectType type;
		juce::String path;
		juce::String originalWavFilePath;
	};

	using ObjectResponseSet = std::set<ObjectResponse>;

	struct PastePropertiesRequest
	{
		juce::String source;
		std::vector<juce::String> targets;
	};

	template <typename Result>
	struct Response
	{
		bool status = false;
		Result result;
		juce::String errorMessage;
	};
} // namespace AK::WwiseTransfer::Waapi
