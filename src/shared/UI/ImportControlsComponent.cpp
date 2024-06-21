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

#include "ImportControlsComponent.h"

#include "Helpers/FileHelper.h"
#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"
#include "Theme/CustomLookAndFeel.h"

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

		constexpr int errorMessageWidth = 260;
		constexpr int errorMessageHeight = 200;
		constexpr int errorMessageMarginLeft = 75;
	}; // namespace ImportControlsComponentConstants

	ImportControlsComponent::ImportControlsComponent(juce::ValueTree appState,
		WaapiClient& waapiClient,
		DawContext& dawContext,
		ApplicationProperties& applicationProperties,
		const juce::String& applicationName)
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
		, transferInProgress(applicationState, IDs::transferInProgress, nullptr)
		, hierarchyMapping(applicationState.getChildWithName(IDs::hierarchyMapping))
		, previewItems(applicationState.getChildWithName(IDs::previewItems))
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
			transferToWwise();
		};

		addAndMakeVisible(importButton);

		refreshComponent();

		applicationState.addListener(this);

		showSilentIncrementWarningToggle.setButtonText("Don't show message again");
		showSilentIncrementWarningToggle.setSize(showSilentIncrementWarningToggleWidth, showSilentIncrementWarningToggleHeight);

		errorMessageContainer.setMultiLine(true);
		errorMessageContainer.setReadOnly(true);
		errorMessageContainer.setSize(errorMessageWidth, errorMessageHeight);
	}

	ImportControlsComponent::~ImportControlsComponent()
	{
		applicationState.removeListener(this);
	}

	void ImportControlsComponent::resized()
	{
		importButton.setBounds(getLocalBounds());
	}

	void ImportControlsComponent::transferToWwise()
	{
		using namespace ImportControlsComponentConstants;

		if(transferInProgress.get())
			return;

		transferInProgress = true;

		const auto hierarchyMappingPath =
			ImportHelper::hierarchyMappingToPath(ImportHelper::valueTreeToHierarchyMappingNodeList(applicationState.getChildWithName(IDs::hierarchyMapping)));
		const Import::Options opts(importDestination, originalsSubFolder, hierarchyMappingPath);

		const auto previewItems = dawContext.getItemsForPreview(opts);

		// Confirm that files where rendered
		std::set<juce::File> directorySet;
		for(const auto item : previewItems)
		{
			directorySet.insert(juce::File(item.audioFilePath).getParentDirectory());
		}

		auto lastModificationTime = juce::Time::getCurrentTime();

		juce::Logger::writeToLog("Sending render request to DAW");

		dawContext.renderItems();

		if(FileHelper::countModifiedFilesInDirectoriesSince(directorySet, lastModificationTime) != previewItems.size())
		{
			onRenderFailedDetected();
			return;
		}

		auto importItems = dawContext.getItemsForImport(opts);

		bool showIncompletePathWarning = false;
		bool showRenameWarning = false;
		bool isCrossMachineTransferEnabled = applicationProperties.getIsCrossMachineTransferEnabled();

		if(importItems.size() > 0)
		{
			for(auto& importItem : importItems)
			{
				if(importItem.renderFilePath.isEmpty())
				{
					onRenderFailedDetected();
					return;
				}

				if(juce::File(importItem.audioFilePath) != juce::File(importItem.renderFilePath))
					showRenameWarning = true;

				if(!WwiseHelper::isPathComplete(importItem.path))
					showIncompletePathWarning = true;

				using namespace juce;
				const File rendPath(importItem.renderFilePath);
				importItem.renderFileName = rendPath.getFileName();

				if(isCrossMachineTransferEnabled)
				{
					MemoryBlock mb;
					std::unique_ptr<FileInputStream> inputStream = rendPath.createInputStream();
					inputStream->readIntoMemoryBlock(mb);
					importItem.renderFileWavBase64 = Base64::toBase64(mb.getData(), mb.getSize());
					// add base64 padding
					importItem.renderFileWavBase64 += String(std::string(importItem.renderFileWavBase64.length() % 4, '='));
				}
			}
		}
		else
		{
			juce::Logger::writeToLog("No items to import...");
			transferInProgress = false;
			return;
		}

		if(showRenameWarning && applicationProperties.getShowSilentIncrementWarning())
			onFileRenamedDetected(showIncompletePathWarning, importItems);
		else if(showIncompletePathWarning)
			onPathIncompleteDetected(importItems);
		else
			onImport(importItems);
	}

	void ImportControlsComponent::showImportSummaryModal(const Import::Summary& summary, const Import::Task::Options& importTaskOptions)
	{
		auto hasErrors = !summary.errors.empty();

		juce::String title(!hasErrors ? "Wwise Import Successful" : "Wwise Imported with Errors");

		juce::String message;
		message << summary.getNumObjectsCreated() << " object(s) created.";
		message << juce::NewLine() << summary.getNumObjectTemplatesApplied() << " object template(s) applied.";
		message << juce::NewLine() << summary.getNumAudiofilesTransfered() << " audio files(s) imported.";

		auto messageBoxOptions = juce::MessageBoxOptions().withTitle(title).withMessage(message).withButton("View Details").withButton("Close");

		auto onDialogBtnClicked = [this, summary = summary, importTaskOptions = importTaskOptions](int result)
		{
			if(result == MessageBoxOption::Continue)
			{
				auto importSummaryFile = createImportSummaryFile(summary, importTaskOptions);
				importSummaryFile.launchInDefaultBrowser();
			}
		};

		juce::AlertWindow::showAsync(messageBoxOptions, onDialogBtnClicked);

		auto modalManager = juce::ModalComponentManager::getInstance();
		auto alertWindow = dynamic_cast<juce::AlertWindow*>(modalManager->getModalComponent(0));

		if(hasErrors)
		{
			juce::String errorMessage;

			for(const auto& error : summary.errors)
			{
				errorMessage << "Error: `" + error.uri + "` for procedure `" + error.procedureUri + "`" << juce::NewLine() << juce::NewLine();
				errorMessage << "Message: " + error.message << juce::NewLine() << juce::NewLine();
			}

			alertWindow->addCustomComponent(&errorMessageContainer);
			auto currentBounds = errorMessageContainer.getBounds();
			errorMessageContainer.setBounds(currentBounds.withX(ImportControlsComponentConstants::errorMessageMarginLeft));
			errorMessageContainer.setText(errorMessage);
			errorMessageContainer.setColour(juce::TextEditor::backgroundColourId, findColour(juce::AlertWindow::backgroundColourId));
			errorMessageContainer.setFont(CustomLookAndFeelConstants::smallFontSize);
		}
		else
		{
			for(int i = 0; i < alertWindow->getNumChildComponents(); ++i)
			{
				auto* component = alertWindow->getChildComponent(i);
				if(component->getName() == "Error Details")
					component->setEnabled(false);
			}
		}
	}

	juce::URL ImportControlsComponent::createImportSummaryFile(const Import::Summary& summary, const Import::Task::Options& importTaskOptions)
	{
		auto currentTime = juce::Time::getCurrentTime();
		auto hasErrors = !summary.errors.empty();

		auto importSummaryFile = juce::File::getSpecialLocation(juce::File::tempDirectory)
		                             .getChildFile(applicationName + "_WwiseImportSummary_" + currentTime.formatted("%Y-%m-%d_%H-%M-%S"))
		                             .withFileExtension(".html");

		importSummaryFile.appendText(ImportHelper::createImportSummary(applicationName, currentTime, summary, importTaskOptions));

		return juce::URL(importSummaryFile.getFullPathName());
	}

	void ImportControlsComponent::refreshComponent()
	{
		auto importButtonEnabled = !transferInProgress.get() && originalsSubfolderValid.get() && importDestinationValid.get() && projectPath.get().isNotEmpty() &&
		                           previewItems.getNumChildren() > 0;

		auto hieararchyMappingNodes = ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping);

		auto hierarchyMappingValid = true;
		for(const auto& hierarchyMappingNode : hieararchyMappingNodes)
		{
			hierarchyMappingValid &= hierarchyMappingNode.typeValid && hierarchyMappingNode.nameValid &&
			                         (!hierarchyMappingNode.propertyTemplatePathEnabled || hierarchyMappingNode.propertyTemplatePathValid);
		}

		importButtonEnabled &= hierarchyMappingValid;

		importButton.setEnabled(importButtonEnabled);

		juce::String tooltip;

		if(projectPath.get().isEmpty())
			tooltip = "Connect to Wwise to continue";
		else if(previewItems.getNumChildren() == 0)
			tooltip = "Nothing to transfer";
		else if(!importButtonEnabled)
			tooltip = "Fix pending errors to continue";

		importButton.setTooltip(tooltip);
	}

	void ImportControlsComponent::valueTreePropertyChanged(juce::ValueTree& treeWhosePropertyHasChanged, const juce::Identifier& property)
	{
		if(treeWhosePropertyHasChanged == applicationState && (property == IDs::originalsSubfolderValid || property == IDs::importDestinationValid ||
																  property == IDs::projectPath || property == IDs::transferInProgress) ||
			treeWhosePropertyHasChanged.getType() == IDs::hierarchyMappingNode)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportControlsComponent::valueTreeChildAdded(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenAdded)
	{
		if(parentTree.getType() == IDs::hierarchyMapping || parentTree.getType() == IDs::previewItems)
		{
			triggerAsyncUpdate();
		}
	}

	void ImportControlsComponent::valueTreeChildRemoved(juce::ValueTree& parentTree, juce::ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
	{
		if(parentTree.getType() == IDs::hierarchyMapping || parentTree.getType() == IDs::previewItems)
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

	void ImportControlsComponent::onRenderFailedDetected()
	{
		const juce::String message("One or more files failed to render.");
		juce::Logger::writeToLog(message);

		juce::AlertWindow::showMessageBoxAsync(juce::MessageBoxIconType::InfoIcon, "Transfer to Wwise Aborted", message);

		transferInProgress = false;
		importTask.reset();
	}

	void ImportControlsComponent::onImportCancelled()
	{
		juce::Logger::writeToLog("Import was cancelled by user...");

		transferInProgress = false;
		importTask.reset();
	}

	void ImportControlsComponent::onFileRenamedDetected(bool isPathIncomplete, const std::vector<Import::Item>& importItems)
	{
		auto onDialogBtnClicked = [this, isPathIncomplete, importItems](int result)
		{
			applicationProperties.setShowSilentIncrementWarning(!showSilentIncrementWarningToggle.getToggleState());

			if(result == MessageBoxOption::Continue)
			{
				if(isPathIncomplete)
					onPathIncompleteDetected(importItems);
				else
					onImport(importItems);
			}
			else
				onImportCancelled();
		};

		const juce::String message("One or more file names where silently incremented to avoid overwriting during the render process.");
		juce::Logger::writeToLog(message);

		auto messageBoxOptions = juce::MessageBoxOptions().withTitle("Action Required").withMessage(message).withButton("Continue").withButton("Cancel");

		juce::AlertWindow::showAsync(messageBoxOptions, onDialogBtnClicked);

		auto modalManager = juce::ModalComponentManager::getInstance();
		auto alertWindow = dynamic_cast<juce::AlertWindow*>(modalManager->getModalComponent(0));
		alertWindow->addCustomComponent(&showSilentIncrementWarningToggle);

		// Reset and reposition the toggle button
		showSilentIncrementWarningToggle.setToggleState(false, true);
		auto bounds = showSilentIncrementWarningToggle.getBounds();
		bounds.setX(ImportControlsComponentConstants::showSilentIncrementWarningToggleMarginLeft);
		showSilentIncrementWarningToggle.setBounds(bounds);
	}

	void ImportControlsComponent::onPathIncompleteDetected(const std::vector<Import::Item>& importItems)
	{
		auto onDialogBtnClicked = [this, importItems](int result)
		{
			if(result == MessageBoxOption::Continue)
				onImport(importItems);
			else
				onImportCancelled();
		};

		const juce::String message("One or more object paths are incomplete and will not be transfered.");
		juce::Logger::writeToLog(message);

		auto messageBoxOptions = juce::MessageBoxOptions().withTitle("Action Required").withMessage(message).withButton("Continue").withButton("Cancel");

		juce::AlertWindow::showAsync(messageBoxOptions, onDialogBtnClicked);
	}

	void ImportControlsComponent::onImport(const std::vector<Import::Item>& importItems)
	{
		const auto hierarchyMappingNodeList = ImportHelper::valueTreeToHierarchyMappingNodeList(hierarchyMapping);

		const Import::Task::Options importTaskOptions{
			importItems, containerNameExistsOption, applyTemplateOption, importDestination, hierarchyMappingNodeList,
			originalsFolder, languageSubfolder, selectObjectsOnImportCommand, applyTemplateFeatureEnabled, undoGroupFeatureEnabled,
			waqlEnabled};

		auto onImportComplete = [this, importTaskOptions = importTaskOptions](const Import::Summary& importSummary)
		{
			showImportSummaryModal(importSummary, importTaskOptions);

			transferInProgress = false;
			importTask.reset(); // Needs to be called last since this lambda gets called inside of importTask
		};

		juce::Logger::writeToLog("Importing files...");

		importTask.reset(new ImportTask(waapiClient, importTaskOptions, onImportComplete));
		importTask->launchThread();
	}

} // namespace AK::WwiseTransfer
