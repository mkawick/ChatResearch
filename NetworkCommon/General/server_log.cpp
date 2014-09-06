#include "server_log.h"

#include <stdarg.h>
#include <stdio.h>
#include <iostream>
#include <fstream>

#if !defined(_WIN32)

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

#else

ofstream fileStream;
void LogOpen() 
{
   string outputFilename  = "c:/temp/";
   outputFilename += PACKAGE_NAME;
   outputFilename += ".log";
/*   myfile.open ("example.txt");
  myfile << "Writing this to a file.\n";
  myfile.close();*/
   fileStream.open( outputFilename );
}

void LogMessage(int priority, const char *fmt, ...)
{
   if( fileStream.isOpen() == false )
      LogOpen();

   //TODO: Hook into the windows event logger.
   va_list args;
   va_start(args, fmt);

   char buffer[256];
   vsprintf(buffer, fmt, args);

   va_end(args);

   cout << buffer << endl;
   fileStream << buffer << endl;
}

void LogClose() 
{
   fileStream.close();
}

#endif   //_WIN32

