#include <stdarg.h>
#include <stdio.h>
#include <iostream>
using namespace std;

void LogMessage(int priority, const char *fmt, ...)
{
   va_list args;
   va_start(args, fmt);

   char buffer[256];
   vsprintf(buffer, fmt, args);

   va_end(args);

   cout << buffer << endl;
}