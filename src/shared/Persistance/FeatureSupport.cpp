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

#include "FeatureSupport.h"

#include "Helpers/WwiseHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer
{
	FeatureSupport::FeatureSupport(juce::ValueTree appState, WaapiClient& waapiClient)
		: applicationState(appState)
		, version(applicationState.getChildWithName(IDs::version))
		, waapiClient(waapiClient)
	{
		auto featureSupport = applicationState.getChildWithName(IDs::featureSupport);
		selectObjectsOnImportCommand.referTo(featureSupport, IDs::selectObjectsOnImportCommand, nullptr);
		applyTemplateFeatureEnabled.referTo(featureSupport, IDs::applyTemplateFeatureEnabled, nullptr);
		undoGroupFeatureEnabled.referTo(featureSupport, IDs::undoGroupFeatureEnabled, nullptr);
		waqlEnabled.referTo(featureSupport, IDs::waqlEnabled, nullptr);
		additionalProjectInfoLookupEnabled.referTo(featureSupport, IDs::additionalProjectInfoLookupEnabled, nullptr);
		newObjectNamesEnabled.referTo(featureSupport, IDs::newObjectNamesEnabled, nullptr);

		applicationState.addListener(this);
	}

	FeatureSupport::~FeatureSupport()
	{
		applicationState.removeListener(this);
	}

	void FeatureSupport::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged,
		const juce::Identifier& property)
	{
		if(treeWhosePropertyHasChanged.getType() == IDs::application && property == IDs::waapiConnected)
		{
			auto onGetVersionAsync = [this](const auto& response)
			{
				using namespace FeatureSupportConstants;

				auto versionValueTree = WwiseHelper::versionToValueTree(response.result);

				version.copyPropertiesAndChildrenFrom(versionValueTree, nullptr);

				selectObjectsOnImportCommand = response.result >= v2022_1_0_0 ? FindInProjectExplorerSelectionChannel1 : FindInProjectExplorerSyncGroup1;
				applyTemplateFeatureEnabled = response.result >= v2022_1_0_0;
				undoGroupFeatureEnabled = response.result >= v2021_1_10_0;
				waqlEnabled = response.result >= v2021_1_0_0;
				additionalProjectInfoLookupEnabled = response.result >= v2022_1_0_0;
				newObjectNamesEnabled = response.result >= v2025_1_0_0;

				triggerListenerOnVersionSensitiveProperties();
			};

			waapiClient.getVersionAsync(onGetVersionAsync);
		}
	}

	void FeatureSupport::triggerListenerOnVersionSensitiveProperties()
	{
		applicationState.sendPropertyChangeMessage(IDs::importDestination);
	}
} // namespace AK::WwiseTransfer
