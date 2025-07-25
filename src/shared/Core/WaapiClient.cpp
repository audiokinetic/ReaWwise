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

#include "WaapiClient.h"

#include "Helpers/ImportHelper.h"
#include "Model/IDs.h"

#include <JSONHelpers.h>
#include <juce_events/juce_events.h>
#include <set>

namespace
{
	juce::String normalizePath(const juce::String& path)
	{
		juce::String normalizedPath(path);
#ifndef WIN32
		// Waapi returns the path as a windows path
		normalizedPath.replace("\\", juce::File::getSeparatorString()).replace("Y:", "~").replace("Z:", "/");
#endif

		return normalizedPath;
	}
}

namespace AK::WwiseTransfer
{
	namespace WaapiCommands
	{
		static constexpr const char* const projectLoaded = "ak.wwise.core.project.loaded";
		static constexpr const char* const projectPostClosed = "ak.wwise.core.project.postClosed";
		static constexpr const char* const objectPostDeleted = "ak.wwise.core.object.postDeleted";
		static constexpr const char* const objectNameChanged = "ak.wwise.core.object.nameChanged";
		static constexpr const char* const objectCreated = "ak.wwise.core.object.created";
		static constexpr const char* const objectGet = "ak.wwise.core.object.get";
		static constexpr const char* const audioImport = "ak.wwise.core.audio.import";
		static constexpr const char* const getProjectInfo = "ak.wwise.core.getProjectInfo";
		static constexpr const char* const undoBeginGroup = "ak.wwise.core.undo.beginGroup";
		static constexpr const char* const undoCancelGroup = "ak.wwise.core.undo.cancelGroup";
		static constexpr const char* const undoEndGroup = "ak.wwise.core.undo.endGroup";
		static constexpr const char* const objectPasteProperties = "ak.wwise.core.object.pasteProperties";
		static constexpr const char* const getInfo = "ak.wwise.core.getInfo";
		static constexpr const char* const getSelectedObjects = "ak.wwise.ui.getSelectedObjects";
		static constexpr const char* const commandsExecute = "ak.wwise.ui.commands.execute";
	} // namespace WaapiCommands

	namespace WaapiURIs
	{
		static constexpr const char* const unknownObject = "ak.wwise.query.unknown_object";
	}

	namespace WaapiMessages
	{
		static constexpr const char* const objectNotFound = "Object not found";
	}

	WaapiClientWatcher::WaapiClientWatcher(juce::ValueTree appState, WaapiClient& waapiClient, WaapiClientWatcherConfig&& waapiClientWatcherConfig)
		: juce::Thread("WaapiService")
		, applicationState(appState)
		, projectId(applicationState, IDs::projectId, nullptr)
		, waapiConnected(applicationState, IDs::waapiConnected, nullptr)
		, wwiseObjectsChanged(applicationState, IDs::wwiseObjectsChanged, nullptr)
		, waapiClient(waapiClient)
		, waapiClientWatcherConfig(waapiClientWatcherConfig)
		, connectionRetryDelay(waapiClientWatcherConfig.MinConnectionRetryDelay)
		, languages(applicationState.getChildWithName(IDs::languages))
		, ip(waapiClientWatcherConfig.Ip)
		, port(waapiClientWatcherConfig.Port)
	{
	}

	void WaapiClientWatcher::start()
	{
		startThread();
	}

	void WaapiClientWatcher::stop()
	{
		stopThread(-1);
	}

	void WaapiClientWatcher::changeParameters(const juce::String& ip, int port)
	{
		{
			std::lock_guard lock(guiMutex);
			this->ip = ip;
			this->port = port;
			shouldReconnect = true;
		}
		notify();
	}

