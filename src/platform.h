#ifndef PLATFORM_H__HEADER_GUARD__
#define PLATFORM_H__HEADER_GUARD__

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#	define __PLATFORM_WINDOWS__
#elif defined(__APPLE__)
#	define __PLATFORM_APPLE__
#elif defined(__linux__) || defined(__gnu_linux__) || defined(linux)
#	define __PLATFORM_LINUX__
#elif defined(__unix__) || defined(unix)
#	define __PLATFORM_UNIX__
#else
#	define __PLATFORM_UNKNOWN__
#endif

#if defined(__clang__)
#	define __COMPILER_CLANG__
#elif defined(__GNUC__)
#	define __COMPILER_GCC__
#elif defined(_MSC_VER)
#	define __COMPILER_MSVC__
#else
#	define __COMPILER_UNKNOWN__
#endif

#endif
