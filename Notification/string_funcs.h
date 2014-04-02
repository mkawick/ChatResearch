
#ifndef __STRING_FUNCS_H__
#define __STRING_FUNCS_H__

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "core_types.h"

#if defined(WIN32)

#define STRNCASECMP( pString1, pString2, len )  (_strnicmp((pString1),(pString2),(len)))
#define STRCASECMP( pString1, pString2 )  (_stricmp((pString1),(pString2)))
#define STRCPY( pDstString, dstStringSize, pSrcString )  strcpy_s((pDstString),(dstStringSize),(pSrcString))
#define STRNCPY( pDstString, dstStringSize, pSrcString, srcStringSize )  strncpy_s((pDstString),(dstStringSize),(pSrcString),(srcStringSize))
#define STRCAT( pDstString, dstStringSize, pSrcString ) strcat_s((pDstString),(dstStringSize),(pSrcString))
#define STRPRINTF( pDstString, dstStringSize, pFormatString, ... ) sprintf_s((pDstString),(dstStringSize),(pFormatString),__VA_ARGS__)

#else

#define STRNCASECMP( pString1, pString2, len )  (strncasecmp((pString1),(pString2),(len)))
#define STRCASECMP( pString1, pString2 )  (strcasecmp((pString1),(pString2)))
#define STRCPY( pDstString, dstStringSize, pSrcString )  strcpy((pDstString),(pSrcString))
#define STRNCPY( pDstString, dstStringSize, pSrcString, srcStringSize )  strncpy((pDstString),(pSrcString),(dstStringSize)); pDstString[dstStringSize-1] = '\0'
#define STRCAT( pDstString, dstStringSize, pSrcString ) strcat((pDstString),(pSrcString))
#define STRPRINTF( pDstString, dstStringSize, pFormatString, ... ) sprintf(pDstString,pFormatString,__VA_ARGS__)

#endif

char *strcpyUtf8(char *dst, size_t dstSize, const char *src);
char *FormatTime(char *outBuffer, size_t len, unsigned int inSeconds);
char* FormatTimeHourMinOnly(char* outBuffer, size_t len, unsigned int inSeconds);
char *FormatTimeWords(char *outBuffer, size_t len, unsigned int inSeconds, bool abbreviations = false, const char ** pLocalizedTimeUnits = NULL);
char* FormatString(char* buffer, size_t len, const char* fmt, ...);
//const char *devicetoa(const unsigned char *device);

char toupper(char lower);

//Character Arrays are comprised of NULL seperated strings and are terminated with double NULLs
//Each string is prefixed by a 1 byte header consisting of a 5-bit code(indicating localization or not) and a 3-bit index (indicating location in parameter list)
//The first string's index is the number of parameters the string yields
#define Hint_Localize_None (0)
#define Hint_Localize_Key  (1)
#define Hint_Localize_CA   (2)

#define CA_MAX_PARAMS    (8)

#define CA_CODE_MASK   (0x1f)
#define CA_CODE_SHIFT  (0)
#define CA_CODE_BITS   (5)

#define CA_INDEX_MASK    (0xE0)
#define CA_INDEX_SHIFT   (5)
#define CA_INDEX_BITS    (3)

#define CA_CODE_NORMAL     (6)
#define CA_CODE_LOCALIZE   (21)

#define CA_GETCODE(b)  ((b & CA_CODE_MASK) >> CA_CODE_SHIFT)
#define CA_GETINDEX(b) ((b & CA_INDEX_MASK) >> CA_INDEX_SHIFT)

#define CA_IS_LOCALIZE(b)     (CA_GETCODE(b) == CA_CODE_LOCALIZE)
#define CA_GETPARAM_INDEX(b)  CA_GETINDEX(b)
#define CA_GETNUMPARAMS(b)    CA_GETINDEX(b)

#define CA_SETHEADER(p,c) ((p << CA_INDEX_SHIFT) | (c << CA_CODE_SHIFT))



char* CharArrayAddEmptyString(char* caBuf, size_t maxlen);
char* CharArrayAddString(char* pStrStart, int strmaxlen, char* caBuf,  unsigned int code, const char* str);
char* CharArrayAddFormattedString(char* pStrStart, int strmaxlen, char* caBuf, unsigned int code, const char* fmt, ...);
char* CharArrayAddSingleDigit(char* pStrStart, int strmaxlen, char* caBuf, unsigned int digit);
size_t CharArrayGetSize(const char* caBufStart);
size_t CharArrayDecode(const char* caBufStart, const char** paramBuf, size_t maxParams);
char* CharArrayCopy(char* dest, size_t len, const char* src);

#define CABUFFER(s,n)      char s[n];
#define CAGETBUFFER(s)     (&(s[0]))
//#define CASTART { char* paBuf = promptString; paBuf[0] = '\0';
#define CASTART(s,n)       { char* paBuf = s; paBuf[0] = '\0'; char* caBufHead = s; const size_t caBufSize = n;
#define CAEND   }
#define CASTR(s, n)        paBuf = CharArrayAddString(caBufHead, caBufSize, paBuf, n, (s));
#define CAKEY(s)           CASTR(s, CA_CODE_LOCALIZE)
#define CAPARAM(s)         CASTR(s, CA_CODE_NORMAL)
#define CAVSTR(n, s, ...)  paBuf = CharArrayAddFormattedString(caBufHead, caBufSize, paBuf, n, (s), (__VA_ARGS__));
#define CAVKEY(s,...)      CAVSTR(CA_CODE_LOCALIZE, s, (__VA_ARGS__))
#define CAVPARAM(s,...)    CAVSTR(CA_CODE_NORMAL, s, (__VA_ARGS__))
#define CADIGIT(n)         paBuf = CharArrayAddSingleDigit(caBufHead, caBufSize, paBuf, n);
#define CACALCSIZE(s)      CharArrayGetSize(s)
#define CASIZE             ((paBuf - caBufHead) + 1)
#define CACPY(d,len,s)     CharArrayCopy(d, len, s)

enum TimeUnitIndex
{
   TU_SECOND = 0,
   TU_SECONDS = 1,
   TU_ABBR_SECOND = 2,
   TU_MINUTE = 3,
   TU_MINUTES = 4,
   TU_ABBR_MINUTE = 5,
   TU_HOUR = 6,
   TU_HOURS = 7,
   TU_ABBR_HOUR = 8,
   TU_ABBR_HOURS = 9,
   TU_DAY = 10,
   TU_DAYS = 11,
   TU_NUM = 12,
};




static inline uint64_t CreateUserIDHash( const char *pUserID )
{
   uint64_t hash = 5381;
   uint64_t c;

   while( (c = *pUserID++) != 0 )
   {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
   }

   return hash;
}

static inline uint64_t CreatePasswordHash( const char *pPassword )
{
   char salted_password[256];
   STRPRINTF( salted_password, sizeof(salted_password), "pd%-14s", pPassword );

   const char *pPasswordText = salted_password;

   uint64_t hash = 5381;
   uint64_t c;

   while( (c = *pPasswordText++) != 0 )
   {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
   }

   return hash;
}

#endif

