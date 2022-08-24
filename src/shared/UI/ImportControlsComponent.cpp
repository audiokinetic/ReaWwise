#include "ImportControlsComponent.h"

#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"

namespace AK::WwiseTransfer
{
	enum MessageBoxOption
	{
		Cancel = 0,
		Continue = 1
	};

	namespace ImportControlsComponentConstants
	{
		constexpr int showSilentIncrementWarningToggleWidth = 300;
		constexpr int showSilentIncrementWarningToggleHeight = 60;
		constexpr int showSilentIncrementWarningToggleMarginLeft = 78;
	}; // namespace ImportControlsComponentConstants

	ImportControlsComponent::ImportControlsComponent(juce::ValueTree appState, WaapiClient& waapiClient, DawContext& dawContext, ApplicationProperties& applicationProperties, const juce::String& applicationName)
		: applicationState(appState)
		, originalsSubfolderValid(applicationState, IDs::originalsSubfolderValid, nullptr)
		, importDestinationValid(applicationState, IDs::importDestinationValid, nullptr)
		, importDestination(applicationState, IDs::importDestination, nullptr)
		, originalsSubFolder(applicationState, IDs::originalsSubfolder, nullptr)
		, projectPath(applicationState, IDs::projectPath, nullptr)
		, containerNameExistsOption(applicationState, IDs::containerNameExists, nullptr)
		, applyTemplateOption(applicationState, IDs::applyTemplate, nullptr)
		, hierarchyMapping(applicationState.getChildWithName(IDs::hierarchyMapping))
		, waapiClient(waapiClient)
		, dawContext(dawContext)
		, applicationProperties(applicationProperties)
		, applicationName(applicationName)
	{
		using namespace ImportControlsComponentConstants;

		auto featureSupport = applicationState.getChildWithName(IDs::featureSupport);
		selectObjectsOnImportCommand.referTo(featureSupport, IDs::selectObjectsOnImportCommand, nullptr);
		applyTemplateFeatureEnabled.referTo(featureSupport, IDs::applyTemplateFeatureEnabled, nullptr);
		undoGroupFeatureEnabled.referTo(featureSupport, IDs::undoGroupFeatureEnabled, nullptr);

		importButton.setButtonText("Transfer to Wwise");

		importButton.onClick = [this]
		{
			onImportButtonClick();
		};

		addAndMakeVisible(importButton);

		refreshComponent();

		applicationState.addListener(this);

		showSilentIncrementWarningToggle.setButtonText("Don't show message again");
		showSilentIncrementWarningToggle.setSize(showSilentIncrementWarningToggleWidth, showSilentIncrementWarningToggleHeight);
	}

	ImportControlsComponent::~ImportControlsComponent()
	{
		applicationState.removeListener(this);
	}

	void ImportControlsComponent::resized()
	{
		importButton.setBounds(getLocalBounds());
	}

	void ImportControlsComponent::onImportButtonClick()
	{
		using namespace ImportControlsComponentConstants;

		juce::Logger::writeToLog("Sending render request to daw");

		dawContext.renderImportItems();

		auto hierarchyMappingPath = ImportHelper::hierarchyMappingToPath(ImportHelper::valueTreeToHierarchyMappingNodeList(applicationState.getChildWithName(IDs::hierarchyMapping)));

		Import::Options opts(importDestination, originalsSubFolder, hierarchyMappingPath);

		auto importItems = dawContext.getImportItems(opts);

		auto showRenameWarning = false, showRenderFailed = false;
		if(importItems.size())
		{
			for(auto& importItem : importItems)
			{
				if(importItem.renderFilePath.isEmpty())
				{
					showRenderFailed = true;
					break;
				}
				else if(importItem.audioFilePath != importItem.renderFilePath)
				{
					showRenameWarning = true;
					break;
				}
			}
		}

		auto hierarchyMappingNodeList = ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping);

		Import::Task::Options importTaskOptions{
			importItems,
			containerNameExistsOption,
			applyTemplateOption,
			importDestination,
			hierarchyMappingNodeList,
			selectObjectsOnImportCommand,
			applyTemplateFeatureEnabled,
			undoGroupFeatureEnabled};

		auto onImportComplete = [this, importTaskOptions = importTaskOptions](const Import::Summary& importSummary)
		{
			showImportSummary(importSummary, importTaskOptions);
		};

		importTask.reset(new ImportTask(waapiClient, importTaskOptions, onImportComplete));

