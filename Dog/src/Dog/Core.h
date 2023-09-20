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

#ifdef DOG_DEBUG
	#define DOG_ENABLE_ASSERTS
#endif

#ifdef DOG_ENABLE_ASSERTS
	#define DOG_ASSERT(x, ...) { if(!(x)) { DOG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define DOG_CORE_ASSERT(x, ...) { if(!(x)) { DOG_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	#define DOG_ASSERT(x, ...)
	#define DOG_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define DOG_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)