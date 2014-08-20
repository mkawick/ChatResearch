// FixedLengthString.h

#pragma once

#include "../ServerConstants.h"
//#include <boost/static_assert.hpp>

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
   int   length() const;

   bool  empty() const { return size() == 0; }   
   int   maxLength() const { return str_length + 1; }
   int   find( const char* subString ) const; // returns maxLength if this is not found

   bool  operator == ( const char * str ) const ;
   bool  operator == ( const FixedLengthString& rhs ) const;
   const FixedLengthString&		operator = ( const char * str );

   const char* c_str() const { return buffer; }
   operator	const char* () const { return buffer; }
   operator	const U8* () const { return reinterpret_cast<const U8*>( buffer ); }

   operator	const string () const { return string( buffer ); }

   //-------------------------------------
   
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   

protected:
   void  copy( const char* str );
   bool  isEqual( const char* str, bool compareCaseless ) const;
   
   char buffer[ str_length + 1 ];// null terminated
};

typedef FixedLengthString< 140 > FixedString140;
typedef FixedLengthString< 128 > FixedString128;
typedef FixedLengthString<  80 > FixedString80;
typedef FixedLengthString<  64 > FixedString64;
typedef FixedLengthString<  32 > FixedString32;
typedef FixedLengthString<  16 > FixedString16;
typedef FixedLengthString<   8 > FixedStringTiny;
typedef FixedString16            UuidString;
typedef FixedString32            TimeString;

////////////////////////////////////////////////////////

template < int str_length = 16 >
class BoundedLengthString
{
   //enum { MAX_LENGTH_BOUNDED_STRING = 16384 };
   //BOOST_STATIC_ASSERT_MSG( str_length < MAX_LENGTH_BOUNDED_STRING, "string length specified is too long");
public:
   BoundedLengthString() : m_length( 0 ) { buffer[0] = 0; }
   BoundedLengthString( const BoundedLengthString<str_length>& rhs ) { copy( rhs.buffer ); }
   BoundedLengthString( const char* str ) { copy( str ); }
   BoundedLengthString( const string& str ) { copy( str.c_str() ); }
   
   void  clear() { buffer[0] = 0; m_length = 0; }
   int   size() const;
   int   length() const;

   bool  empty() const { return size() == 0; }   
   int   maxLength() const { return str_length + 1; }
   int   find( const char* subString ) const; // returns maxLength if this is not found

   bool  operator == ( const char * str ) const ;
   bool  operator == ( const BoundedLengthString& rhs ) const;
   const BoundedLengthString&		operator = ( const char * str );

   const char* c_str() const { return buffer; }
   operator	const char* () const { return buffer; }
   operator	const U8* () const { return reinterpret_cast<const U8*>( buffer ); }

   operator	const string () const { return string( buffer ); }

   //-------------------------------------
   
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
   

protected:
   void  copy( const char* str );
   bool  isEqual( const char* str, bool compareCaseless ) const;
   
   char  buffer[ str_length + 1 ];// null terminated
   int   m_length;
};

typedef BoundedLengthString< 140 > BoundedString140;
typedef BoundedLengthString< 128 > BoundedString128;
typedef BoundedLengthString<  80 > BoundedString80;
typedef BoundedLengthString<  64 > BoundedString64;
typedef BoundedLengthString<  32 > BoundedString32;
typedef BoundedLengthString<  16 > BoundedString16;
typedef BoundedLengthString<   8 > BoundedStringTiny;

////////////////////////////////////////////////////////

#include "FixedLengthString.inl"