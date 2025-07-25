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

#pragma once

#include "AK/WwiseAuthoringAPI/AkAutobahn/AkJson.h"
#include "AK/WwiseAuthoringAPI/AkAutobahn/Client.h"
#include "Helpers/WaapiHelper.h"
#include "Model/Import.h"
#include "Model/Waapi.h"
#include "Model/Wwise.h"

#include <juce_core/juce_core.h>
#include <juce_events/juce_events.h>

namespace AK::WwiseTransfer
{
	struct WaapiClientWatcherConfig
	{
		juce::String Ip;
		int Port;
		const int ConnectionMonitorDelay;
		const int MinConnectionRetryDelay;
		const int MaxConnectionRetryDelay;
	};

	template <class Function, class Callback>
	class AsyncJob
		: public juce::ThreadPoolJob
	{
	public:
		AsyncJob(const Function& function, const Callback& callback)
			: function(function)
			, callback(callback)
			, juce::ThreadPoolJob("AsyncJob")
		{
		}

		juce::ThreadPoolJob::JobStatus runJob() override
		{
			decltype(function()) response;

			auto onExecute = [this, &response]()
			{
				response = function();

				return response.status || shouldExit();
			};

			WaapiHelper::executeWithRetry(onExecute);

			auto onJobComplete = [callback = callback, response = std::move(response)]
			{
				callback(response);
			};

			juce::MessageManager::callAsync(onJobComplete);

			return juce::ThreadPoolJob::JobStatus::jobHasFinished;
		}

	private:
		Callback callback;
		Function function;
	};

