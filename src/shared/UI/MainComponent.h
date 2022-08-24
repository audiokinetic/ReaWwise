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

	private:
		juce::ValueTree applicationState;

		ApplicationState::Validator validator;
		ApplicationProperties applicationProperties;
		WaapiClient waapiClient;
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
