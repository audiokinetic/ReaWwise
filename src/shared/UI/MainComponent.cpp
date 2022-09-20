#include "MainComponent.h"

#include "Persistance/ApplicationState.h"

namespace AK::WwiseTransfer
{
	namespace MainComponentConstants
	{
		constexpr int margin = 10;
		constexpr int originalsSubfolderComponentHeight = 62;
		constexpr int importDestinationComponentHeight = 26;
		constexpr int importComponentHeight = 292;
		constexpr int importConflictsComponentHeight = 62;
		constexpr int importControlsComponentHeight = 24;
		constexpr int ouputLogComponentHeight = 50;
		constexpr int mainComponentWidth = 600;
		constexpr int mainComponentHeight = 600;
		constexpr int connectionMonitorDelayDefault = 200;
		constexpr int minConnectionRetryDelayDefault = 200;
		constexpr int maxConnectionRetryDelayDefault = 1000;
		constexpr int splitterHeight = 8;
	} // namespace MainComponentConstants

	MainComponent::MainComponent(DawContext& dawContext, const juce::String& applicationName)
		: applicationState(ApplicationState::create())
		, validator(applicationState, waapiClient)
		, applicationProperties(applicationName)
		, waapiClientWatcher(applicationState, waapiClient, WaapiClientWatcherConfig{applicationProperties.getWaapiIp(), applicationProperties.getWaapiPort(), MainComponentConstants::connectionMonitorDelayDefault, MainComponentConstants::minConnectionRetryDelayDefault, MainComponentConstants::maxConnectionRetryDelayDefault})
		, originalsSubfolderComponent(applicationState, applicationName)
		, importDestinationComponent(applicationState, waapiClient)
		, importComponent(applicationState, waapiClient, applicationProperties, applicationName)
		, importPreviewComponent(applicationState)
		, dawWatcher(applicationState, waapiClient, dawContext, applicationProperties.getPreviewRefreshInterval())
		, importConflictsComponent(applicationState)
		, importControlsComponent(applicationState, waapiClient, dawContext, applicationProperties, applicationName)
		, featureSupport(applicationState, waapiClient)
		, persistanceSupport(applicationState, dawContext)
		, wwiseProjectSupport(applicationState, waapiClient)
		, collapsedUI(applicationState, IDs::collapsedUI, nullptr)
		, logger(applicationName)
		, tooltipWindow(this)
	{
		using namespace MainComponentConstants;

		if(hasScaleFactorOverride())
			juce::Desktop::getInstance().setGlobalScaleFactor(applicationProperties.getScaleFactorOverride());

		setSize(mainComponentWidth, mainComponentHeight);

		juce::Logger::setCurrentLogger(&logger);

		splitter.getToggleStateValue().referTo(collapsedUI.getPropertyAsValue());

		waapiClientWatcher.start();
		dawWatcher.start();

		applicationState.addListener(this);

		addAndMakeVisible(originalsSubfolderComponent);

		addChildComponent(importDestinationComponent);
		addChildComponent(importComponent);
		addChildComponent(importConflictsComponent);

		addAndMakeVisible(importPreviewComponent);
		addAndMakeVisible(importControlsComponent);
		addAndMakeVisible(splitter);

		refreshComponent(false);
	}

	MainComponent::~MainComponent()
	{
		waapiClientWatcher.stop();
		dawWatcher.stop();

		applicationState.removeListener(this);

		juce::Logger::setCurrentLogger(nullptr);
	}

	void MainComponent::resized()
	{
		using namespace MainComponentConstants;

		auto area = getLocalBounds();
		area.reduce(margin, margin);

		originalsSubfolderComponent.setBounds(area.removeFromTop(originalsSubfolderComponentHeight));

		if(!collapsedUI)
		{
			area.removeFromTop(margin);

			importDestinationComponent.setBounds(area.removeFromTop(importDestinationComponentHeight));

			area.removeFromTop(margin);

			importComponent.setBounds(area.removeFromTop(importComponentHeight));

			area.removeFromTop(margin);

			importConflictsComponent.setBounds(area.removeFromTop(importConflictsComponentHeight));
		}

		area.removeFromTop(margin);

		splitter.setBounds(area.removeFromTop(splitterHeight));

		area.removeFromTop(margin);

		importControlsComponent.setBounds(area.removeFromBottom(importControlsComponentHeight));

		area.removeFromBottom(margin);

		importPreviewComponent.setBounds(area);
	}

	void MainComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		if(treeWhosePropertyHasChanged.getType() == IDs::application && property == IDs::collapsedUI)
		{
			triggerAsyncUpdate();
		}
	}

	void MainComponent::handleAsyncUpdate()
	{
		refreshComponent(true);
	}

	void MainComponent::refreshComponent(bool shouldResize)
	{
		importDestinationComponent.setVisible(!collapsedUI);
		importComponent.setVisible(!collapsedUI);
		importConflictsComponent.setVisible(!collapsedUI);

		if(shouldResize)
			resized();
	}

	bool MainComponent::hasScaleFactorOverride()
	{
		return applicationProperties.getScaleFactorOverride() > 0.0;
	}

	void MainComponent::transferToWwise()
	{
		importControlsComponent.transferToWwise();
	}
} // namespace AK::WwiseTransfer
