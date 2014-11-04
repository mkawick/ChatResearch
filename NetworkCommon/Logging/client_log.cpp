#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include "../Platform.h"
using namespace std;

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

void LogMessage(int priority, const char *fmt, ...)
{
   const int maxStringLength = 256;
   va_list args;
   va_start(args, fmt);

   char buffer[maxStringLength];
   vsnprintf( buffer, maxStringLength, fmt, args);

   va_end(args);
#if defined(ANDROID)
   __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, "%s", buffer);
#else
   cout << buffer << endl;
#endif
}