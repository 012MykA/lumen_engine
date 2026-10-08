#pragma once

#if LUMEN_EXPORT_DEPRECATED

	#if defined(LUMEN_PLATFORM_WINDOWS)
		#if defined(LUMEN_DYNAMIC_LINK)
			#if defined(LUMEN_BUILD_DLL)
				#define __declspec(dllexport)
			#else
				#define __declspec(dllimport)
			#endif
		#else
			#define LUMEN_API
		#endif

	#elif defined(LUMEN_PLATFORM_LINUX)
		#if defined(LUMEN_DYNAMIC_LINK)
			#if defined(LUMEN_BUILD_DLL)
				#define __attribute__((visibility("default")))
			#else
				#define LUMEN_API
			#endif
		#else
			#define LUMEN_API
		#endif
	#else
		#define LUMEN_API
	#endif

#endif
