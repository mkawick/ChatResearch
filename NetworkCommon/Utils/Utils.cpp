// Utils.cpp

#include "Utils.h"

#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <time.h>
#include <errno.h>

#if PLATFORM == PLATFORM_WINDOWS
   #include <windows.h>
   #include <direct.h>
   #include <mmsystem.h>
   #include <sys/stat.h>
   #include <sys/types.h>
#pragma warning (disable:4996)
#else
   #include <sys/time.h>
   #include <termios.h>
   #include <unistd.h>
   #include <fcntl.h>
   #include <sys/types.h>
   #include <sys/stat.h>  // not too sure about this
#endif

//#define BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS  // workaround for compile error on linux


#include <climits>


#if PLATFORM == PLATFORM_WINDOWS
#define mkdir(a,b) _mkdir(a)
#else
   #ifndef min
   #define min(a,b)            (((a) < (b)) ? (a) : (b))
   #endif
#define _stat stat
#endif

using namespace std;

//////////////////////////////////////////////////////////////////////////

string   Get0PrefixedValue( int value )
{
   if( value == 0 )
      return string ("00");

   ostringstream Convert;
   Convert << value;

   string ret;
   if( value < 10 )
   {
      ret = "0";
   }
   ret += Convert.str();

   return ret;
   
}

//////////////////////////////////////////////////////////////////////////

string GetDateInUTC( int diffDays, int diffHours, int diffMinutes )
{
   time_t rawtime;
   time( &rawtime );
   struct tm *now = gmtime( &rawtime ); // get time now

   if( diffDays || diffHours || diffMinutes )
   {
      struct tm* temp = now;
      temp->tm_mday += diffDays;
      temp->tm_hour += diffHours;
      temp->tm_min += diffMinutes;
      rawtime = mktime( temp );
      now = gmtime( &rawtime );
   }

#if PLATFORM != PLATFORM_MAC
   string strMonth( Get0PrefixedValue( now->tm_mon + 1 ) );
   string strDay( Get0PrefixedValue( now->tm_mday ) );
   string strHour( Get0PrefixedValue( now->tm_hour ) );
   string strMin( Get0PrefixedValue( now->tm_min ) );
   string strSec( Get0PrefixedValue( now->tm_sec ) );


   ostringstream Convert;
   Convert << (now->tm_year + 1900) << '-'  
         << strMonth << '-' 
         << strDay << " "
         << strHour << ":"
         << strMin << ":"
         << strSec;
   return Convert.str();
#else
   char buffer[256];
   sprintf( buffer, "%04d-%02d-%02d %02d:%02d:%02d", (now->tm_year + 1900), (now->tm_mon + 1), now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec );

   return string( buffer );
#endif
}

//////////////////////////////////////////////////////////////////////////

string GetDateInUTC( time_t t )
{
   struct tm * now = gmtime( & t );
   
#if PLATFORM != PLATFORM_MAC
   string strMonth( Get0PrefixedValue( now->tm_mon + 1 ) );
   string strDay( Get0PrefixedValue( now->tm_mday ) );
   string strHour( Get0PrefixedValue( now->tm_hour ) );
   string strMin( Get0PrefixedValue( now->tm_min ) );
   string strSec( Get0PrefixedValue( now->tm_sec ) );
   
   
   ostringstream Convert;
   Convert << (now->tm_year + 1900) << '-'
   << strMonth << '-'
   << strDay << " "
   << strHour << ":"
   << strMin << ":"
   << strSec;
   return Convert.str();
#else
   char buffer[256];
   sprintf( buffer, "%04d-%02d-%02d %02d:%02d:%02d", (now->tm_year + 1900), (now->tm_mon + 1), now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec );
   
   return string( buffer );
#endif
}

U64   GetDateFromString( const char* UTCFormatted )
{   
   time_t now = time(0);
   struct tm *nowtm;
   nowtm = gmtime(&now);
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

//////////////////////////////////////////////////////////////////////////

std::string    CreatePrintablePair( const std::string& key, const std::string& value )
{
   string text = " { ";
   text += key;
   text += ", ";
   text += value;
   text += " } ";
   return text;
}

//////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////

bool           HasTimeWindowExpired( const time_t& currentTime, const time_t& windowBegin, int numSecondsForWindow )
{
   int secondsElapsed = static_cast< int >( difftime( currentTime, windowBegin ) );
   if( secondsElapsed < 1 ) // already passed
      return true;
   if( numSecondsForWindow < secondsElapsed )
      return true;

   return false;
}

////////////////////////////////////////////////////////

bool           InCurrentTimeWindow( const time_t& currentTime, const time_t& windowBegin, int numSecondsForWindow )
{
   int secondsElapsed = static_cast< int >( difftime( currentTime, windowBegin ) );
   if( secondsElapsed < 1 ) // already passed
      return false;
   if( numSecondsForWindow < secondsElapsed )
      return false;

   return true;
}

//////////////////////////////////////////////////////////

time_t  ZeroOutMinutes( time_t currentTime )
{
   struct tm * now = gmtime( &currentTime );
   now->tm_min = 0;
   now->tm_sec = 0;
   return mktime( now );
}

time_t  ZeroOutHours( time_t currentTime )
{
   struct tm * now = gmtime( &currentTime );
   now->tm_hour = 0;
   now->tm_min = 0;
   now->tm_sec = 0;
   return mktime( now );
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

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
   nowtm = gmtime(&now);
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


//////////////////////////////////////////////////////////////////////////
// found here and is the fastest
// http://stackoverflow.com/questions/12774207/fastest-way-to-check-if-a-file-exist-using-standard-c-c11-c

bool DoesFileExist ( const std::string& name ) 
{
  struct _stat buffer;
  return ( _stat( name.c_str(), &buffer ) == 0 ); 
}

time_t   GetFileModificationTime( const std::string& name )
{
   struct _stat fileInfo;
   if( _stat( name.c_str(), &fileInfo ) == 0 )
   {
      return  fileInfo.st_mtime;
   }
   return 0;
}
/*
static int do_mkdir(const char *path, mode_t mode)
{
    Stat            st;
    int             status = 0;

    if (stat(path, &st) != 0)
    {
        if (mkdir(path, mode) != 0 && errno != EEXIST)
            status = -1;
    }
    else if (!S_ISDIR(st.st_mode))
    {
        errno = ENOTDIR;
        status = -1;
    }

    return(status);
}*/

bool     CreateSubDirectory( std::string dir_path )
{
    size_t pre=0,pos;
    std::string dir;
    int mdret;
    int mode = 0777;

    if(dir_path[dir_path.size()-1]!='/')
    {
        // force trailing / so we can handle everything in loop
        dir_path+='/';
    }

    while((pos=dir_path.find_first_of('/',pre))!=std::string::npos)
    {
        dir=dir_path.substr(0,pos++);
        pre=pos;
        if(dir.size()==0) continue; // if leading / first time is 0 length
        if((mdret=mkdir( dir_path.c_str(), mode )) && 
           errno!=EEXIST)
        {
            return true;
        }
    }
    return false;
}


U16   CalculateChecksum( U8* fileData, int fileSize )
{
   U16 checksum = 0;
   int halfway = fileSize >> 1;
   int extraByte = fileSize %1;
   U16* twoBytePtr = reinterpret_cast< U16* >( fileData );

   for( int index=0; index<halfway; index++ )
   {
      U16 value = htons( twoBytePtr[ index ] );
      checksum += value;
   }
   if( extraByte )
   {
      checksum += htons( fileData[ fileSize -1 ] );
   }

   return checksum;
}

//////////////////////////////////////////////////////////////////////////