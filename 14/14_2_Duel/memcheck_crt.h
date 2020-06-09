#pragma once
#ifndef MEMCHECK_CRT_
#define MEMCHECK_CRT_

#if defined(_MSC_VER) && !defined(__INTEL_COMPILER) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define ENABLE_CRT\
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);\
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);\
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDOUT);
#else
#define ENABLE_CRT ;
#endif

#endif // !MEMCHECK_CRT_
