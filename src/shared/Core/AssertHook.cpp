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

#include <AK/Tools/Common/AkAssert.h>
#include <cassert>
#include <stdio.h>

#ifdef AK_ENABLE_ASSERTS

namespace AK::WwiseTransfer
{
	void AssertHook(const char* in_pszExpression, const char* in_pszFileName, int in_lineNumber)
	{
		char buff[1024];
		snprintf(buff, sizeof(buff), "AKASSERT: %s, %s - %d", in_pszExpression, in_pszFileName, in_lineNumber);
		assert(0 && buff);
	}
} // namespace AK::WwiseTransfer

AkAssertHook g_pAssertHook = AK::WwiseTransfer::AssertHook;

#endif
