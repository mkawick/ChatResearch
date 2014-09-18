#include "server_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

#include <boost/format.hpp>
#include "../Utils/Utils.h"

#include <assert.h>

#pragma message ("BUILD PLATFORM = " STRINGIFY(PLATFORM))

const int   logTempBufferSize = 512;
ofstream    loggingFile;
char        loggingFilterName[ logTempBufferSize ];
time_t      lastTimeLogged = 0;
bool        isCreatingFile = false;

//////////////////////////////////////////////////////////////////////////

void LogMessage(int priority, const std::string& str )// overload
{
   LogMessage( priority, str.c_str() );
}

void  CreateLogFile( const char* externalFilename, const char* processName, bool erasePreviousContents )
{
   cout << "Creating log file: " << externalFilename << endl;

   if( isCreatingFile == true )// simple atomic-like operation
      return;
   isCreatingFile = true;
   assert( externalFilename );
   if( loggingFile.is_open() )
      loggingFile.close();

   strcpy( loggingFilterName, processName );

   if( erasePreviousContents == true )
   {
      remove( externalFilename );
   }
   loggingFile.open ( externalFilename, ofstream::out | ofstream::app );//, ios::out | ios::app | ios::binary);//ios::out | ios::app | ios::trunc | ios::binary );

   if( loggingFile.is_open() == false )
   {
      cout << "Alert: Unable to open log file: " << externalFilename << endl;
   }
   if( loggingFile.fail() )
   { 
      cout << "Alert: Could not write the file: " << externalFilename << endl << flush; 
   }
   isCreatingFile = false;

   if( erasePreviousContents == false )
   {
      const int numLeadingLines = 2;
      loggingFile << endl << endl << endl;
      for( int i=0; i<numLeadingLines; i++ )
         loggingFile << "*****************************************************************" << endl;
      loggingFile << "*****************     " << processName << "     ***************************" << endl;
      loggingFile << "*************     " << GetDateInUTC() << "     ***********************" << endl;
      loggingFile << "***************************   new log   *************************" << endl;
      for( int i=0; i<numLeadingLines; i++ )
         loggingFile << "*****************************************************************" << endl;
   }
}

void  FileLog( int priority, const char* buffer )
{
   if( loggingFile.is_open() == false )
      return;
   bool shouldPrint = true;
   if( buffer[0] == buffer[1] )
   {
      // simple test for separators meant to add clarity for logging. 
      // we do not need special formatting.
      if( buffer[0] == '-' || buffer[0] == '*' || buffer[0] == '=' || buffer[0] == '+' )
      {
         shouldPrint = false;
         loggingFile << buffer << endl;
      }
   }
   if( shouldPrint == true )
   {
      time_t currentTime;
      time( &currentTime );
      
      string timeString;
      if( difftime( currentTime, lastTimeLogged ) > 0 )
      {
         timeString = " - [" + GetDateInUTC() + "]";
         lastTimeLogged = currentTime;
      }
      loggingFile << boost::format("{%1%(p:%4%)}: %|18t|%2% %|80t|%3%" ) % loggingFilterName % buffer % timeString % priority << endl;
   }
   //loggingFile.flush();// let it self-flush... it's too slow otherwise
}

void  FileLogClose()
{
   if( loggingFile.is_open() )
   {
      loggingFile.close();
   }
}


//////////////////////////////////////////////////////////////////////////

#if PLATFORM == PLATFORM_UNIX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void LogOpen()
{
   openlog(PACKAGE_NAME, LOG_CONS, LOG_USER);
}

void LogMessage(int priority, const char *printFormat, ...)
{
   va_list args;
   va_start( args, printFormat );

   vsyslog( priority, printFormat, args );

   va_end( args );

   // write to file
   char buffer[ logTempBufferSize ];

   va_start( args, printFormat );
   vsprintf ( buffer, printFormat, args);
   va_end( args );

   cout << buffer << endl;

   FileLog( priority, buffer );
}

void LogClose()
{
   closelog();
}

void  FileLogOpen( const char* processName, bool erasePreviousContents ) 
{
   string directory  = "/usr/local/bin/mber";
   CreateSubDirectory( directory );
   string filename  = directory+"/";
   filename += processName;
   filename += ".log";

   CreateLogFile( filename.c_str(), processName, erasePreviousContents );
}

#elif PLATFORM == PLATFORM_MAC

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void LogOpen()
{
}

void LogMessage(int priority, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   char buffer[ logTempBufferSize ];
   vsprintf(buffer, fmt, args);

   va_end(args);

   cout << buffer << endl;
}

void LogClose()
{
}

void  FileLogOpen( const char* extFilename, bool erasePreviousContents ) // TODO, unimplemented
{
}

#else // WINDOWS


void LogOpen() {}

void LogMessage( int priority, const char *printFormat, ...)
{
   char buffer[ logTempBufferSize ];

   va_list args;
   va_start( args, printFormat );
   vsprintf ( buffer, printFormat, args);
   va_end(args);

   cout << buffer << endl;

   FileLog( priority, buffer );
   //TODO: Hook into the windows event logger.
}

void LogClose() {}

#pragma warning ( disable:4996 )


void  FileLogOpen( const char* processName, bool erasePreviousContents )
{
   string directory  = "c:/temp/mber_logging";
   CreateSubDirectory( directory );
   string filename  = directory+"/";
   filename += processName;
   filename += ".log";

   CreateLogFile( filename.c_str(), processName, erasePreviousContents );
}


#endif   //_WIN32

