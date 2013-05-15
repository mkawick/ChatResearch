// Utils.cpp

#include "Utils.h"

#include <sstream>
#include <iostream>
#include <iomanip>
using namespace std;

#include <ctime>
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>


#if PLATFORM == PLATFORM_WINDOWS
   #include <windows.h>
   #include <mmsystem.h>
#endif
#pragma warning (disable:4996)

//-------------------------------------------------------------------------

string   GenerateUUID( U32 xorValue )
{
   string returnString;

   for( int i=0; i< 2; i++ )
   {
      U32 value = ( (rand() % 256) << 24 ) + ( (rand() % 256) << 16 ) + ( (rand() % 256) << 8 ) + (rand() % 256);
      value ^= xorValue;// xor with the elapsed time.
      stringstream stream;
      stream << hex << value;
      returnString += stream.str();
   }

   while( returnString.size() < 16 )// fill in with 0's to be sure that we have the proper length
   {
      returnString += '0';
   }
   return returnString;
}

//-------------------------------------------------------------------------

U64      GenerateUniqueHash( const std::string& str )
{
   size_t seed = 1027; // a good starting point
   std::string randomtext("playdek.games"); // add an unpredictable starting point

   boost::hash_combine( seed, str );
   U64 temp = seed;
   boost::hash_combine( seed, randomtext );
   temp = ( temp << 32 ) + seed;

   return temp;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

string   Get0PrefixedValue( int value )
{
   ostringstream Convert;
   Convert << value;

   string ret = Convert.str();
   if( value < 10 )
   {
      if( value == 0 )
         return string ("00");

      ret = "0";
      ret += Convert.str();
   }

   return ret;
}

//-------------------------------------------------------------------------

string GetDateInUTC()
{
   ostringstream Convert;
   time_t t = time(0);   // get time now
   struct tm * now = localtime( & t );

   string strMonth = Get0PrefixedValue( now->tm_mon + 1 ) ;
   string strDay = Get0PrefixedValue( now->tm_mday + 1 );
   string strHour = Get0PrefixedValue( now->tm_hour );
   string strMin = Get0PrefixedValue( now->tm_min );
   string strSec = Get0PrefixedValue( now->tm_sec );

   Convert << (now->tm_year + 1900) << '-'  
         << strMonth << '-' 
         << strDay << " "
         << strHour << ":"
         << strMin << ":"
         << strSec;

   return Convert.str();
}

U64   GetDateFromString( const char* UTCFormatted )
{   
   time_t now = time(0);
   struct tm *nowtm;
   nowtm = localtime(&now);
   sscanf( UTCFormatted, "%d-%d-%d %d:%d:%d", &nowtm->tm_year, &nowtm->tm_mon, &nowtm->tm_wday,  
      &nowtm->tm_hour, &nowtm->tm_min, &nowtm->tm_sec );

   nowtm->tm_year -= 1898;

   return mktime( nowtm );
}

//-------------------------------------------------------------------------

std::string    CreatePrintablePair( const std::string& key, const std::string& value )
{
   string text = " { ";
   text += key;
   text += ", ";
   text += value;
   text += " } ";
   return text;
}

//-------------------------------------------------------------------------

U32            GetCurrentMilliseconds()
{
#if PLATFORM == PLATFORM_WINDOWS
   return timeGetTime();
#else
   return 0;
#endif
}
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

std::string    Trim( const std::string& str,
                 const std::string& whitespace )
{
   const string::size_type begin = str.find_first_not_of( whitespace );
   const string::size_type end   = str.find_last_not_of( whitespace );

   return str.substr( begin, end-begin + 1 );
}

//-------------------------------------------------------------------------

std::string    Reduce( const std::string& str,
                      const std::string& fill,
                      const std::string& whitespace )
{
    // trim first
    string result = Trim( str, whitespace );

    // replace sub ranges
    size_t beginSpace = result.find_first_of( whitespace );
    while ( beginSpace != std::string::npos )
    {
        const string::size_type endSpace = result.find_first_not_of( whitespace, beginSpace );
        const string::size_type range = endSpace - beginSpace;

        result.replace( beginSpace, range, fill );

        const string::size_type newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

//-------------------------------------------------------------------------