	void WaapiClientWatcher::run()
	{
		using namespace WwiseAuthoringAPI;
		auto onDisconnect = [this]
		{
			juce::Logger::writeToLog("Disconnected from waapi");

			setWaapiConnected(false);
		};

		auto onProjectLoaded = [this](auto, auto)
		{
			juce::Logger::writeToLog("Received project loaded event");

			setProjectId("");
		};

		auto onProjectPostClosed = [this](auto, auto)
		{
			juce::Logger::writeToLog("Received project post close event");

			applicationState.setProperty(IDs::projectPath, "", nullptr);
		};

		auto onObjectEvent = [this](auto, auto)
		{
			juce::Logger::writeToLog("Received object related event");

			setWwiseObjectsChanged(true);
		};

		while(!threadShouldExit())
		{
			{
				std::unique_lock lock(guiMutex);
				if(shouldReconnect)
				{
					waapiClientWatcherConfig.Ip = ip;
					waapiClientWatcherConfig.Port = port;
					shouldReconnect = false;
					lock.unlock();
					disconnectFromWaapi();
				}
			}

			int waitTime = waapiClientWatcherConfig.ConnectionMonitorDelay;

			if(!waapiClient.isConnected())
			{
				if(waapiClient.connect(static_cast<const char*>(waapiClientWatcherConfig.Ip.toUTF8()), waapiClientWatcherConfig.Port, onDisconnect))
				{
					juce::Logger::writeToLog("Connected to waapi");

					connectionRetryDelay = waapiClientWatcherConfig.MinConnectionRetryDelay;

					AkJson subscribeResult;
					if(waapiClient.subscribe(WaapiCommands::projectLoaded, AkJson::Map{}, onProjectLoaded, projectLoadedSubscriptionId, subscribeResult))
					{
						juce::Logger::writeToLog("Subscribed to project loaded");
					}
					else
					{
						juce::Logger::writeToLog("Failed to subscribed to project loaded");
					}

					if(waapiClient.subscribe(WaapiCommands::projectPostClosed, AkJson::Map{}, onProjectPostClosed, projectPostClosedSubscriptionId, subscribeResult))
					{
						juce::Logger::writeToLog("Subscribed to project unloaded");
					}
					else
					{
						juce::Logger::writeToLog("Failed to subscribed to project post closed");
					}

					if(waapiClient.subscribe(WaapiCommands::objectCreated, AkJson::Map{}, onObjectEvent, objectCreatedEventSubscriptionId, subscribeResult))
					{
						juce::Logger::writeToLog("Subscribed to object created");
					}
					else
					{
						juce::Logger::writeToLog("Failed to subscribed to object created");
					}

					if(waapiClient.subscribe(WaapiCommands::objectPostDeleted, AkJson::Map{}, onObjectEvent, objectPostDeletedEventSubscriptionId, subscribeResult))
					{
						juce::Logger::writeToLog("Subscribed to object postDeleted");
					}
					else
					{
						juce::Logger::writeToLog("Failed to subscribed to object postDeleted");
					}

					if(waapiClient.subscribe(WaapiCommands::objectNameChanged, AkJson::Map{}, onObjectEvent, objectNameChangedEventSubscriptionId, subscribeResult))
					{
						juce::Logger::writeToLog("Subscribed to object nameChanged");
					}
					else
					{
						juce::Logger::writeToLog("Failed to subscribed to object nameChanged");
					}

					setWaapiConnected(true);
				}
				else
				{
					waitTime = connectionRetryDelay;
					connectionRetryDelay = juce::jmin(connectionRetryDelay * 2, waapiClientWatcherConfig.MaxConnectionRetryDelay);

					juce::String errorMessage("Error trying to connect to waapi ... Retrying in ");

					errorMessage << connectionRetryDelay << " ms";

					juce::Logger::writeToLog(errorMessage);
				}
			}

			wait(waitTime);
		}
		if(waapiClient.isConnected())
		{
			disconnectFromWaapi();
		}
	}
	void WaapiClientWatcher::setProjectId(const juce::String& id)
	{
		juce::Logger::writeToLog("Setting projectId");

		auto onCallAsync = [this, id = id]
		{
			projectId = id;
			// On the first run of the extension, the project ID will be empty. On project load we
			// also explicetly set the id to empty so that Project Support knows it has to refresh the project data.
			// The juce value tree will not send updates if the property doesnt change. Lets force it to send an update.
			applicationState.sendPropertyChangeMessage(IDs::projectId);
		};

		juce::MessageManager::callAsync(onCallAsync);
	}

