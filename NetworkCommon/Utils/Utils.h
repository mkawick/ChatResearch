// Utils.h

#pragma once

#include "../DataTypes.h"
#include <boost/type_traits/is_signed.hpp>
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

template < class T >
bool ConvertFromString( const std::string& InputString, T& Value )
{
   Value = 0;
   T tempValue = 0;
   int numConverted = 0;
   bool makeNegative = false;
   for(std::string::const_iterator it = InputString.begin() ; it < InputString.end(); ++it)
   {
      char c = *it;
      if( c>= '0' && c<='9' )
      {
         Value = Value * 10 + (c - '0' );
         numConverted ++;
      }
      else if( c == '-' && numConverted == 0 )
      {
         makeNegative = true;
      }
      else
      {
         return false;
      }
   }
   if( makeNegative == true && boost::is_signed<T>::value == true )
   {
      Value *= -1;
   }
   return true;
}