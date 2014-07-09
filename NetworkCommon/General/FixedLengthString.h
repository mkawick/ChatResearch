// FixedLengthString.h

#pragma once

#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#define strcasecmp stricmp 
#else
#include <iostream>
#endif

////////////////////////////////////////////////////////

template < int str_length = 16 >
class FixedLengthString
{
public:
   FixedLengthString() { buffer[0] = 0; }
   FixedLengthString( const FixedLengthString<str_length>& rhs ) { copy( rhs.buffer ); }
   FixedLengthString( const char* str ) { copy( str ); }
   FixedLengthString( const string& str ) { copy( str.c_str() ); }
   
   void  clear() { buffer[0] = 0; }
   int   size() const;
   bool  empty() const { return size() == 0; }
   int   length() const;
   int   maxLength() const { return str_length + 1; }

   bool  operator == ( const char * str ) const ;
   bool  operator == ( const FixedLengthString& rhs ) const;
   const FixedLengthString&		operator = ( const char * str );

   const char* c_str() const { return buffer; }
   operator	const char* () const { return buffer; }
   operator	const U8* () const { return reinterpret_cast<const U8*>( buffer ); }

   //operator	string () { return string( buffer ); }
   operator	const string () const { return string( buffer ); }

   //-------------------------------------
   
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
   

protected:
   void  copy( const char* str );
   bool  isEqual( const char* str, bool compareCaseless ) const;
   
   char buffer[ str_length + 1 ];// null terminated
};

typedef FixedLengthString< 128 > FixedString128;
typedef FixedLengthString<  80 > FixedString80;
typedef FixedLengthString<  60 > FixedString60;
typedef FixedLengthString<  32 > FixedString32;
typedef FixedLengthString<  16 > FixedString16;
typedef FixedLengthString<  16 > UuidString;

////////////////////////////////////////////////////////

#include "FixedLengthString.inl"