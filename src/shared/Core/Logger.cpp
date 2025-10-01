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

#include "Logger.h"

#include "Helpers/FileHelper.h"

#include <cstring>

namespace AK::WwiseTransfer
{
	namespace
	{
		const auto newLineSizeBytes = static_cast<juce::int64>(std::strlen(juce::NewLine::getDefault()));

		juce::int64 getSizeOnDisk(const juce::String& message)
		{
			return static_cast<juce::int64>(message.getNumBytesAsUTF8()) + newLineSizeBytes;
		}

		juce::File getDefaultLogFile(const juce::String& applicationName)
		{
			return juce::FileLogger::getSystemLogFileFolder().getChildFile(applicationName)
#ifdef WIN32
				.getChildFile("Logs")
#endif
				.getChildFile("Log_" + juce::Time::getCurrentTime().formatted("%Y-%m-%d"))
				.withFileExtension(".txt");
		}
	} // namespace

	namespace LoggerConstants
	{
		constexpr juce::int64 maxLogFileSizeBytes = 25 * 1024 * 1024;
		constexpr juce::int64 trimToLogFileSizeBytes = maxLogFileSizeBytes / 2;
		constexpr juce::int64 trimRetryIntervalBytes = 1024 * 1024; // If trimming fails, wait for this much more to be logged before trying again.
		constexpr int maxLogAgeDays = 14;
	} // namespace LoggerConstants

	Logger::Logger(const juce::String& applicationName)
		: Logger(getDefaultLogFile(applicationName), applicationName + " Log", LoggerConstants::maxLogFileSizeBytes, LoggerConstants::trimToLogFileSizeBytes)
	{
		FileHelper::deleteOldFiles(getLogFile().getParentDirectory(), "Log_*.txt", LoggerConstants::maxLogAgeDays);
	}

	Logger::Logger(const juce::File& logFile, const juce::String& welcomeMessage, juce::int64 maxFileSizeBytes, juce::int64 trimToBytes)
		: juce::FileLogger(logFile, welcomeMessage, -1) // No initial trim: an existing file is trimmed on the first message that exceeds the limit.
		, maxFileSizeBytes(maxFileSizeBytes)
		, trimToBytes(trimToBytes)
		, nextTrimAtBytes(maxFileSizeBytes)
	{
		jassert(trimToBytes > 0 && trimToBytes < maxFileSizeBytes);

		bytesLogged = getLogFile().getSize();
	}

	void Logger::trimLogFile()
	{
		const auto sizeBeforeTrim = getLogFile().getSize();

		juce::FileLogger::trimFileSize(getLogFile(), trimToBytes);

		const auto sizeAfterTrim = getLogFile().getSize();

		if(sizeAfterTrim < sizeBeforeTrim)
		{
			bytesLogged = sizeAfterTrim;
			nextTrimAtBytes = maxFileSizeBytes;

			juce::String marker;
			marker << juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S") << " Log file reached its size limit. Older entries were removed.";

			juce::FileLogger::logMessage(marker);
			bytesLogged += getSizeOnDisk(marker);
		}
		else
		{
			// The file could not be trimmed (e.g. it is locked by another program). Don't retry on every message.
			bytesLogged = sizeBeforeTrim;
			nextTrimAtBytes = sizeBeforeTrim + LoggerConstants::trimRetryIntervalBytes;
		}
	}

	void Logger::logMessage(const juce::String& message)
	{
		juce::String messageWithTimeStamp;

		messageWithTimeStamp << juce::Time::getCurrentTime().formatted("%Y-%m-%d %H:%M:%S") << " " << message;

		{
			const juce::ScopedLock lock(writeLock);

			const auto messageSize = getSizeOnDisk(messageWithTimeStamp);

			if(bytesLogged + messageSize > nextTrimAtBytes)
				trimLogFile();

			juce::FileLogger::logMessage(messageWithTimeStamp);
			bytesLogged += messageSize;
		}

		auto onLogMessage = [&](IListener& listener)
		{
			listener.onLogMessage(messageWithTimeStamp);
		};

		listeners.call(onLogMessage);
	}

	void Logger::addListener(IListener& listener)
	{
		listeners.add(&listener);
	}

	void Logger::removeListener(IListener& listener)
	{
		listeners.remove(&listener);
	}
} // namespace AK::WwiseTransfer
