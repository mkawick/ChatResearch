#include "string_funcs.h"
#include <stdarg.h>

#if defined(WIN32)
#pragma warning ( disable: 4996 )
#endif

inline float min(float a, float b)
{
    return (((a)<(b))?(a):(b));
}

inline float max(float a, float b)
{
    return (((a)>(b))?(a):(b));
}

char toupper(char lower)
{
    if (lower >= 'a' && lower <= 'z')
        lower += ('A' - 'a');

    return lower;
}

char *strcpyUtf8(char *dst, size_t dstSize, const char *src)
{
   //quick check for invalid utf8 string
   if ((*src & 0xc0) == 0x80)
   {
      *dst = '\0';
      return dst;
   }
   
   char *d = dst;
   const char *s = src;
   
   for (--dstSize; dstSize && *s; --dstSize)
   {
      *d++ = *s++;
   }
   
   // check to see if we truncated a multibyte character
   while ((*s-- & 0xc0) == 0x80) --d;
   
   *d = '\0';
   
   return dst;
}


char* FormatTime(char* outBuffer, size_t len, unsigned int inSeconds)
{   
   int seconds = (inSeconds % 60);
   int minutes = ((inSeconds - seconds) / 60) % 60;
   int hours = (inSeconds - (minutes * 60) - (seconds)) / (60 * 60);
   
   STRPRINTF(outBuffer, len, "%02d:%02d:%02d", hours, minutes, seconds);

   return outBuffer;
}

char* FormatTimeHourMinOnly(char* outBuffer, size_t len, unsigned int inSeconds)
{
   int seconds = (inSeconds % 60);
   int minutes = ((inSeconds - seconds) / 60) % 60;
   int hours = (inSeconds - (minutes * 60) - (seconds)) / (60 * 60);
   
   STRPRINTF(outBuffer, len, "%02d:%02d", hours, minutes);
   
   return outBuffer;
}

static const char* s_DefaultTimeUintStr[TU_NUM]={
   "second",
   "seconds",
   "SEC",
   "minute",
   "minutes",
   "MIN",
   "HOUR",
   "HOURS",
   "HR",
   "HRS",
   "DAY",
   "DAYS",
};

char* FormatTimeWords(char* outBuffer, size_t len, unsigned int inSeconds, bool abbreviate, const char ** pLocalizedTimeUnits)
{
   if(pLocalizedTimeUnits == NULL) pLocalizedTimeUnits = s_DefaultTimeUintStr;
   
   if(inSeconds >= (24*60*60))
   {
      int days = inSeconds / (24*60*60);
      if(days == 1)
      {
         STRPRINTF(outBuffer, len, "1 %s", pLocalizedTimeUnits[TU_DAY]);
      }
      else
      {
         STRPRINTF(outBuffer, len, "%d %s", days, pLocalizedTimeUnits[TU_DAYS]);
      }
   }
   else if(inSeconds >= (60*60))
   {
      int hours = inSeconds / (60*60);
      if(hours == 1)
      {
         const char* strTU = (abbreviate)?pLocalizedTimeUnits[TU_ABBR_HOUR]:pLocalizedTimeUnits[TU_HOUR];
         STRPRINTF(outBuffer, len, "1 %s", strTU);
      }
      else
      {
         const char* strTU = (abbreviate)?pLocalizedTimeUnits[TU_ABBR_HOURS]:pLocalizedTimeUnits[TU_HOURS];
         STRPRINTF(outBuffer, len, "%d %s", hours, strTU);
      }
   }
   else if(inSeconds >= 60)
   {
      int minutes = inSeconds / (60);
      if(minutes == 1)
      {
         const char* strTU = (abbreviate)?pLocalizedTimeUnits[TU_ABBR_MINUTE]:pLocalizedTimeUnits[TU_MINUTE];
         STRPRINTF(outBuffer, len, "1 %s", strTU);
      }
      else
      {
         const char* strTU = (abbreviate)?pLocalizedTimeUnits[TU_ABBR_MINUTE]:pLocalizedTimeUnits[TU_MINUTES];
         STRPRINTF(outBuffer, len,  "%d %s", minutes, strTU);
      }
   }
   else
   {
      if(inSeconds == 1)
      {
         const char* strTU = (abbreviate)?pLocalizedTimeUnits[TU_ABBR_SECOND]:pLocalizedTimeUnits[TU_SECOND];
         STRPRINTF(outBuffer, len, "1 %s", strTU);
      }
      else
      {
         const char* strTU = (abbreviate)?pLocalizedTimeUnits[TU_ABBR_SECOND]:pLocalizedTimeUnits[TU_SECONDS];
         STRPRINTF(outBuffer, len, "%d %s", inSeconds, strTU);
      }
      
   }
   
   //STRPRINTF(outBuffer, len, "%02d:%02d:%02d", hours, minutes, seconds);
   
   return outBuffer;
}

