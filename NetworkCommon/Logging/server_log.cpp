#include "server_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

ofstream  loggingFile;

#if PLATFORM != PLATFORM_WINDOWS

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void LogOpen()
{
   openlog(PACKAGE_NAME, LOG_CONS, LOG_USER);
}

void LogMessage(int priority, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   vsyslog(priority, fmt, args);

   va_end(args);
}

void LogClose()
{
   closelog();
}

void  FileLogOpen() // TODO, unimplemented
{
}
void  FileLog( const char* text )
{
}
void  FileLogClose()
{
}

#else

void LogOpen() {}

void LogMessage(int priority, const char *fmt, ...)
{
   //TODO: Hook into the windows event logger.
   va_list args;
   va_start(args, fmt);

   vprintf(fmt, args);

   va_end(args);
}

void LogClose() {}

bool  isCreatingFile = false;
void  FileLogOpen( const char* extFilename )
{
   if( isCreatingFile == true )// simple atomic-like operation
      return;
   isCreatingFile = true;

   string filename =  "c:/temp/logFile.log";
   if( extFilename != NULL )
   {
     filename = extFilename;
     filename += ".log";
   }
   remove( filename.c_str() );
   loggingFile.open ( filename.c_str(), ios::out | ios::app | ios::binary);//ios::out | ios::app | ios::trunc | ios::binary );

   if( loggingFile.is_open() == false )
   {
      cout << "Alert: Unable to open log file " << endl;
   }
   isCreatingFile = false;
}

void  FileLog( const char* text )
{
   if( loggingFile.is_open() == false )
   {
      FileLogOpen();
   }
   if( text == NULL )
      return;

   string printable( text );
   printable += "\r\n";
   loggingFile.write( printable.c_str(), printable.length() );
   loggingFile.flush();
}

void  FileLogClose()
{
   loggingFile.close();
}

#endif   //_WIN32

