#include "StringUtils.h"

#include <stdio.h>
#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include <sstream>
#include <iostream>
#include <algorithm>

#define BOOST_NO_CXX11_EXPLICIT_CONVERSION_OPERATORS  // workaround for compile error on linux
#include <boost/functional/hash.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>
#include <boost/tokenizer.hpp>
#include <boost/range/algorithm/remove_if.hpp>
#include <boost/random.hpp>
#include <boost/generator_iterator.hpp>



using namespace std;
static boost::random::mt19937 rng;

//////////////////////////////////////////////////////////////////////////

string   GenerateUUID( U32 xorValue )
{
   string returnString;

   U32 max = INT_MAX;
   //boost::random::mt19937 rng;         // produces randomness out of thin air
                                    // see pseudo-random number generators
   boost::random::uniform_int_distribution<> roll(0, max);
                                       // distribution that maps to 1..6
                                       // see random number distributions
   //int x = six(rng);                   // simulate rolling a die

   for( int i=0; i< 2; i++ )
   {
      //U32 value = ( (rand() % 256) << 24 ) + ( (rand() % 256) << 16 ) + ( (rand() % 256) << 8 ) + (rand() % 256);
      U32 value = roll( rng );
      value ^= xorValue;// xor with the elapsed time.
      stringstream stream;
      stream << hex << value;
      returnString += stream.str();
   }

   while( returnString.size() < 16 )// fill in with 0's to be sure that we have the proper length
   {
      returnString += '0';
   }
   return returnString;
}

//////////////////////////////////////////////////////////////////////////

