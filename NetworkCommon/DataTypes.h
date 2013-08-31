// DataTypes.h

#pragma once

#include "Platform.h"

typedef unsigned char   U8;
typedef unsigned short  U16;	
typedef unsigned int    U32;
typedef char            S8;
typedef short           S16;
typedef int             S32;

#if PLATFORM == PLATFORM_WINDOWS

typedef unsigned __int64   U64;
typedef __int64            S64;

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <inttypes.h>

typedef uint64_t        U64;
typedef int64_t         S64;

#endif

typedef  U32    ConnectionId;
typedef  U64    stringhash;

struct   TimerInfo
{
   U32   scheduleTimeMs;
   U32   lastTimeMs;
   U32   timerId;
};

enum PlatformType
{
   Platform_None,
   Platform_ios,
   Platform_android,
   Platform_pc,
   Platform_mac,
   Platform_vita,
   Platform_xbox,
   Platform_blackberry,
   Platform_wii,
   Platform_count
};