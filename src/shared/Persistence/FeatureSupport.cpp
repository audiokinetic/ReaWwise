/*----------------------------------------------------------------------------------------

Copyright (c) 2026 AUDIOKINETIC Inc.

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
		newObjectNamesEnabled.referTo(featureSupport, IDs::newObjectNamesEnabled, nullptr);
		isSegmentUniversal.referTo(featureSupport, IDs::isSegmentUniversal, nullptr);
		isUsingInteractiveMusicHierarchy.referTo(featureSupport, IDs::isUsingInteractiveMusicHierarchy, nullptr);

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

				applicationState.setProperty(IDs::minimumVersionRequirementMet, response.result >= minimum_supported_version, nullptr);

				newObjectNamesEnabled = response.result >= v2025_1_0_0;
				isSegmentUniversal = response.result >= v2026_1_0_0;
				isUsingInteractiveMusicHierarchy = response.result < v2025_1_0_0;

				// Manually trigger listeners on version sensitive properties.
				applicationState.sendPropertyChangeMessage(IDs::importDestination);
			};

			waapiClient.getVersionAsync(onGetVersionAsync);
		}
	}
} // namespace AK::WwiseTransfer