	void WaapiClientWatcher::setWwiseObjectsChanged(bool changed)
	{
		juce::Logger::writeToLog("Setting wwise objects changed");

		auto onCallAsync = [this, changed = changed]
		{
			wwiseObjectsChanged = changed;
		};

		juce::MessageManager::callAsync(onCallAsync);
	}

	void WaapiClientWatcher::disconnectFromWaapi()
	{
		using namespace WwiseAuthoringAPI;
		AkJson result;
		if(waapiClient.unsubscribe(projectLoadedSubscriptionId, result))
		{
			juce::Logger::writeToLog("Unsubscribed from project loaded");
		}
		else
		{
			juce::Logger::writeToLog("Failed to unsubscribed from project unloaded");
		}

		if(waapiClient.unsubscribe(projectPostClosedSubscriptionId, result))
		{
			juce::Logger::writeToLog("Unsubscribed from project post closed");
		}
		else
		{
			juce::Logger::writeToLog("Failed to unsubscribed from project post closed");
		}

		if(waapiClient.unsubscribe(objectCreatedEventSubscriptionId, result))
		{
			juce::Logger::writeToLog("Unsubscribed from object created");
		}
		else
		{
			juce::Logger::writeToLog("Failed to unsubscribed from object created");
		}

		if(waapiClient.unsubscribe(objectPostDeletedEventSubscriptionId, result))
		{
			juce::Logger::writeToLog("Unsubscribed from object postDeleted");
		}
		else
		{
			juce::Logger::writeToLog("Failed to unsubscribed from object postDeleted");
		}

		if(waapiClient.unsubscribe(objectNameChangedEventSubscriptionId, result))
		{
			juce::Logger::writeToLog("Unsubscribed from object postDeleted");
		}
		else
		{
			juce::Logger::writeToLog("Failed to unsubscribed from object postDeleted");
		}

		waapiClient.disconnect();

		setWaapiConnected(false);
		setProjectId("");
	}

	void WaapiClientWatcher::setWaapiConnected(bool connected)
	{
		juce::Logger::writeToLog("Setting waapi connected");

		auto resetProjectInfo = [this, connected = connected]
		{
			waapiConnected = connected;
		};

		juce::MessageManager::callAsync(resetProjectInfo);
	}

	WaapiClient::WaapiClient()
	{
	}

	bool WaapiClient::connect(const char* in_uri, unsigned int in_port, WwiseAuthoringAPI::disconnectHandler_t disconnectHandler, int in_timeoutMs)
	{
		return Connect(in_uri, in_port, disconnectHandler, in_timeoutMs);
	}

	bool WaapiClient::subscribe(const char* in_uri, const WwiseAuthoringAPI::AkJson& in_options, WampEventCallback in_callback, uint64_t& out_subscriptionId, WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs)
	{
		return Subscribe(in_uri, in_options, in_callback, out_subscriptionId, out_result, in_timeoutMs);
	}

	bool WaapiClient::unsubscribe(const uint64_t& in_subscriptionId, WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs)
	{
		return Unsubscribe(in_subscriptionId, out_result, in_timeoutMs);
	}

	bool WaapiClient::isConnected() const
	{
		return IsConnected();
	}

	void WaapiClient::disconnect()
	{
		Disconnect();
	}

