// Utils.cpp

#include "Utils.h"

#include <stdio.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>

#include <ctime>
#include <time.h>

#if PLATFORM == PLATFORM_WINDOWS
   #include <windows.h>
   #include <mmsystem.h>
#pragma warning (disable:4996)
#else
   #include <sys/time.h>
   #include <termios.h>
   #include <unistd.h>
   #include <fcntl.h>
#endif

#define BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS  // workaround for compile error on linux
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/range/algorithm/remove_if.hpp>

#include <climits>
#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>
//#include <boost/chrono/.hpp>

#if PLATFORM != PLATFORM_WINDOWS
   #ifndef min
   #define min(a,b)            (((a) < (b)) ? (a) : (b))
   #endif
#endif

using namespace std;

static boost::random::mt19937 rng;
//-------------------------------------------------------------------------

string   GenerateUUID( U32 xorValue )
{
   string returnString;

   U32 max = INT_MAX;
   //boost::random::mt19937 rng;         // produces randomness out of thin air
                                    // see pseudo-random number generators
   boost::random::uniform_int_distribution<> roll(0, max);
                                       // distribution that maps to 1..6
                                       // see random number distributions
   //int x = six(rng);                   // simulate rolling a die

   for( int i=0; i< 2; i++ )
   {
      //U32 value = ( (rand() % 256) << 24 ) + ( (rand() % 256) << 16 ) + ( (rand() % 256) << 8 ) + (rand() % 256);
      U32 value = roll( rng );
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

U64      GenerateUniqueHash( const std::string& str, int maxHexLen )
{
   size_t seed = 1027; // a good starting point
   std::string randomtext("playdek.games"); // add an unpredictable starting point

   boost::hash_combine( seed, str );
   U64 temp = seed;
   boost::hash_combine( seed, randomtext );
   temp = ( temp << 32 ) + seed;

   if( maxHexLen < 16 )
   {
      U64 mask = -1;
      int numToStrip = ( 16 - maxHexLen ) * 4;// 4 bits per hex char
      mask <<= numToStrip; // push off the most significant which are the most likely to be similar.
      mask >>= numToStrip;
      temp &= mask;
   }

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

string GetDateInUTC( int diffDays, int diffHours, int diffMinutes )
{
   ostringstream Convert;
   time_t t = time(0);   // get time now
   struct tm * now = localtime( & t );
   if( diffDays || diffHours || diffMinutes )
   {
      struct tm* temp = now;
      temp->tm_mday += diffDays;
      temp->tm_hour += diffHours;
      temp->tm_min += diffMinutes;
      t = mktime( temp );
      now = localtime( & t );
      //ctime(&temp);
   }

   string strMonth = Get0PrefixedValue( now->tm_mon + 1 ) ;
   string strDay = Get0PrefixedValue( now->tm_mday );
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

//-------------------------------------------------------------------------

string GetDateInUTC( time_t t )
{
   ostringstream Convert;
   struct tm * now = localtime( & t );
   string strMonth = Get0PrefixedValue( now->tm_mon + 1 ) ;
   string strDay = Get0PrefixedValue( now->tm_mday );
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
   sscanf( UTCFormatted, "%d-%d-%d %d:%d:%d", &nowtm->tm_year, &nowtm->tm_mon, &nowtm->tm_mday,  
      &nowtm->tm_hour, &nowtm->tm_min, &nowtm->tm_sec );

   // based on this http://www.cplusplus.com/reference/ctime/mktime/
   nowtm->tm_year -= 1900;
   nowtm->tm_mon -= 1;

   return mktime( nowtm );
}

int   GetDiffTimeFromRightNow( const char* UTCFormatted )
{
   time_t now = time(0);
   double seconds;

   time_t compareTime = GetDateFromString( UTCFormatted );

   seconds = difftime( now, compareTime );

   return static_cast< int >( seconds );
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
   struct timeval now;
   int rv = gettimeofday( &now, 0 );
   if( rv ) return rv;/// some kind of error
   
   return static_cast< U32 >( now.tv_sec * 1000ul + now.tv_usec / 1000ul );
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

unsigned int split( const std::string &txt, std::vector<std::string> &strs, char ch )
{
   unsigned int pos = txt.find( ch );
   unsigned int initialPos = 0;
   strs.clear();

   // Decompose statement
   while( pos != std::string::npos ) 
   {
      string temp = Trim( txt.substr( initialPos, pos - initialPos + 1 ) );
      strs.push_back( temp );
      initialPos = pos + 1;

      pos = txt.find( ch, initialPos );
   }

   // Add the last one
   strs.push_back( txt.substr( initialPos, min( pos, txt.size() ) - initialPos + 1 ) );

   return strs.size();
}

//-------------------------------------------------------------------------

#if PLATFORM != PLATFORM_WINDOWS
int kbhit()
{
   struct termios oldt, newt;
   int ch;
   int oldf;
   
   tcgetattr(STDIN_FILENO, &oldt);
   newt = oldt;
   newt.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &newt);
   oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
   fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
   
   ch = getchar();
   
   tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
   fcntl(STDIN_FILENO, F_SETFL, oldf);
   
   if(ch != EOF)
   {
      ungetc(ch, stdin);
      return 1;
   }
   
   return 0;
}

int getch()
{
   struct termios oldt, newt;
   int ch;
   int oldf;
   
   tcgetattr(STDIN_FILENO, &oldt);
   newt = oldt;
   newt.c_lflag &= ~(ICANON | ECHO);
   tcsetattr(STDIN_FILENO, TCSANOW, &newt);
   oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
   fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
   
   while( (ch = getchar() ) == EOF )
   {
       Sleep( 30 );// give it a rest.
   }
   
   tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
   fcntl(STDIN_FILENO, F_SETFL, oldf);
   
   return ch;

}
#endif

void  PrintCurrentTime()
{
   char nowBuf[100];
   time_t now = time(0);
   struct tm *nowtm;
   nowtm = localtime(&now);
   strftime(nowBuf, sizeof(nowBuf), "%Y-%m-%d %H:%M:%S", nowtm);

   cout << nowBuf << endl;
}

const string OpenAndLoadFile( const string& path )
{
   string returnString;
   ifstream file( path.c_str(), ios::in|ios::binary|ios::ate );
   std::string temp;
   if( file.is_open() )
   {
      ifstream::pos_type  size = file.tellg();
      int num = size;
      char* memblock = new char [num+1];
      memblock[num] = 0;
      file.seekg (0, ios::beg);
      file.read (memblock, size);
      file.close();
      returnString = memblock;
      returnString += "\0";
      delete memblock;
   }

   
   return returnString;
}

U64 StringToU64( const char * str )
{
    U64 u64Result = 0;
    while (*str != '\0')
    {
        u64Result *= 10 ;
        u64Result += *str -  '0';
        str ++;
    }
    return u64Result;
}


const string  ConvertStringToLower( const string& str )
{
   string retString = str;
   std::transform( retString.begin(), retString.end(), retString.begin(), ::tolower );
   return retString;
}

const string itos(int n)
{
   const int max_size = std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 /*0-terminator*/;
   char buffer[max_size] = {0};
   sprintf(buffer, "%d", n);
   return string(buffer);
}


bool  splitOnFirstFound( vector< string >& listOfStuff, const string& text, const char* delimiter )
{
   size_t position = text.find_first_of( delimiter );
   if( position != string::npos )
   {
      std::string substr1 = text.substr( 0, position );
      std::string substr2 = text.substr( position+1, std::string::npos );// assuming only one
      listOfStuff.push_back( substr1 );
      listOfStuff.push_back( substr2 );
      return true;
   }
   else
   {
      listOfStuff.push_back( text );
      return false;
   }
}

bool  ParseListOfItems( vector< string >& listOfStuff, string text, const char* delimiter, const char* charsToRemove )
{
   //text.erase( boost::remove_if( text.begin(), text.end(), "[]{}"), text.end() );
   if( charsToRemove )
   {
      text.erase( boost::remove_if( text, boost::is_any_of( charsToRemove )), text.end() );
   }

   string separator1( "" );//dont let quoted arguments escape themselves
   string separator2( delimiter );//split on = and :
   string separator3( "\"\'" );//let it have quoted arguments


   boost::escaped_list_separator<char> els( separator1, separator2, separator3 );
   boost::tokenizer<boost::escaped_list_separator<char> > tokens( text, els );

   for (boost::tokenizer<boost::escaped_list_separator<char> >::iterator i(tokens.begin());
      i!=tokens.end(); ++i) 
   {
      listOfStuff.push_back(*i);
   }

   if( listOfStuff.size() > 0 )
      return true;

   return false;
}

string RemoveEnds( std::string s, const char* charsToStrip )
{
   int len = strlen( charsToStrip );

   for( int i=0; i<len; i++ )
   {
      while( *s.begin() == charsToStrip[i] )
      {
         s = s.substr(1, s.size());
      }
      while( *s.rbegin() == charsToStrip[i] )
      {
         s = s.substr(0, s.size()-1);
      }
      //s.erase(remove( s.begin(), s.end(), charsToStrip[i] ),s.end());
   }

   return s;
}