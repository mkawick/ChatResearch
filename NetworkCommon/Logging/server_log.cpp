#include "server_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;

#include <assert.h>

ofstream  loggingFile;

#if PLATFORM == PLATFORM_LINUX

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

void  FileLogOpen( const char* extFilename ) // TODO, unimplemented
{
}
void  FileLog( const char* text )
{
}
void  FileLogClose()
{
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

   char buffer[256];
   vsprintf(buffer, fmt, args);

   va_end(args);

   cout << buffer << endl;
}

void LogClose()
{
}

void  FileLogOpen( const char* extFilename ) // TODO, unimplemented
{
}
void  FileLog( const char* text )
{
}
void  FileLogClose()
{
}
#else

#pragma warning ( disable:4996 )

void LogOpen() {}

void LogMessage(int priority, const char *format, ...)
{
   va_list args;
   va_start( args, format );

   char buffer[256];
   vsprintf ( buffer, format, args);

   va_end(args);
   cout << buffer << endl;

   if( loggingFile.is_open() == false )
      return;
   //TODO: Hook into the windows event logger.
   loggingFile << buffer << endl;
   loggingFile.flush();
}

void LogClose() {}

bool  isCreatingFile = false;
void  FileLogOpen( const char* extFilename )
{
   if( isCreatingFile == true )// simple atomic-like operation
      return;
   isCreatingFile = true;
   assert( extFilename );
   if( loggingFile.is_open() )
      loggingFile.close();

   string filename  = "c:/temp/";
   filename += extFilename;
   filename += ".log";

   remove( filename.c_str() );
   loggingFile.open ( filename.c_str(), ofstream::out | ofstream::app );//, ios::out | ios::app | ios::binary);//ios::out | ios::app | ios::trunc | ios::binary );

   if( loggingFile.is_open() == false )
   {
      cout << "Alert: Unable to open log file " << endl;
   }
   if(loggingFile.fail())
   { 
      cout << "Alert: Could not write the file" << flush; 
   }
   isCreatingFile = false;
}

void  FileLog( const char* text )
{
}

void  FileLogClose()
{
   loggingFile.close();
}

#endif   //_WIN32