	class WaapiClient
		: private WwiseAuthoringAPI::Client
	{
	public:
		explicit WaapiClient();

		bool connect(const char* in_uri, unsigned int in_port, WwiseAuthoringAPI::disconnectHandler_t disconnectHandler = nullptr, int in_timeoutMs = -1);
		bool subscribe(const char* in_uri, const WwiseAuthoringAPI::AkJson& in_options, WampEventCallback in_callback, uint64_t& out_subscriptionId, WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs = -1);
		bool unsubscribe(const uint64_t& in_subscriptionId, WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs = -1);
		bool isConnected() const;
		void disconnect();
		bool call(const char* in_uri, const WwiseAuthoringAPI::AkJson& in_args, const WwiseAuthoringAPI::AkJson& in_options, WwiseAuthoringAPI::AkJson& out_result, int in_timeoutMs = -1);
		bool call(const char* in_uri, const char* in_args, const char* in_options, std::string& out_result, int in_timeoutMs = -1);

		Waapi::Response<Wwise::Version> getVersion();
		Waapi::Response<Waapi::ProjectInfo> getProjectInfo();
		Waapi::Response<Waapi::AdditionalProjectInfo> getAdditionalProjectInfo();
		Waapi::Response<Waapi::ObjectResponse> getSelectedObject();
		Waapi::Response<Waapi::ObjectResponseSet> pasteProperties(const Waapi::PastePropertiesRequest& pastePropertiesRequest);
		Waapi::Response<Waapi::ObjectResponseSet> import(const std::vector<Waapi::ImportItemRequest>& importItemsRequest, Import::ContainerNameExistsOption containerNameExistsOption, const juce::String& objectLanguage);
		Waapi::Response<Waapi::ObjectResponseSet> getObjectAncestorsAndDescendants(const juce::String& objectPath);
		Waapi::Response<Waapi::ObjectResponseSet> getObjectAncestorsAndDescendantsLegacy(const juce::String& objectPath);
		Waapi::Response<std::vector<juce::String>> getProjectLanguages();
		Waapi::Response<Waapi::ObjectResponse> getObject(const juce::String& objectPath);

		bool selectObjects(const juce::String& selectObjectsCommand, const std::vector<juce::String>& objectPaths);

		void beginUndoGroup();
		void cancelUndoGroup();
		void endUndoGroup(const juce::String& displayName);

		template <typename Callback>
		void getVersionAsync(const Callback& callback)
		{
			auto onJobExecute = [this]()
			{
				return getVersion();
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

		template <typename Callback>
		void getProjectInfoAsync(const Callback& callback)
		{
			auto onJobExecute = [this]()
			{
				return getProjectInfo();
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

		template <typename Callback>
		void getAdditionalProjectInfoAsync(const Callback& callback)
		{
			auto onJobExecute = [this]()
			{
				return getAdditionalProjectInfo();
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

		template <typename Callback>
		void getSelectedObjectAsync(const Callback& callback)
		{
			auto onJobExecute = [this]()
			{
				return getSelectedObject();
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

		template <typename Callback>
		void pastePropertiesAsync(const Waapi::PastePropertiesRequest& pastePropertiesRequest, const Callback& callback)
		{
			auto onJobExecute = [this, pastePropertiesRequest = pastePropertiesRequest]()
			{
				return pasteProperties(pastePropertiesRequest);
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

		template <typename Callback>
		void getProjectLanguagesAsync(const Callback& callback)
		{
			auto onJobExecute = [this]()
			{
				return getProjectLanguages();
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}
		template <typename Callback>
		void importAsync(const std::vector<Waapi::ImportItemRequest>& importItemsRequest, Import::ContainerNameExistsOption containerNameExistsOption, const juce::String& objectLanguage, const Callback& callback)
		{
			auto onJobExecute = [this, importItemsRequest = importItemsRequest, containerNameExistsOption = containerNameExistsOption, objectLanguage = objectLanguage]()
			{
				return import(importItemsRequest, containerNameExistsOption, objectLanguage);
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

		template <typename Callback>
		void getObjectAncestorsAndDescendantsAsync(const juce::String& objectPath, Callback& callback)
		{
			auto onJobExecute = [objectPath, this]()
			{
				return getObjectAncestorsAndDescendants(objectPath);
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

		template <typename Callback>
		void getObjectAncestorsAndDescendantsLegacyAsync(const juce::String& objectPath, Callback& callback)
		{
			auto onJobExecute = [objectPath, this]()
			{
				return getObjectAncestorsAndDescendantsLegacy(objectPath);
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

		template <typename Callback>
		void getObjectAsync(const juce::String& objectPath, Callback& callback)
		{
			auto onJobExecute = [objectPath, this]()
			{
				return getObject(objectPath);
			};

			threadPool.addJob(new AsyncJob(onJobExecute, callback), true);
		}

	private:
		juce::ThreadPool threadPool;
	};

	class WaapiClientWatcher
		: private juce::Thread
	{
	public:
		WaapiClientWatcher(juce::ValueTree applicationState, WaapiClient& waapiClient, WaapiClientWatcherConfig&& waapiClientWatcherConfig);

		void start();
		void stop();
		void changeParameters(const juce::String& ip, int port);

	private:
		juce::ValueTree applicationState;
		juce::CachedValue<juce::String> projectId;
		juce::CachedValue<bool> waapiConnected;
		juce::CachedValue<bool> wwiseObjectsChanged;

		juce::ValueTree languages;

		WaapiClientWatcherConfig waapiClientWatcherConfig;
		WaapiClient& waapiClient;

		int connectionRetryDelay;

		uint64_t projectLoadedSubscriptionId{0};
		uint64_t projectPostClosedSubscriptionId{0};
		uint64_t objectCreatedEventSubscriptionId{0};
		uint64_t objectPostDeletedEventSubscriptionId{0};
		uint64_t objectNameChangedEventSubscriptionId{0};

		// protected by guiMutex except for on construction
		std::mutex guiMutex;
		bool shouldReconnect = false;
		juce::String ip;
		int port;

		void run() override;

		void setProjectId(const juce::String& projectId);
		void setWaapiConnected(bool connected);
		void setWwiseObjectsChanged(bool changed);
		void disconnectFromWaapi();
	};
} // namespace AK::WwiseTransfer
