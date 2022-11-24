/*----------------------------------------------------------------------------------------

Copyright (c) 2023 AUDIOKINETIC Inc.

This file is licensed to use under the license available at:
https://github.com/audiokinetic/ReaWwise/blob/main/License.txt (the "License").
You may not use this file except in compliance with the License.

Unless required by applicable law or agreed to in writing, software distributed
under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
CONDITIONS OF ANY KIND, either express or implied.  See the License for the
specific language governing permissions and limitations under the License.

----------------------------------------------------------------------------------------*/

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

	struct AdditionalProjectInfo
	{
		juce::String originalsFolder;
		juce::String referenceLanguage;
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
			if(json.HasKey("workunitType"))
			{
				auto workUnitType = json["workunitType"].GetVariant().GetString();

				if(path == "\\Actor-Mixer Hierarchy")
					type = Wwise::ObjectType::ActorMixer;
				else if(workUnitType == "folder")
					type = Wwise::ObjectType::PhysicalFolder;
			}
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
		Wwise::ObjectType type{Wwise::ObjectType::Unknown};
		juce::String path;
		juce::String originalWavFilePath;
	};

	using ObjectResponseSet = std::set<ObjectResponse>;

	struct PastePropertiesRequest
	{
		juce::String source;
		std::vector<juce::String> targets;
	};

	struct Error
	{
		juce::String uri;
		juce::String procedureUri;
		juce::String message;
		juce::String raw;
	};

	template <typename Result>
	struct Response
	{
		bool status = false;
		Result result;
		Error error;
	};

} // namespace AK::WwiseTransfer::Waapi
