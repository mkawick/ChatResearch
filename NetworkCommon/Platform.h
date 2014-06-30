// Platform.h

#pragma once
// platform detection

#include <memory.h>// this needs a beter home

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32) || defined(WIN32)
#define PLATFORM   PLATFORM_WINDOWS

#elif defined(__APPLE__)
#define PLATFORM   PLATFORM_MAC

#elif defined(ANDROID)
#include <pthread.h>
typedef unsigned int size_t;
#define PLATFORM   PLATFORM_UNIX

#else
#define PLATFORM   PLATFORM_UNIX
#endif

