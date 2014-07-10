#include "server_log.h"

#include <stdarg.h>
#include <stdio.h>

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

#endif   //_WIN32

