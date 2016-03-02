#pragma once

/*
 * Visibility helpers, from https://gcc.gnu.org/wiki/Visibility
 */

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define COMMKIT_HELPER_DLL_IMPORT __declspec(dllimport)
#define COMMKIT_HELPER_DLL_EXPORT __declspec(dllexport)
#define COMMKIT_HELPER_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define COMMKIT_HELPER_DLL_IMPORT __attribute__((visibility("default")))
#define COMMKIT_HELPER_DLL_EXPORT __attribute__((visibility("default")))
#define COMMKIT_HELPER_DLL_LOCAL __attribute__((visibility("hidden")))
#else
#define COMMKIT_HELPER_DLL_IMPORT
#define COMMKIT_HELPER_DLL_EXPORT
#define COMMKIT_HELPER_DLL_LOCAL
#endif
#endif

// Now we use the generic helper definitions above to define COMMKIT_API and COMMKIT_LOCAL.
// COMMKIT_API is used for the public API symbols. It either DLL imports or DLL exports (or does
// nothing for static build)
// COMMKIT_LOCAL is used for non-api symbols.

#ifdef COMMKIT_DLL         // defined if COMMKIT is compiled as a DLL
#ifdef COMMKIT_DLL_EXPORTS // defined if we are building the COMMKIT DLL (instead of using it)
#define COMMKIT_API COMMKIT_HELPER_DLL_EXPORT
#else
#define COMMKIT_API COMMKIT_HELPER_DLL_IMPORT
#endif // COMMKIT_DLL_EXPORTS
#define COMMKIT_LOCAL COMMKIT_HELPER_DLL_LOCAL
#else // COMMKIT_DLL is not defined: this means COMMKIT is a static lib.
#define COMMKIT_API
#define COMMKIT_LOCAL
#endif // COMMKIT_DLL
