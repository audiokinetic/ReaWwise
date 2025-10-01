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

#include "Helpers/FileHelper.h"

#include <catch2/catch_test_macros.hpp>

namespace AK::WwiseTransfer::Test
{
	TEST_CASE("countModifiedFilesInDirectoriesSince")
	{
		SECTION("One directory, multiple files created at same time")
		{
			auto tmpDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)
			                  .getChildFile("temp_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt()));

			tmpDir.createDirectory();

			auto time = juce::Time::getCurrentTime();

			juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 100);

			tmpDir.getChildFile("tempFile1.txt").create();
			tmpDir.getChildFile("tempFile2.txt").create();

			std::set<juce::File> directorySet = {
				tmpDir};

			REQUIRE(FileHelper::countModifiedFilesInDirectoriesSince(directorySet, time) == 2);

			juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 100);

			time = juce::Time::getCurrentTime();

			REQUIRE(FileHelper::countModifiedFilesInDirectoriesSince(directorySet, time) == 0);
		}

		SECTION("Multiple directories, multiple files created at different times")
		{
			auto tmpDir1 = juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)
			                   .getChildFile("temp_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt()));

			auto tmpDir2 = juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)
			                   .getChildFile("temp_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt()));

			tmpDir1.createDirectory();
			tmpDir2.createDirectory();

			auto time = juce::Time::getCurrentTime();

			juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 100);

			tmpDir1.getChildFile("tempFile1.txt").create();
			tmpDir2.getChildFile("tempFile2.txt").create();

			std::set<juce::File> directorySet = {
				tmpDir1,
				tmpDir2};

			REQUIRE(FileHelper::countModifiedFilesInDirectoriesSince(directorySet, time) == 2);

			time = juce::Time::getCurrentTime();

			juce::Time::waitForMillisecondCounter(juce::Time::getMillisecondCounter() + 100);

			tmpDir1.getChildFile("tempFile3.txt").create();

			REQUIRE(FileHelper::countModifiedFilesInDirectoriesSince(directorySet, time) == 1);
		}
	}

	TEST_CASE("deleteOldFiles")
	{
		auto tmpDir = juce::File::getSpecialLocation(juce::File::SpecialLocationType::tempDirectory)
		                  .getChildFile("temp_" + juce::String::toHexString(juce::Random::getSystemRandom().nextInt()));

		tmpDir.createDirectory();

		auto oldMatchingFile = tmpDir.getChildFile("Log_2020-01-01.txt");
		auto recentMatchingFile = tmpDir.getChildFile("Log_2020-01-02.txt");
		auto oldOtherFile = tmpDir.getChildFile("settings.txt");

		oldMatchingFile.create();
		recentMatchingFile.create();
		oldOtherFile.create();

		auto thirtyDaysAgo = juce::Time::getCurrentTime() - juce::RelativeTime::days(30);
		oldMatchingFile.setLastModificationTime(thirtyDaysAgo);
		oldOtherFile.setLastModificationTime(thirtyDaysAgo);

		FileHelper::deleteOldFiles(tmpDir, "Log_*.txt", 14);

		SECTION("Only files matching the wildcard and older than the maximum age are deleted")
		{
			REQUIRE_FALSE(oldMatchingFile.exists());
			REQUIRE(recentMatchingFile.exists());
			REQUIRE(oldOtherFile.exists());
		}

		tmpDir.deleteRecursively();
	}
} // namespace AK::WwiseTransfer::Test