	bool WaapiClient::call(const char* in_uri, const WwiseAuthoringAPI::AkJson& in_args, const WwiseAuthoringAPI::AkJson& in_options, WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs)
	{
		using namespace WwiseAuthoringAPI;

		auto status = Call(in_uri, in_args, in_options, out_result, in_timeoutMs);

		juce::Logger::writeToLog(juce::String(in_uri) +
								 juce::NewLine() + juce::String("args: ") + JSONHelpers::GetAkJsonString(in_args).substr(0, 10'000) + // Cap the args strings logged to 10,000 characters to avoid crashing the JUCE logger.
								 juce::NewLine() + juce::String("options: ") + JSONHelpers::GetAkJsonString(in_options) +
								 juce::NewLine() + juce::String("result: ") + JSONHelpers::GetAkJsonString(out_result));

		return status;
	}

	bool WaapiClient::call(const char* in_uri, const char* in_args, const char* in_options, std::string& out_result, int in_timeoutMs)
	{
		auto status = Call(in_uri, in_args, in_options, out_result, in_timeoutMs);

		juce::Logger::writeToLog(juce::String(in_uri) +
								 juce::NewLine() + juce::String("args: ") + in_args +
								 juce::NewLine() + juce::String("options: ") + in_options +
								 juce::NewLine() + juce::String("result: ") + out_result);

		return status;
	}

	Waapi::Response<Waapi::ObjectResponseSet> WaapiClient::import(const std::vector<Waapi::ImportItemRequest>& importItemsRequest, Import::ContainerNameExistsOption containerNameExistsOption, const juce::String& objectLanguage)
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Waapi::ObjectResponseSet> response;

		AkJson::Array importItemsAsJson;
		for(const auto& importItemRequest : importItemsRequest)
		{
			std::string key;
			std::string value;
			if(importItemRequest.renderFileWavBase64.isEmpty())
			{
				key = "audioFile";
				value = importItemRequest.renderFilePath.toStdString();
			}
			else
			{
				key = "audioFileBase64";
				value = importItemRequest.renderFileName.toStdString() + "|" + importItemRequest.renderFileWavBase64.toStdString();
			}
			using namespace juce;
			auto importItemAsJson = AkJson(AkJson::Map{
				{key,
					AkVariant(value)},
				{
					"objectPath",
					AkVariant(importItemRequest.path.toStdString()),
				},
				{
					"originalsSubFolder",
					AkVariant(importItemRequest.originalsSubFolder.toStdString()),
				},
			});

			importItemsAsJson.push_back(importItemAsJson);
		};

		const auto args = AkJson::Map{
			{
				"importOperation",
				AkVariant(ImportHelper::containerNameExistsOptionToString(containerNameExistsOption).toStdString()),
			},
			{
				"default",
				AkJson::Map{{"importLanguage", AkVariant(objectLanguage.toStdString())}},
			},
			{
				"imports",
				importItemsAsJson,
			},
			{
				"autoAddToSourceControl",
				AkVariant(true),
			},
		};

		static const auto options = AkJson::Map{
			{
				"return",
				AkJson::Array{
					AkVariant{"id"},
					AkVariant{"name"},
					AkVariant{"type"},
					AkVariant{"path"},
					AkVariant{"sound:originalWavFilePath"},
				},
			},
		};

		AkJson result;
		response.status = call(WaapiCommands::audioImport, args, options, result);

		if(response.status)
		{
			if(result.HasKey("objects"))
			{
				auto objects = result["objects"].GetArray();

				for(auto& object : objects)
				{
					response.result.emplace(object);
				}
			}
		}
		else
		{
			response.error = WaapiHelper::parseError(WaapiCommands::audioImport, result);
		}

		return response;
	}

	bool WaapiClient::selectObjects(const juce::String& selectObjectsOnImportCommand, const std::vector<juce::String>& objectPaths)
	{
		using namespace WwiseAuthoringAPI;

		AkJson::Array objects;
		for(const auto& objectPath : objectPaths)
		{
			objects.emplace_back(AkVariant(objectPath.toStdString()));
		}

		const auto args = AkJson::Map{
			{
				"command",
				AkVariant(selectObjectsOnImportCommand.toStdString()),
			},
			{
				"objects",
				objects,
			},
		};

		AkJson result;
		return call(WaapiCommands::commandsExecute, args, AkJson::Map{}, result);
	}

	Waapi::Response<Waapi::ObjectResponseSet> WaapiClient::getObjectAncestorsAndDescendants(const juce::String& objectPath)
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Waapi::ObjectResponseSet> response;

		auto waql = juce::String("\"" + objectPath + "\" select this, ancestors, descendants");

		const auto args = AkJson::Map{
			{
				"waql",
				AkVariant{waql.toStdString()},
			},
		};

		static const auto options = AkJson::Map{
			{
				"return",
				AkJson::Array{
					AkVariant{"id"},
					AkVariant{"name"},
					AkVariant{"type"},
					AkVariant{"path"},
					AkVariant{"sound:originalWavFilePath"},
					AkVariant{"workunitType"},
				},
			},
		};

		AkJson result;
		response.status = call(WaapiCommands::objectGet, args, options, result);

