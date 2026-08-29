#pragma once

#if defined(LUMEN_PLATFORM_WINDOWS)
	#if defined(LUMEN_DYNAMIC_LINK)
		#if defined(LUMEN_BUILD_DLL)
			#define LUMEN_API __declspec(dllexport)
		#else
			#define LUMEN_API __declspec(dllimport)
		#endif
	#else
		#define LUMEN_API
	#endif

#elif defined(LUMEN_PLATFORM_LINUX)
	#if defined(LUMEN_DYNAMIC_LINK)
		#if defined(LUMEN_BUILD_DLL)
			#define LUMEN_API __attribute__((visibility("default")))
		#else
			#define LUMEN_API
		#endif
	#else
		#define LUMEN_API
	#endif
#else
	#define LUMEN_API
#endif
