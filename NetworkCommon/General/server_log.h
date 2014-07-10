#ifndef H_SERVER_LOG_H
#define H_SERVER_LOG_H

#if defined(WIN32)

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

void LogClose();

#endif   //H_SERVER_LOG_H