U64      GenerateUniqueHash( const std::string& str, int maxHexLen )
{
   size_t seed = 1027; // a good starting point
   std::string randomtext("playdek.games"); // add an unpredictable starting point

   boost::hash_combine( seed, str );
   U64 temp = seed;
   boost::hash_combine( seed, randomtext );
   temp = ( temp << 32 ) + seed;

   if( maxHexLen < 16 )
   {
      U64 mask = -1;
      int numToStrip = ( 16 - maxHexLen ) * 4;// 4 bits per hex char
      mask <<= numToStrip; // push off the most significant which are the most likely to be similar.
      mask >>= numToStrip;
      temp &= mask;
   }

   return temp;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

const string itos(int n)
{
   const int max_size = std::numeric_limits<int>::digits10 + 1 /*sign*/ + 1 /*0-terminator*/;
   char buffer[max_size] = {0};
   sprintf(buffer, "%d", n);
   return string(buffer);
}

//////////////////////////////////////////////////////////////////////////

const string  ConvertStringToLower( const string& str )
{
   string retString = str;
   std::transform( retString.begin(), retString.end(), retString.begin(), ::tolower );
   return retString;
}

//////////////////////////////////////////////////////////////////////////

std::string    ToHexString( U32 value )
{
   std::stringstream stream;
   if( value == 0 )
      stream << "..";
   else if( value < 16 )
   {
      stream << "." << std::hex << value;
   }
   else
   {
      stream << std::hex << value;
   }
   return stream.str();
}

//////////////////////////////////////////////////////////////////////////

string BufferToHexString( const U8* buffer, int numBytes )
{
   string str;
   str.reserve( 3*numBytes + 2 );
   for( int j=0; j< numBytes; j++ )
   {
      str += ToHexString( *buffer ) + " ";
      buffer ++;
   }
   return str;
}

//////////////////////////////////////////////////////////////////////////

const char*    ConvertToTrueFalseString( bool value ) 
{ 
   if( value == true ) 
      return "true"; 
   return "false"; 
}

//////////////////////////////////////////////////////////////////////////

std::string    Trim( const std::string& str,
                 const std::string& whitespace )
{
   const string::size_type begin = str.find_first_not_of( whitespace );
   const string::size_type end   = str.find_last_not_of( whitespace );

   return str.substr( begin, end-begin + 1 );
}

//////////////////////////////////////////////////////////////////////////

std::string    Reduce( const std::string& str,
                      const std::string& fill,
                      const std::string& whitespace )
{
    // trim first
    string result = Trim( str, whitespace );

    // replace sub ranges
    size_t beginSpace = result.find_first_of( whitespace );
    while ( beginSpace != std::string::npos )
    {
        const string::size_type endSpace = result.find_first_not_of( whitespace, beginSpace );
        const string::size_type range = endSpace - beginSpace;

        result.replace( beginSpace, range, fill );

        const string::size_type newStart = beginSpace + fill.length();
        beginSpace = result.find_first_of(whitespace, newStart);
    }

    return result;
}

//////////////////////////////////////////////////////////////////////////

unsigned int split( const std::string &txt, std::vector<std::string> &strs, char ch )
{
   size_t pos = txt.find( ch ) ;
   size_t initialPos = 0;
   strs.clear();

   // Decompose statement
   while( pos != std::string::npos ) 
   {
      string temp = Trim( txt.substr( initialPos, pos - initialPos + 1 ) );
      strs.push_back( temp );
      initialPos = pos + 1;

      pos = txt.find( ch, initialPos ) ;
   }

   // Add the last one
   strs.push_back( txt.substr( initialPos, min( pos, txt.size() ) - initialPos + 1 ) );

   return static_cast< unsigned int > ( strs.size() );
}

//////////////////////////////////////////////////////////////////////////

void  PrintDebugText( const char* text, int extraCr )
{
#if defined (VERBOSE)

   cout << text << endl;
   while( --extraCr >= 0 )
   {
      cout << endl;
   }
#endif
}

//////////////////////////////////////////////////////////////////////////

bool  ParseListOfItems( vector< string >& listOfStuff, string text, const char* delimiter, const char* charsToRemove )
{
   //text.erase( boost::remove_if( text.begin(), text.end(), "[]{}"), text.end() );
   if( charsToRemove )
   {
      text.erase( boost::remove_if( text, boost::is_any_of( charsToRemove )), text.end() );
   }

   string separator1( "" );//dont let quoted arguments escape themselves
   string separator2( delimiter );//split on = and :
   string separator3( "\"\'" );//let it have quoted arguments


   boost::escaped_list_separator<char> els( separator1, separator2, separator3 );
   boost::tokenizer<boost::escaped_list_separator<char> > tokens( text, els );

   for (boost::tokenizer<boost::escaped_list_separator<char> >::iterator i(tokens.begin());
      i!=tokens.end(); ++i) 
   {
      listOfStuff.push_back(*i);
   }

   if( listOfStuff.size() > 0 )
      return true;

   return false;
}

//////////////////////////////////////////////////////////////////////////

string RemoveEnds( std::string s, const char* charsToStrip )
{
   if( s.size() == 0 )
      return s;

   int len = static_cast< int > ( strlen( charsToStrip ) );

   for( int i=0; i<len; i++ )
   {
      while( s.size() && *s.begin() == charsToStrip[i] )
      {
         s = s.substr(1, s.size());
      }
      while( s.size() && *s.rbegin() == charsToStrip[i] )
      {
         s = s.substr(0, s.size()-1);
      }
      //s.erase(remove( s.begin(), s.end(), charsToStrip[i] ),s.end());
   }

   return s;
}

//////////////////////////////////////////////////////////////////////////

//#define PRINT_BUFFER_INFO

void  DumpBuffer( U8* buffer, int offset, int length )
{
   if( buffer == NULL || length == 0 )
      return;

#ifdef PRINT_BUFFER_INFO
      cout << "offset: " << offset << "   length: " << length << endl;
      int bytesToPrint = length; if( bytesToPrint > 16 ) bytesToPrint = 16;
      string str = BufferToHexString( buffer, bytesToPrint );
      cout << " ->[ " << str << " ]" << endl;
#endif
}

//////////////////////////////////////////////////////////////////////////