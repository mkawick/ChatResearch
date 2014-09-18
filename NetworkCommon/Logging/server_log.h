#pragma once

#include "../Platform.h"
#include <string>

#if PLATFORM == PLATFORM_WINDOWS

#define LOG_PRIO_ERR    0
#define LOG_PRIO_WARN   1
#define LOG_PRIO_INFO   2
#define LOG_PRIO_DEBUG  3

#else

#include <syslog.h>

#define LOG_PRIO_ERR    LOG_ERR
#define LOG_PRIO_WARN   LOG_WARNING
#define LOG_PRIO_INFO   LOG_INFO
#define LOG_PRIO_DEBUG  LOG_DEBUG

#endif



void LogOpen();

// Writes a message to the server log. priority is one of the LOG_PRIO constants above.
void LogMessage(int priority, const char *fmt, ...);
void LogMessage(int priority, const std::string& str );

void LogClose();


void  FileLogOpen( const char* filename = 0L, bool erasePreviousContents = false );
void  FileLog( int priority, const char* text );
void  FileLogClose();