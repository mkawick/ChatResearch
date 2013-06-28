// Utils.h

#pragma once

#include "../DataTypes.h"
#include <string>

std::string    GenerateUUID( U32 xorValue = 0 );
U64            GenerateUniqueHash( const std::string& str );
std::string    GetDateInUTC( int diffDays = 0, int diffHours = 0, int diffMinutes = 0 );
U64            GetDateFromString( const char* UTCFormatted );
std::string    CreatePrintablePair( const std::string& key, const std::string& value );
U32            GetCurrentMilliseconds();

void           PrintCurrentTime();

std::string    Trim(const std::string& str,
                 const std::string& whitespace = " \t");

// use case:
// const std::string foo = "    too much\t   \tspace\t\t\t  ";
// std::cout << foo << std::endl;
// std::cout << "[" << reduce(foo) << "]" << std::endl;
// std::cout << "[" << reduce(foo, "-") << "]" << std::endl;
//
// > [too much               space] 
// > [too much space]  
// > [too-much-space]  
std::string    Reduce(const std::string& str,
                   const std::string& fill = " ",
                   const std::string& whitespace = " \t");
std::string    OpenAndLoadFile( const std::string& path );

#if PLATFORM != PLATFORM_WINDOWS
int kbhit();
int getch();
#define Sleep(a)           usleep(( useconds_t )(a * 1000))

#define  SOCKET_ERROR   -1
#define closesocket  close
#endif

