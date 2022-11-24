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

#include "Core/DawContext.h"
#include "Core/DawWatcher.h"
#include "Core/Logger.h"
#include "Core/WaapiClient.h"
#include "ImportComponent.h"
#include "ImportConflictsComponent.h"
#include "ImportControlsComponent.h"
#include "ImportDestinationComponent.h"
#include "ImportPreviewComponent.h"
#include "OriginalsSubfolderComponent.h"
#include "OutputLogComponent.h"
#include "Persistance/ApplicationProperties.h"
#include "Persistance/ApplicationStateValidator.h"
#include "Persistance/FeatureSupport.h"
#include "Persistance/PersistanceSupport.h"
#include "Persistance/WwiseProjectSupport.h"
#include "Splitter.h"
#include "Theme/CustomLookAndFeel.h"

#include <memory>

namespace AK::WwiseTransfer
{
	class MainComponent
		: public juce::Component
		, juce::ValueTree::Listener
		, juce::AsyncUpdater
	{
	public:
		MainComponent(DawContext& dawContext, const juce::String& applicationName);
		~MainComponent();

		void resized() override;
		bool hasScaleFactorOverride();

		void transferToWwise();

	private:
		juce::ValueTree applicationState;
		WaapiClient waapiClient;

		ApplicationState::Validator validator;
		ApplicationProperties applicationProperties;
		WaapiClientWatcher waapiClientWatcher;
		Logger logger;
		DawWatcher dawWatcher;

		FeatureSupport featureSupport;
		PersistanceSupport persistanceSupport;
		WwiseProjectSupport wwiseProjectSupport;

		OutputLogComponent outputLog;
		OriginalsSubfolderComponent originalsSubfolderComponent;
		ImportDestinationComponent importDestinationComponent;
		ImportComponent importComponent;
		ImportConflictsComponent importConflictsComponent;
		ImportPreviewComponent importPreviewComponent;
		ImportControlsComponent importControlsComponent;

		juce::TooltipWindow tooltipWindow;

		Splitter splitter;

		juce::CachedValue<bool> collapsedUI;

		void valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property) override;
		void handleAsyncUpdate() override;

		void refreshComponent(bool shouldResize);

		JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
	};
} // namespace AK::WwiseTransfer