		if(showRenderFailed)
		{
			juce::String message("One or more files failed to render.");

			juce::Logger::writeToLog(message);

			juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Import Aborted", message);
		}
		else if(showRenameWarning && applicationProperties.getShowSilentIncrementWarning())
		{
			juce::String message("Several file names where silently incremented to avoid overwriting during the render process.");

			juce::Logger::writeToLog(message);

			auto onDialogBtnClicked = [this, importItems = importItems](int result)
			{
				applicationProperties.setShowSilentIncrementWarning(!showSilentIncrementWarningToggle.getToggleState());

				if(result == MessageBoxOption::Continue)
				{
					importTask->launchThread();
				}
			};

			auto messageBoxOptions = juce::MessageBoxOptions()
			                             .withTitle("Action Required")
			                             .withMessage(message)
			                             .withButton("Continue")
			                             .withButton("Cancel");

			juce::AlertWindow::showAsync(messageBoxOptions, onDialogBtnClicked);

			auto modalManager = juce::ModalComponentManager::getInstance();
			auto alertWindow = dynamic_cast<juce::AlertWindow*>(modalManager->getModalComponent(0));
			alertWindow->addCustomComponent(&showSilentIncrementWarningToggle);

			// Reset and reposition the toggle button
			showSilentIncrementWarningToggle.setToggleState(false, true);
			auto bounds = showSilentIncrementWarningToggle.getBounds();
			bounds.setX(showSilentIncrementWarningToggleMarginLeft);
			showSilentIncrementWarningToggle.setBounds(bounds);
		}
		else
		{
			importTask->launchThread();
		}
	}

	void ImportControlsComponent::showImportSummary(const Import::Summary& summary, const Import::Task::Options& importTaskOptions)
	{
		juce::String message;

		message << summary.objectsCreated << " object(s) created.";
		message << juce::NewLine() << summary.objectTemplatesApplied << " object template(s) applied.";
		message << juce::NewLine() << summary.audioFilesImported << " audio files(s) imported.";

		if(summary.errorMessage.isNotEmpty())
			message << juce::NewLine() << summary.errorMessage;

		auto messageBoxOptions = juce::MessageBoxOptions()
		                             .withTitle("Import Summary")
		                             .withMessage(message)
		                             .withButton("View Details")
		                             .withButton("Dismiss");

		auto onDialogBtnClicked = [this, summary = summary, importTaskOptions = importTaskOptions](int result)
		{
			if(result == MessageBoxOption::Continue)
			{
				viewImportSummaryDetails(summary, importTaskOptions);
			}
		};

		juce::AlertWindow::showAsync(messageBoxOptions, onDialogBtnClicked);
	}

	void ImportControlsComponent::viewImportSummaryDetails(const Import::Summary& summary, const Import::Task::Options& importTaskOptions)
	{
		auto currentTime = juce::Time::getCurrentTime();

		auto importSummaryFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
		                             .getChildFile(applicationName + "_ImportSummary_" + currentTime.formatted("%Y-%m-%d_%H-%M-%S"))
		                             .withFileExtension(".html");

		importSummaryFile.create();

		importSummaryFile.appendText("<style>table td, th { border:1px solid black; padding:10px; }");
		importSummaryFile.appendText("table { border-collapse:collapse; }");
		importSummaryFile.appendText("table th { text-align: left; }</style>");
		importSummaryFile.appendText("<pre>" + applicationName + ": Import Summary " + currentTime.formatted("%Y-%m-%d %H:%M:%S") + "\n\n");
		importSummaryFile.appendText("Import Destination: " + importTaskOptions.importDestination + "\n");
		importSummaryFile.appendText("Container Name Exists: " + ImportHelper::containerNameExistsOptionToReadableString(importTaskOptions.containerNameExistsOption) + "\n");
		importSummaryFile.appendText("Apply Template: " + ImportHelper::applyTemplateOptionToReadableString(importTaskOptions.applyTemplateOption) + "\n\n");

		importSummaryFile.appendText("Objects created: " + juce::String(summary.objectsCreated) + "\n");
		importSummaryFile.appendText("Object Templates Applied: " + juce::String(summary.objectTemplatesApplied) + "\n");
		importSummaryFile.appendText("Audio Files Imported: " + juce::String(summary.audioFilesImported) + "\n\n");

		importSummaryFile.appendText("<table><tr><th>Object Path</th><th>Type</th><th>Created</th><th>Originals Wav</th><th>Property Template Applied</th></tr>");

		for(const auto& [objectPath, object] : summary.objects)
		{
			importSummaryFile.appendText("<tr><td>" + objectPath + "</td><td>" + WwiseHelper::objectTypeToReadableString(object.type) + "</td><td>" + (object.newlyCreated ? "X" : "") + "</td><td>" + object.originalWavFilePath + "</td><td>" + object.propertyTemplatePath + "</td></tr>");
		}

		importSummaryFile.appendText("</table></pre>");

		juce::URL importSummaryFileUrl(importSummaryFile.getFullPathName());
		importSummaryFileUrl.launchInDefaultBrowser();
	}

	void ImportControlsComponent::refreshComponent()
	{
		auto importButtonEnabled = originalsSubfolderValid.get() && importDestinationValid.get() && projectPath.get().isNotEmpty();

		auto hieararchyMappingNodes = ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping);

		auto hierarchyMappingValid = true;
		for(const auto& hierarchyMappingNode : hieararchyMappingNodes)
		{
			hierarchyMappingValid &= hierarchyMappingNode.typeValid &&
			                         hierarchyMappingNode.nameValid &&
			                         (!hierarchyMappingNode.propertyTemplatePathEnabled || hierarchyMappingNode.propertyTemplatePathValid);
		}

		importButtonEnabled &= hierarchyMappingValid;

		importButton.setEnabled(importButtonEnabled);

		juce::String tooltip = "";

		if(projectPath.get().isEmpty())
			tooltip = "Connect to Wwise to continue";
		else if(!importButtonEnabled)
			tooltip = "Fix pending errors to continue";

		importButton.setTooltip(tooltip);
	}

	void ImportControlsComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		if(treeWhosePropertyHasChanged == applicationState && (property == IDs::originalsSubfolderValid || property == IDs::importDestinationValid || property == IDs::projectPath) ||
			treeWhosePropertyHasChanged.getType() == IDs::hierarchyMappingNode)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportControlsComponent::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportControlsComponent::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		if(parentTree.getType() == IDs::hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportControlsComponent::valueTreeChildOrderChanged(juce::ValueTree& parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex)
	{
		if(parentTreeWhoseChildrenHaveMoved.getType() == IDs::hierarchyMapping)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportControlsComponent::handleAsyncUpdate()
	{
		refreshComponent();
	}
} // namespace AK::WwiseTransfer
