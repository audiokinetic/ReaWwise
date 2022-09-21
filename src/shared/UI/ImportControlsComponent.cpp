#include "ImportControlsComponent.h"

#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"

#include <set>

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
		, originalsFolder(applicationState, IDs::originalsFolder, nullptr)
		, languageSubfolder(applicationState, IDs::languageSubfolder, nullptr)
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
		waqlEnabled.referTo(featureSupport, IDs::waqlEnabled, nullptr);

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

	namespace
	{
		bool RenderDirContentModified(const std::set<juce::File>& directorySet, const juce::Time& lastWriteTime)
		{
			for(const auto& directory : directorySet)
			{
				for(const auto& file : directory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false))
				{
					if(file.getLastModificationTime() > lastWriteTime)
					{
						return true;
					}
				}
			}
			return false;
		}
	} // namespace

	void ImportControlsComponent::onImportButtonClick()
	{
		using namespace ImportControlsComponentConstants;

		// Disable the import button while rendering
		importButton.setEnabled(false);

		const auto hierarchyMappingPath = ImportHelper::hierarchyMappingToPath(ImportHelper::valueTreeToHierarchyMappingNodeList(applicationState.getChildWithName(IDs::hierarchyMapping)));
		const Import::Options opts(importDestination, originalsSubFolder, hierarchyMappingPath);

		const auto previewItems = dawContext.getItemsForPreview(opts);
		std::set<juce::File> directorySet;
		for(const auto item : previewItems)
		{
			directorySet.insert(juce::File(item.audioFilePath).getParentDirectory());
		}
		auto lastModificationTime = juce::Time::getCurrentTime();

		juce::Logger::writeToLog("Sending render request to DAW");
		dawContext.renderItems();

		if(!RenderDirContentModified(directorySet, lastModificationTime))
		{
			const juce::String message("One or more files failed to render.");
			juce::Logger::writeToLog(message);
			juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Import Aborted", message);
			importButton.setEnabled(true);
			return;
		}

		const auto importItems = dawContext.getItemsForImport(opts);

		bool showRenameWarning = false;
		bool showRenderFailed = false;
		if(importItems.size() > 0)
		{
			for(const auto& importItem : importItems)
			{
				if(importItem.renderFilePath.isEmpty())
				{
					showRenderFailed = true;
					break;
				}

				if(importItem.audioFilePath != importItem.renderFilePath)
				{
					showRenameWarning = true;
					break;
				}
			}
		}
		else
		{
			juce::Logger::writeToLog("No items to import.");
			importButton.setEnabled(true);
			return;
		}

		if(showRenderFailed)
		{
			const juce::String message("One or more files failed to render.");
			juce::Logger::writeToLog(message);
			juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Import Aborted", message);
			importButton.setEnabled(true);
			return;
		}

		const auto hierarchyMappingNodeList = ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping);
		const Import::Task::Options importTaskOptions{
			importItems,
			containerNameExistsOption,
			applyTemplateOption,
			importDestination,
			hierarchyMappingNodeList,
			originalsFolder,
			languageSubfolder,
			selectObjectsOnImportCommand,
			applyTemplateFeatureEnabled,
			undoGroupFeatureEnabled,
			waqlEnabled};

		auto onImportComplete = [this, importTaskOptions = importTaskOptions](const Import::Summary& importSummary)
		{
			showImportSummary(importSummary, importTaskOptions);
			importButton.setEnabled(true);
		};

		importTask.reset(new ImportTask(waapiClient, importTaskOptions, onImportComplete));

		if(!showRenameWarning || !applicationProperties.getShowSilentIncrementWarning())
		{
			juce::Logger::writeToLog("Importing files...");
			importTask->launchThread();
			return;
		}

		const juce::String message("Several file names where silently incremented to avoid overwriting during the render process.");
		juce::Logger::writeToLog(message);

		auto onDialogBtnClicked = [this, importItems = importItems](int result)
		{
			applicationProperties.setShowSilentIncrementWarning(!showSilentIncrementWarningToggle.getToggleState());

			if(result == MessageBoxOption::Continue)
			{
				juce::Logger::writeToLog("Importing files...");
				importTask->launchThread();
				return;
			}

			juce::Logger::writeToLog("Import aborted.");
			importTask.reset();
			importButton.setEnabled(true);
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

	void ImportControlsComponent::showImportSummary(const Import::Summary& summary, const Import::Task::Options& importTaskOptions)
	{
		juce::String message;

		message << summary.getNumObjectsCreated() << " object(s) created.";
		message << juce::NewLine() << summary.getNumObjectTemplatesApplied() << " object template(s) applied.";
		message << juce::NewLine() << importTaskOptions.importItems.size() << " audio files(s) imported.";

		if(summary.errorMessage.isNotEmpty())
			message << juce::NewLine() << summary.errorMessage;

		auto messageBoxOptions = juce::MessageBoxOptions()
		                             .withTitle("Import Summary")
		                             .withMessage(message)
		                             .withButton("View Details")
		                             .withButton("Close");

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

		importSummaryFile.appendText("Objects created: " + juce::String(summary.getNumObjectsCreated()) + "\n");
		importSummaryFile.appendText("Object Templates Applied: " + juce::String(summary.getNumObjectTemplatesApplied()) + "\n");
		importSummaryFile.appendText("Audio Files Imported: " + juce::String(importTaskOptions.importItems.size()) + "\n\n");

		importSummaryFile.appendText("<table><tr><th>Object Path</th><th>Type</th><th>Object Status</th><th>Originals Wav</th><th>Wav Status</th><th>Property Template Applied</th></tr>");

		for(const auto& [objectPath, object] : summary.objects)
		{
			importSummaryFile.appendText("<tr><td>" + objectPath + "</td><td>" + WwiseHelper::objectTypeToReadableString(object.type) + "</td>" +
										 "<td>" + ImportHelper::objectStatusToReadableString(object.objectStatus) + "</td><td>" + object.originalWavFilePath + "</td>" +
										 "<td>" + ImportHelper::wavStatusToReadableString(object.wavStatus) + "</td><td>" + object.propertyTemplatePath + "</td></tr>");
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
