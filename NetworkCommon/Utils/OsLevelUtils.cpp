// OsLevelUtils.cpp

#include "OsLevelUtils.h"
#include "../Platform.h"

#include <wchar.h>
#include <stdio.h>

#if PLATFORM == PLATFORM_WINDOWS
   #include <windows.h>
#endif

void SetConsoleWindowTitle( const char* name )
{
#if PLATFORM == PLATFORM_WINDOWS
   //system( name );
   const int stringWidth = MAX_PATH;
   wchar_t  wide_name[stringWidth];
   swprintf( wide_name, stringWidth, L"%hs", name );
   SetConsoleTitle( wide_name );
#else
   printf("%c]0;%s%c", '\033', name, '\007');

#endif
}