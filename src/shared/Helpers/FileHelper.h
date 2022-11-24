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

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <set>

namespace AK::WwiseTransfer::FileHelper
{
	inline int countModifiedFilesInDirectoriesSince(const std::set<juce::File>& directorySet, const juce::Time& lastWriteTime)
	{
		int modifiedFiles = 0;

		for(const auto& directory : directorySet)
		{
			for(const auto& file : directory.findChildFiles(juce::File::TypesOfFileToFind::findFiles, false))
			{
				if(file.getLastModificationTime() > lastWriteTime)
					++modifiedFiles;
			}
		}

		return modifiedFiles;
	}
} // namespace AK::WwiseTransfer::FileHelper