		if(!response.status)
		{
			// The waql query above will fail if the object was not found.
			// We still want to know if ancestors exist.
			auto objectAncestors = WwiseHelper::pathToAncestorPaths(objectPath);

			for(int i = objectAncestors.size() - 1; i >= 0; --i)
			{
				auto waql = juce::String("\"" + objectAncestors[i] + "\" select this, ancestors");

				const auto args = AkJson::Map{
					{
						"waql",
						AkVariant{waql.toStdString()},
					},
				};

				response.status = call(WaapiCommands::objectGet, args, options, result);

				if(response.status)
					break;
			}
		}

		if(response.status)
		{
			if(result.HasKey("return"))
			{
				auto objects = result["return"].GetArray();

				for(auto& object : objects)
				{
					response.result.emplace(object);
				}
			}
		}
		else
		{
			if(result.HasKey("message"))
			{
				juce::String message(result["message"].GetVariant().GetString());
				if(message.contains(WaapiMessages::objectNotFound))
					response.status = true;
			}

			response.error = WaapiHelper::parseError(WaapiCommands::objectGet, result);
		}

		return response;
	}

	Waapi::Response<std::vector<juce::String>> WaapiClient::getProjectLanguages()
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<std::vector<juce::String>> response;

		static const auto args(AkJson::Map{
			{
				"from",
				AkJson::Map{
					{
						"ofType",
						AkJson::Array{
							{AkVariant("Language")},
						},
					},
				},
			},
		});

		static const auto options(AkJson::Map{
			{
				"return",
				AkJson::Array{
					{AkVariant("name")},
				},
			},
		});

		AkJson result;
		response.status = call(WaapiCommands::objectGet, args, options, result);

		static const std::set<juce::String> exceptions = {
			"Mixed",
			"SFX",
			"External",
			"SoundSeed Grain",
		};

		if(response.status)
		{
			if(result.HasKey("return"))
			{
				auto objects = result["return"].GetArray();

				for(auto& object : objects)
				{
					auto language = juce::String(object["name"].GetVariant().GetString());

					if(exceptions.find(language) == exceptions.end())
						response.result.emplace_back(language);
				}
			}
		}
		else
		{
			response.error = WaapiHelper::parseError(WaapiCommands::objectGet, result);
		}

		return response;
	}

	Waapi::Response<Waapi::ObjectResponseSet> WaapiClient::getObjectAncestorsAndDescendantsLegacy(const juce::String& objectPath)
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Waapi::ObjectResponseSet> response;

		static auto buildArgs = [](const juce::String& path)
		{
			auto from = AkJson::Map{
				{
					"from",
					AkJson::Map{
						{
							"path",
							AkJson::Array{
								AkVariant{path.toStdString()},
							},
						},
					},
				},
			};

			return from;
		};

		static auto addTransform = [](AkJson::Map& args, const juce::String& transform)
		{
			args["transform"] = AkJson::Array{
				AkJson::Map{
					{
						"select",
						AkJson::Array{
							AkVariant{transform.toStdString()},
						},
					},
				},
			};
		};

		static auto fillResponse = [](auto& response, const auto& result)
		{
			if(result.HasKey("return"))
			{
				auto objects = result["return"].GetArray();

				for(auto& object : objects)
				{
					response.result.emplace(object);
				}
			}
		};

		static const auto options = AkJson::Map{
			{
				"return",
				AkJson::Array{
					AkVariant{"id"},
					AkVariant{"name"},
					AkVariant{"type"},
					AkVariant{"path"},
					AkVariant{"sound:originalWavFilePath"},
					AkVariant{"workunitType"},
				},
			},
		};

		auto args = buildArgs(objectPath);

		AkJson result;
		response.status = call(WaapiCommands::objectGet, args, options, result);

		if(response.status)
			fillResponse(response, result);

		args = buildArgs(objectPath);
		addTransform(args, "descendants");

		response.status = call(WaapiCommands::objectGet, args, options, result);

		if(response.status)
			fillResponse(response, result);

		args = buildArgs(objectPath);
		addTransform(args, "ancestors");

		response.status = call(WaapiCommands::objectGet, args, options, result);

		if(response.status)
			fillResponse(response, result);
		else
		{
			auto objectAncestors = WwiseHelper::pathToAncestorPaths(objectPath);

			for(int i = objectAncestors.size() - 1; i >= 0; --i)
			{
				args = buildArgs(objectAncestors[i]);

				response.status = call(WaapiCommands::objectGet, args, options, result);

				if(response.status)
				{
					fillResponse(response, result);

					args = buildArgs(objectAncestors[i]);
					addTransform(args, "ancestors");

					response.status = call(WaapiCommands::objectGet, args, options, result);

					if(response.status)
					{
						fillResponse(response, result);
						break;
					}
				}
			}
		}

		if(!response.status)
			response.error = WaapiHelper::parseError(WaapiCommands::objectGet, result);

		return response;
	}

	Waapi::Response<Waapi::ObjectResponse> WaapiClient::getObject(const juce::String& objectPath)
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Waapi::ObjectResponse> response;

		const auto args = AkJson::Map{
			{
				"from",
				AkJson::Map{
					{
						"path",
						AkJson::Array{
							AkVariant{objectPath.toStdString()},
						},
					},
				},
			},
		};

		static const auto options = AkJson::Map{
			{
				"return",
				AkJson::Array{
					AkVariant{"id"},
					AkVariant{"name"},
					AkVariant{"type"},
					AkVariant{"path"},
					AkVariant{"sound:originalWavFilePath"},
					AkVariant{"workunitType"},
				},
			},
		};

		AkJson result;
		response.status = call(WaapiCommands::objectGet, args, options, result);

		if(response.status)
		{
			if(result.HasKey("return"))
			{
				auto objects = result["return"].GetArray();

				for(auto& object : objects)
				{
					response.result = object;
				}
			}
		}
		// Special Case: The call actually succeeds but does not find the object
		else if(result.HasKey("uri") && result["uri"].GetVariant().GetString() == WaapiURIs::unknownObject)
		{
			response.status = true;
		}
		else
		{
			response.error = WaapiHelper::parseError(WaapiCommands::objectGet, result);
		}

		return response;
	}

	void WaapiClient::beginUndoGroup()
	{
		using namespace WwiseAuthoringAPI;

		AkJson result;
		call(WaapiCommands::undoBeginGroup, AkJson::Map{}, AkJson::Map{}, result);
	}

	void WaapiClient::cancelUndoGroup()
	{
		using namespace WwiseAuthoringAPI;

		AkJson result;
		call(WaapiCommands::undoCancelGroup, AkJson::Map{}, AkJson::Map{}, result);
	}

	void WaapiClient::endUndoGroup(const juce::String& displayName)
	{
		using namespace WwiseAuthoringAPI;

		auto arguments = AkJson::Map{
			{
				"displayName",
				AkVariant{displayName.toUTF8()},
			},
		};

		AkJson result;
		call(WaapiCommands::undoEndGroup, arguments, AkJson::Map{}, result);
	}

	Waapi::Response<Waapi::ObjectResponseSet> WaapiClient::pasteProperties(const Waapi::PastePropertiesRequest& pastePropertiesRequest)
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Waapi::ObjectResponseSet> response;

		AkJson::Array targets;
		for(auto& target : pastePropertiesRequest.targets)
		{
			targets.push_back(AkVariant(target.toStdString()));
		}

		const auto args(AkJson::Map{
			{
				"source",
				AkVariant(pastePropertiesRequest.source.toStdString()),
			},
			{
				"targets",
				targets,
			},
		});

		AkJson result;
		response.status = call(WaapiCommands::objectPasteProperties, args, AkJson::Map{}, result);

		if(!response.status)
		{
			response.error = WaapiHelper::parseError(WaapiCommands::objectPasteProperties, result);
		}

		return response;
	}

	Waapi::Response<Wwise::Version> WaapiClient::getVersion()
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Wwise::Version> response;

		AkJson result;
		response.status = call(WaapiCommands::getInfo, AkJson::Map{}, AkJson::Map{}, result);

		if(response.status)
		{
			if(result.HasKey("version") && result["version"].HasKey("displayName"))
			{
				response.result = {
					result["version"]["year"].GetVariant().GetInt32(),
					result["version"]["major"].GetVariant().GetInt32(),
					result["version"]["minor"].GetVariant().GetInt32(),
					result["version"]["build"].GetVariant().GetInt32(),
				};
			}
		}
		else
		{
			response.error = WaapiHelper::parseError(WaapiCommands::getInfo, result);
		}

		return response;
	}

	Waapi::Response<Waapi::ProjectInfo> WaapiClient::getProjectInfo()
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Waapi::ProjectInfo> response;

		static const auto args(AkJson::Map{
			{
				"from",
				AkJson::Map{
					{
						"ofType",
						AkJson::Array{
							{AkVariant("Project")},
						},
					},
				},
			},
		});

		static const auto options(AkJson::Map{
			{
				"return",
				AkJson::Array{
					{AkVariant("filePath")},
					{AkVariant("id")},
				},
			},
		});

		AkJson result;
		response.status = call(WaapiCommands::objectGet, args, options, result);

		if(response.status)
		{
			if(result.HasKey("return"))
			{
				const auto& returnResult = result["return"];

				if(!returnResult.GetArray().empty())
				{
					auto projectPath = normalizePath(juce::String(returnResult[0]["filePath"].GetVariant().GetString()));
					auto projectId = juce::String(returnResult[0]["id"].GetVariant().GetString()).removeCharacters("{}");

					response.result.projectPath = projectPath;
					response.result.projectId = projectId;
				}
			}
		}
		else
		{
			response.error = WaapiHelper::parseError(WaapiCommands::objectGet, result);
		}

		return response;
	}

	Waapi::Response<Waapi::AdditionalProjectInfo> WaapiClient::getAdditionalProjectInfo()
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Waapi::AdditionalProjectInfo> response;

		AkJson result;
		response.status = call(WaapiCommands::getProjectInfo, AkJson::Map{}, AkJson::Map{}, result);

		if(result.HasKey("directories") && result["directories"].HasKey("originals"))
		{
			auto originalsFolder = normalizePath(juce::String(result["directories"]["originals"].GetVariant().GetString()));

			response.result.originalsFolder = originalsFolder;
		}

		if(result.HasKey("referenceLanguageId") && result.HasKey("languages"))
		{
			const auto& referenceLanguageId = result["referenceLanguageId"].GetVariant().GetString();
			const auto& languages = result["languages"].GetArray();

			for(const auto& languageAsAkJson : languages)
			{
				const auto& language = languageAsAkJson.GetMap();
				const auto& langIdIt = language.find("id");
				const auto& langNameIt = language.find("name");

				if(langIdIt != language.cend() && langNameIt != language.cend())
				{
					if(langIdIt->second.GetVariant().GetString() == referenceLanguageId)
					{
						response.result.referenceLanguage = langNameIt->second.GetVariant().GetString();
						break;
					}
				}
			}
		}

		if(result.HasKey("defaultImportWorkUnit") && result["defaultImportWorkUnit"].HasKey("path"))
		{
			auto defaultImportWorkUnitPath = juce::String(result["defaultImportWorkUnit"]["path"].GetVariant().GetString());
			response.result.defaultImportWorkUnitPath = defaultImportWorkUnitPath;
		}

		return response;
	}

	Waapi::Response<Waapi::ObjectResponse> WaapiClient::getSelectedObject()
	{
		using namespace WwiseAuthoringAPI;

		Waapi::Response<Waapi::ObjectResponse> response;

		static const auto options = AkJson::Map{
			{
				"return",
				AkJson::Array{
					AkVariant{"id"},
					AkVariant{"name"},
					AkVariant{"type"},
					AkVariant{"path"},
				},
			},
		};

		AkJson result;
		response.status = call(WaapiCommands::getSelectedObjects, AkJson::Map{}, options, result);

		if(response.status)
		{
			if(result.HasKey("objects"))
			{
				const auto& returnObjects = result["objects"];

				if(!returnObjects.GetArray().empty())
					response.result = Waapi::ObjectResponse(returnObjects.GetArray()[0]);
			}
		}
		else
		{
			response.error = WaapiHelper::parseError(WaapiCommands::getSelectedObjects, result);
		}

		return response;
	}
} // namespace AK::WwiseTransfer
