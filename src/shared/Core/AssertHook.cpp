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
