#pragma once

#ifdef CGIPCOMMON_EXPORTS
	#define CGIPCOMMON_DLL __declspec(dllexport)
#else
	#define CGIPCOMMON_DLL __declspec(dllimport)
#endif

#include <assert.h>
