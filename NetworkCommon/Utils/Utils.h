// Utils.h

#pragma once

#include "../DataTypes.h"
#include <string>
#include <vector>

enum 
{
   TypicalMaxHexLenForNetworking = 15,
   TypicalMaxHexLen = 16
};

std::string    Get0PrefixedValue( int value );
std::string    GetDateInUTC( int diffDays = 0, int diffHours = 0, int diffMinutes = 0 );
std::string    GetDateInUTC( time_t t );
U64            GetDateFromString( const char* UTCFormatted );
int            GetDiffTimeFromRightNow( const char* UTCFormatted );// negative times are in the past
std::string    CreatePrintablePair( const std::string& key, const std::string& value );
U32            GetCurrentMilliseconds();

bool           HasTimeWindowExpired( const time_t& currentTime, const time_t& windowBegin, int numSecondsForWindow );
bool           InCurrentTimeWindow( const time_t& currentTime, const time_t& windowBegin, int numSecondsForWindow );

time_t         ZeroOutMinutes( time_t currentTime );
time_t         ZeroOutHours( time_t currentTime );

void           PrintCurrentTime();

const std::string    OpenAndLoadFile( const std::string& path );

#if PLATFORM != PLATFORM_WINDOWS
int kbhit();
int getch();
#define Sleep(microSeconds)           usleep(( useconds_t )(microSeconds * 1000))

#define  SOCKET_ERROR   -1

#ifndef closesocket
#define closesocket  close
#endif

#endif

template <typename T>
class SetValueOnExit
{
public:
   SetValueOnExit( T& valueToChange, T valueToSet ): m_valueToChange( valueToChange ), m_valueToSet( valueToSet ) {}
   ~SetValueOnExit() 
   {
      m_valueToChange = m_valueToSet;
   }
private:
   T& m_valueToChange;
   T m_valueToSet;
};


bool DoesFileExist ( const std::string& name ) ;
time_t   GetFileModificationTime( const std::string& name );

bool     CreateSubDirectory( std::string path );