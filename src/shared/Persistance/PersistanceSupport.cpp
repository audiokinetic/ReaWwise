#include "PersistanceSupport.h"

#include "Model/IDs.h"

namespace AK::WwiseTransfer
{
	namespace PersistanceSupportConstants
	{
		const std::initializer_list<juce::Identifier> fieldsToPersist{IDs::originalsSubfolder, IDs::importDestination,
			IDs::originalsSubfolder, IDs::containerNameExists, IDs::applyTemplate};

		const std::initializer_list<juce::Identifier> hierarchyMappingNodeFieldsToPersist{IDs::objectName, IDs::objectType, IDs::propertyTemplatePath, IDs::propertyTemplatePathEnabled, IDs::objectLanguage};
	} // namespace PersistanceSupportConstants

	PersistanceSupport::PersistanceSupport(juce::ValueTree appState, DawContext& dawContext)
		: dawContext(dawContext)
		, applicationState(appState)
	{
		applicationState.addListener(this);
	}

	PersistanceSupport::~PersistanceSupport()
	{
		applicationState.removeListener(this);
	}

	void PersistanceSupport::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		using namespace PersistanceSupportConstants;

		juce::String projectId = applicationState[IDs::projectId];

		auto treeType = treeWhosePropertyHasChanged.getType();

		if(treeType == IDs::application && property == IDs::sessionName)
		{
			juce::ValueTree savedState;

			juce::String cacheKey = dawContext.getSessionName();

			auto it = stateCache.find(cacheKey);

			if(it != stateCache.end())
			{
				savedState = it->second;
			}
			else
			{
				savedState = dawContext.retrieveState();
				if(savedState.isValid())
					stateCache[cacheKey] = savedState;
			}

			if(savedState.isValid())
			{
				// Only apply fields we care about in case user added/tampered with state fields
				for(const auto& field : fieldsToPersist)
				{
					if(savedState.hasProperty(field))
						applicationState.setPropertyExcludingListener(this, field, savedState[field], nullptr);
				}

				auto savedHierarchyMapping = savedState.getChildWithName(IDs::hierarchyMapping);

				if(savedHierarchyMapping.isValid())
				{
					juce::ValueTree sanitizedHiearchyMapping(IDs::hierarchyMapping);

					for(int i = 0; i < savedHierarchyMapping.getNumChildren(); ++i)
					{
						juce::ValueTree sanitizedHierarchyMappingNode(IDs::hierarchyMappingNode);

						for(const auto& field : hierarchyMappingNodeFieldsToPersist)
							sanitizedHierarchyMappingNode.setPropertyExcludingListener(this, field, savedHierarchyMapping.getChild(i)[field], nullptr);

						sanitizedHiearchyMapping.addChild(sanitizedHierarchyMappingNode, i, nullptr);
					}

					auto existingHierarchyMapping = applicationState.getChildWithName(IDs::hierarchyMapping);

					// Since there is no way to exclude this as a listener for copyPropertiesAndChildrenFrom, remove this as listener temporarily
					applicationState.removeListener(this);
					existingHierarchyMapping.copyPropertiesAndChildrenFrom(sanitizedHiearchyMapping, nullptr);
					applicationState.addListener(this);
				}
			}
		}
		else if(treeType == IDs::application && std::find(fieldsToPersist.begin(), fieldsToPersist.end(), property) != fieldsToPersist.end() ||
				treeType == IDs::hierarchyMappingNode && std::find(hierarchyMappingNodeFieldsToPersist.begin(), hierarchyMappingNodeFieldsToPersist.end(), property) != hierarchyMappingNodeFieldsToPersist.end())
		{
			saveState();
		}
	}

	void PersistanceSupport::valueTreeChildRemoved(juce::ValueTree& parent, juce::ValueTree& child, int indexOfChild)
	{
		juce::ignoreUnused(child, indexOfChild);

		if(parent.getType() == IDs::hierarchyMapping)
			saveState();
	}

	void PersistanceSupport::valueTreeChildOrderChanged(juce::ValueTree& parent, int oldIndex, int newIndex)
	{
		juce::ignoreUnused(oldIndex, newIndex);

		if(parent.getType() == IDs::hierarchyMapping)
			saveState();
	}

	void PersistanceSupport::saveState()
	{
		// For reasons of simplicity, we store the whole state everytime. It would be possible to retrieve the current state,
		// and only modify the value that has changed. It would be more complicated and due to the fact that the size of the state is quite small,
		// not provide much performance gain.
		using namespace PersistanceSupportConstants;

		juce::String projectId = applicationState[IDs::projectId];

		if(projectId.isNotEmpty())
		{
			juce::ValueTree stateToBeSaved(IDs::application);

			for(const auto& field : fieldsToPersist)
				stateToBeSaved.setPropertyExcludingListener(this, field, applicationState[field], nullptr);

			juce::ValueTree hierarchyMappingToBeSaved(IDs::hierarchyMapping);
			auto hierarchyMapping = applicationState.getChildWithName(IDs::hierarchyMapping).createCopy();

			for(int i = 0; i < hierarchyMapping.getNumChildren(); ++i)
			{
				juce::ValueTree hierarchyMappingNodeToBeSaved(IDs::hierarchyMappingNode);

				for(const auto& field : hierarchyMappingNodeFieldsToPersist)
					hierarchyMappingNodeToBeSaved.setPropertyExcludingListener(this, field, hierarchyMapping.getChild(i)[field], nullptr);

				hierarchyMappingToBeSaved.addChild(hierarchyMappingNodeToBeSaved, i, nullptr);
			}

			stateToBeSaved.appendChild(hierarchyMappingToBeSaved, nullptr);

			juce::ValueTree savedState = dawContext.retrieveState();

			if(!stateToBeSaved.isEquivalentTo(savedState))
			{
				dawContext.saveState(stateToBeSaved);
				stateCache[dawContext.getSessionName()] = stateToBeSaved;
			}
		}
	}
} // namespace AK::WwiseTransfer
