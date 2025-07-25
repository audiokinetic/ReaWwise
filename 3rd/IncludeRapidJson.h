/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided 
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

  Copyright (c) 2025 Audiokinetic Inc.
*******************************************************************************/
// Based on authoring's IncludeRapidJson.h

#pragma once

#ifdef _MSC_VER
#define BEGIN_SUPPRESS_WARNINGS __pragma(warning(push, 0))
#define END_SUPPRESS_WARNINGS __pragma(warning(pop))
#else
#define BEGIN_SUPPRESS_WARNINGS
#define END_SUPPRESS_WARNINGS
#endif

BEGIN_SUPPRESS_WARNINGS
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
END_SUPPRESS_WARNINGS
