#pragma once

// Windows (MSVC, MinGW)
#if defined(_WIN32)
	#ifdef ENGINE_BUILD 
		#define ENGINE_API __declspec(dllexport) 
	#else 
		#define ENGINE_API __declspec(dllimport) 
	#endif
// Linux/Unix (GCC, Clang)
#elif defined(__GNUC__) || defined(__clang__)
		#define ENGINE_API __attribute__((visibility("default")))
// Fallback for unsupported compilers
#else
	#define ENGINE_API
#endif