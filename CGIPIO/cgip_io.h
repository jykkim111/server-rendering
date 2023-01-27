#pragma once

#ifdef CGIPIO_EXPORTS
	#define CGIPIO_DLL __declspec(dllexport)
#else
	#define CGIPIO_DLL __declspec(dllimport)
#endif

#include <assert.h>