char* FormatString(char* buffer, size_t len, const char* fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   vsnprintf(buffer, len, fmt, args);
   va_end(args);
   
   return buffer;
}

//
//Added empty string to end of buf, returns end of empty string
char* CharArrayAddEmptyString(char* caBuf, size_t maxlen)
{
   const size_t len = strlen(caBuf);
   
   caBuf[len + 1] = '\0';
   
   return &caBuf[len + 1];
}

char* CharArrayAddString(char* pHead, int maxbufsize, char* caBuf, const unsigned int code, const char* str)
{
   const int usedLen = caBuf - pHead;
   const int unusedLen = maxbufsize - usedLen;

   if(pHead == caBuf)
   {
      pHead[0] = CA_SETHEADER(0, code);
   }
   else
   {
      const unsigned int primeHeader = (unsigned int)pHead[0];
      unsigned int numParams = CA_GETNUMPARAMS(primeHeader);
      const unsigned int primeCode = CA_GETCODE(primeHeader);

      caBuf[0] = CA_SETHEADER(numParams, code);
      
      numParams++;
      pHead[0] = CA_SETHEADER(numParams, primeCode);
   }

   STRNCPY((caBuf + 1), (unusedLen - 1), str, (unusedLen - 1));
   
   return CharArrayAddEmptyString(caBuf + 1, unusedLen - 1);
}

char* CharArrayAddFormattedStringNoHeader(char* caBuf, size_t len, const char* fmt, ...)
{
   va_list args;
   va_start(args, fmt);
   vsnprintf(caBuf, len, fmt, args);
   va_end(args);
   
   return CharArrayAddEmptyString(caBuf, len);
}

char* CharArrayAddFormattedString(char* pHead, int maxbufsize, char* caBuf, const unsigned int code,  const char* fmt, ...)
{
   const int usedLen = caBuf - pHead;
   const int unusedLen = maxbufsize - usedLen;

   if(pHead == caBuf)
   {
      caBuf[0] = CA_SETHEADER(0, code);
   }
   else
   {
      const unsigned int primeHeader = (unsigned int)pHead[0];
      unsigned int numParams = CA_GETNUMPARAMS(primeHeader);
      const unsigned int primeCode = CA_GETCODE(primeHeader);

      caBuf[0] = CA_SETHEADER(numParams, code);
      
      numParams++;
      pHead[0] = CA_SETHEADER(numParams, primeCode);
   }
   
   va_list args;
   va_start(args, fmt);
   vsnprintf(caBuf + 1, unusedLen - 1, fmt, args);
   va_end(args);

   return CharArrayAddEmptyString(caBuf + 1, unusedLen - 1);
}

char* CharArrayAddSingleDigit(char* pHead, int maxbufsize, char* caBuf, unsigned int digit)
{
   const int usedLen = caBuf - pHead;
   const int unusedLen = maxbufsize - usedLen;
   
   if(pHead == caBuf)
   {
      caBuf[0] = CA_SETHEADER(0, CA_CODE_NORMAL);
   }
   else
   {
      const unsigned int primeHeader = (unsigned int)pHead[0];
      unsigned int numParams = CA_GETNUMPARAMS(primeHeader);
      const unsigned int primeCode = CA_GETCODE(primeHeader);
      
      caBuf[0] = CA_SETHEADER(numParams, CA_CODE_NORMAL);
      
      numParams++;
      pHead[0] = CA_SETHEADER(numParams, primeCode);
   }
   
   char* pStr = caBuf + 1;
   *pStr++ = '0' + digit;
   *pStr++ = '\0';
   
   return CharArrayAddEmptyString(caBuf + 1, unusedLen - 1);
}

size_t CharArrayGetSize(const char* caBufStart)
{
	size_t caSize = 0;
   
	const char* paBuf = caBufStart;
	while(*paBuf)
	{
		size_t w = strlen(paBuf);
		caSize += w + 1;
		paBuf = &paBuf[w + 1];
	}
	caSize++;
   
   return caSize;
}
//Store pointers to the start of each string in the character array
//Returns the number of strings found
size_t CharArrayDecode(const char* caBufStart, const char** paramBuf, size_t maxParams)
{
   size_t numParams = 0;
   
   if((caBufStart == NULL) || (*caBufStart == '\0'))return 0;
   
   const char* paBuf = caBufStart;
	while(*paBuf)
	{
		paramBuf[numParams] = paBuf;
		numParams++;
		size_t w = strlen(paBuf);
		paBuf = &paBuf[w + 1];
	}
   
   return numParams;
}

char* CharArrayCopy(char* dest, size_t destmaxlen, const char* src)
{
   const size_t srclen = CharArrayGetSize(src);
   
   memcpy(dest, src, (destmaxlen < srclen)?destmaxlen:srclen );
   
   return dest;
}
