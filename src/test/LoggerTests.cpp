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

#include "Core/Logger.h"

#include <catch2/catch_test_macros.hpp>

namespace AK::WwiseTransfer::Test
{
	TEST_CASE("Logger size limit")
	{
		constexpr juce::int64 maxSize = 2'000;
		constexpr juce::int64 trimToSize = 1'000;

		auto tmpDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)
		                  .getChildFile("temp_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt()));

		tmpDir.createDirectory();

		auto logFile = tmpDir.getChildFile("Log_test.txt");

		auto writeMessages = [&](int count)
		{
			Logger logger(logFile, "Test Log", maxSize, trimToSize);

			juce::Logger::setCurrentLogger(&logger);

			for(int i = 0; i < count; ++i)
				juce::Logger::writeToLog("message " + juce::String(i).paddedLeft('0', 3));

			juce::Logger::setCurrentLogger(nullptr);
		};

		SECTION("Nothing is removed while under the limit")
		{
			writeMessages(5);

			auto content = logFile.loadFileAsString();

			REQUIRE(content.contains("message 000"));
			REQUIRE(content.contains("message 004"));
			REQUIRE_FALSE(content.contains("Older entries were removed"));
		}

		SECTION("Oldest entries are erased once the limit is reached, newest are kept")
		{
			writeMessages(200);

			auto content = logFile.loadFileAsString();

			REQUIRE(logFile.getSize() <= maxSize);
			REQUIRE(content.contains("message 199"));
			REQUIRE_FALSE(content.contains("message 000"));
			REQUIRE(content.contains("Older entries were removed"));
		}

		SECTION("The file never exceeds the limit, whatever the number of messages written")
		{
			juce::int64 largestSize = 0;

			{
				Logger logger(logFile, "Test Log", maxSize, trimToSize);

				juce::Logger::setCurrentLogger(&logger);

				for(int i = 0; i < 500; ++i)
				{
					juce::Logger::writeToLog("message " + juce::String(i).paddedLeft('0', 3));

					largestSize = juce::jmax(largestSize, logFile.getSize());
				}

				// Checked after the logger is unregistered, so that a failure cannot leave a dangling current logger.
				juce::Logger::setCurrentLogger(nullptr);
			}

			REQUIRE(largestSize <= maxSize);
		}

		SECTION("An existing file larger than the limit is trimmed")
		{
			logFile.replaceWithText(juce::String::repeatedString("message old\n", 1'000)); // About 12 KB.

			writeMessages(1);

			REQUIRE(logFile.getSize() <= maxSize);
			REQUIRE(logFile.loadFileAsString().contains("message 000"));
		}

		SECTION("Trimming never leaves a partial line")
		{
			writeMessages(200);

			for(const auto& line : juce::StringArray::fromLines(logFile.loadFileAsString()))
			{
				if(line.contains("essage"))
					REQUIRE(line.matchesWildcard("????-??-?? ??:??:?? message ???", false)); // Each logged line is a complete timestamp followed by its message.
			}
		}

		tmpDir.deleteRecursively();
	}
} // namespace AK::WwiseTransfer::Test
