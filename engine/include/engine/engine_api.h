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
	#ifdef ENGINE_BUILD 
		#define ENGINE_API __attribute__((visibility("default")))
	#else 
		#define ENGINE_API 
#endif

// Fallback for unsupported compilers
#else
	#define ENGINE_API
	#pragma warning Unknown dynamic link import/export semantics.
#endif