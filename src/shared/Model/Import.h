#pragma once

#include "Model/IDs.h"
#include "Model/Wwise.h"

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

	struct Item
	{
		Item(const juce::String& name, Wwise::ObjectType type, const juce::String& path, const juce::String& originalsSubFolder, const juce::String& audioFilePath, const juce::String& renderFilePath)
			: name(name)
			, type(type)
			, path(path)
			, originalsSubFolder(originalsSubFolder)
			, audioFilePath(audioFilePath)
			, renderFilePath(renderFilePath)
		{
		}

		juce::String name;
		Wwise::ObjectType type;
		juce::String path;
		juce::String originalsSubFolder;
		juce::String audioFilePath;
		juce::String renderFilePath;
	};

	struct PreviewItem
	{
		juce::String name;
		Wwise::ObjectType type;
		ObjectStatus objectStatus;
		juce::String audioFilePath;
		WavStatus wavStatus;
	};

	struct PreviewItemNode : public Item
	{
		PreviewItemNode(const Item& item)
			: Item(item.name, item.type, item.path, item.originalsSubFolder, item.audioFilePath, item.renderFilePath)
		{
		}

		std::unordered_map<juce::String, PreviewItemNode> children;
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
		bool nameValid;
		juce::String nameErrorMessage;
		Wwise::ObjectType type;
		bool typeValid;
		juce::String typeErrorMessage;
		juce::String propertyTemplatePath;
		bool propertyTemplatePathEnabled;
		bool propertyTemplatePathValid;
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
			Wwise::ObjectType type;
			juce::String originalWavFilePath;
			juce::String propertyTemplatePath;
			bool newlyCreated{false};
		};

		std::map<juce::String, Object> objects;

		int objectsCreated{0};
		int audioFilesImported{0};
		int objectTemplatesApplied{0};

		juce::String errorMessage;
	};

	namespace Task
	{
		struct Options
		{
			std::vector<Import::Item> importItems;
			Import::ContainerNameExistsOption containerNameExistsOption;
			Import::ApplyTemplateOption applyTemplateOption;
			juce::String importDestination;
			std::vector<Import::HierarchyMappingNode> hierarchyMappingNodeList;
			juce::String selectObjectsOnImportCommand;
			bool applyTemplateFeatureEnabled{false};
			bool undoGroupFeatureEnabled{false};
			bool waqlEnabled{false};
			juce::String objectLanguage;
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
