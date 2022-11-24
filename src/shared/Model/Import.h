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

#include "Model/IDs.h"
#include "Model/Waapi.h"
#include "Model/Wwise.h"

#include <algorithm>

namespace AK::WwiseTransfer::Import
{
	enum class ContainerNameExistsOption : int
	{
		Unknown,
		UseExisting,
		CreateNew,
		Replace
	};

	enum class AudioFilenameExistsOption
	{
		UseExisting = 1,
		Replace
	};

	enum class ApplyTemplateOption
	{
		Always = 1,
		NewObjectCreationOnly
	};

	enum class ObjectStatus
	{
		NoChange,
		Replaced,
		New,
		NewRenamed
	};

	enum class WavStatus
	{
		Unknown,
		Replaced,
		New
	};

	struct PreviewItem
	{
		juce::String path;
		juce::String originalsSubFolder;
		juce::String audioFilePath;
	};

	struct Item : public PreviewItem
	{
		juce::String renderFilePath;
	};

	struct PreviewItemNode
	{
		juce::String name;
		Wwise::ObjectType type{};
		ObjectStatus objectStatus{};
		juce::String audioFilePath;
		WavStatus wavStatus{};
		bool unresolvedWildcard{false};
	};

	struct HierarchyMappingNode
	{
		HierarchyMappingNode(const juce::String& name, bool nameValid, const juce::String& nameErrorMessage, Wwise::ObjectType type, bool typeValid, const juce::String& typeErrorMessage,
			const juce::String& propertyTemplatePath, bool propertyTemplatePathEnabled, bool propertyTemplatePathValid, const juce::String& propertyTemplatePathErrorMessage, const juce::String& language)
			: name(name)
			, nameValid(nameValid)
			, nameErrorMessage(nameErrorMessage)
			, type(type)
			, typeValid(typeValid)
			, typeErrorMessage(typeErrorMessage)
			, propertyTemplatePath(propertyTemplatePath)
			, propertyTemplatePathEnabled(propertyTemplatePathEnabled)
			, propertyTemplatePathValid(propertyTemplatePathValid)
			, propertyTemplatePathErrorMessage(propertyTemplatePathErrorMessage)
			, language(language)
		{
		}

		HierarchyMappingNode(juce::String name, Wwise::ObjectType type)
			: name(name)
			, nameValid(true)
			, type(type)
			, typeValid(true)
			, propertyTemplatePathEnabled(false)
			, propertyTemplatePathValid(true)
		{
		}

		juce::String name;
		bool nameValid{};
		juce::String nameErrorMessage;
		Wwise::ObjectType type{};
		bool typeValid{};
		juce::String typeErrorMessage;
		juce::String propertyTemplatePath;
		bool propertyTemplatePathEnabled{};
		bool propertyTemplatePathValid{};
		juce::String propertyTemplatePathErrorMessage;
		juce::String language;
	};

	struct Options
	{
		Options(const juce::String& importDestination, const juce::String& originalsSubfolder, const juce::String& hierarchyMappingPath)
			: importDestination(importDestination)
			, originalsSubfolder(originalsSubfolder)
			, hierarchyMappingPath(hierarchyMappingPath)
		{
		}

		juce::String importDestination;
		juce::String originalsSubfolder;
		juce::String hierarchyMappingPath;
	};

	struct Summary
	{
		struct Object
		{
			juce::String id;
			Wwise::ObjectType type{};
			juce::String propertyTemplatePath;
			juce::String originalWavFilePath;
			Import::WavStatus wavStatus{};
			Import::ObjectStatus objectStatus{};
		};

		std::map<juce::String, Object> objects;
		std::vector<AK::WwiseTransfer::Waapi::Error> errors;

		using PathObjectPair = std::pair<juce::String, Object>;

		int getNumAudiofilesTransfered() const
		{
			auto predicate = [](const PathObjectPair& pathObjectPair)
			{
				return pathObjectPair.second.type == Wwise::ObjectType::AudioFileSource;
			};
			return std::count_if(objects.begin(), objects.end(), predicate);
		}

		int getNumObjectsCreated() const
		{
			auto predicate = [](const PathObjectPair& pathObjectPair)
			{
				return pathObjectPair.second.objectStatus == Import::ObjectStatus::New;
			};
			return std::count_if(objects.begin(), objects.end(), predicate);
		}

		int getNumObjectTemplatesApplied() const
		{
			auto predicate = [](const PathObjectPair& pathObjectPair)
			{
				return pathObjectPair.second.propertyTemplatePath.isNotEmpty();
			};
			return std::count_if(objects.begin(), objects.end(), predicate);
		}
	};

	namespace Task
	{
		struct Options
		{
			std::vector<Import::Item> importItems;
			Import::ContainerNameExistsOption containerNameExistsOption{};
			Import::ApplyTemplateOption applyTemplateOption{Import::ApplyTemplateOption::Always};
			juce::String importDestination;
			std::vector<Import::HierarchyMappingNode> hierarchyMappingNodeList;
			juce::String originalsFolder;
			juce::String languageSubfolder;
			juce::String selectObjectsOnImportCommand;
			bool applyTemplateFeatureEnabled{false};
			bool undoGroupFeatureEnabled{false};
			bool waqlEnabled{false};
		};
	} // namespace Task
} // namespace AK::WwiseTransfer::Import

template <>
struct juce::VariantConverter<AK::WwiseTransfer::Import::ContainerNameExistsOption>
{
	static AK::WwiseTransfer::Import::ContainerNameExistsOption fromVar(const juce::var& v)
	{
		return static_cast<AK::WwiseTransfer::Import::ContainerNameExistsOption>(int(v));
	}

	static juce::var toVar(AK::WwiseTransfer::Import::ContainerNameExistsOption option)
	{
		return int(option);
	}
};

template <>
struct juce::VariantConverter<AK::WwiseTransfer::Import::AudioFilenameExistsOption>
{
	static AK::WwiseTransfer::Import::AudioFilenameExistsOption fromVar(const juce::var& v)
	{
		return static_cast<AK::WwiseTransfer::Import::AudioFilenameExistsOption>(int(v));
	}

	static juce::var toVar(AK::WwiseTransfer::Import::AudioFilenameExistsOption option)
	{
		return int(option);
	}
};

template <>
struct juce::VariantConverter<AK::WwiseTransfer::Import::ApplyTemplateOption>
{
	static AK::WwiseTransfer::Import::ApplyTemplateOption fromVar(const juce::var& v)
	{
		return static_cast<AK::WwiseTransfer::Import::ApplyTemplateOption>(int(v));
	}

	static juce::var toVar(AK::WwiseTransfer::Import::ApplyTemplateOption option)
	{
		return int(option);
	}
};

template <>
struct juce::VariantConverter<AK::WwiseTransfer::Import::ObjectStatus>
{
	static AK::WwiseTransfer::Import::ObjectStatus fromVar(const juce::var& v)
	{
		return static_cast<AK::WwiseTransfer::Import::ObjectStatus>(int(v));
	}

	static juce::var toVar(AK::WwiseTransfer::Import::ObjectStatus option)
	{
		return int(option);
	}
};

template <>
struct juce::VariantConverter<AK::WwiseTransfer::Import::WavStatus>
{
	static AK::WwiseTransfer::Import::WavStatus fromVar(const juce::var& v)
	{
		return static_cast<AK::WwiseTransfer::Import::WavStatus>(int(v));
	}

	static juce::var toVar(AK::WwiseTransfer::Import::WavStatus option)
	{
		return int(option);
	}
};
