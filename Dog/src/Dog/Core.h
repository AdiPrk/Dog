#pragma once

#ifdef DOG_PLATFORM_WINDOWS
	#ifdef DOG_BUILD_DLL
		#define DOG_API __declspec(dllexport)
	#else
		#define DOG_API __declspec(dllimport)
	#endif
#else
	#error Dog only supports Windows
#endif