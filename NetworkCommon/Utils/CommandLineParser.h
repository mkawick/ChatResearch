// CommandLineParser.h

#pragma once

#include <vector>
#include <string>
#include "../Platform.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning (disable:4996)
#endif

using namespace std;

//////////////////////////////////////////////////////////////////

// accepts formats of 
// -ip=192.168.0.1
// ip=192.168.0.1
// ip:192.168.1.1
// ip 192.168.0.1


//////////////////////////////////////////////////////////////////

class CommandLineParser
{
public:
   CommandLineParser( int num, const char* arguments[] );
   CommandLineParser( const vector< std::string >& strings );// simple pairing
   CommandLineParser( const string& textToSeparateAndSort, const vector< std::string >& stringDictionary );

   bool  FindValue( const string& key, string& valueOut ) const;
   bool  FindValue( const string& key, int& valueOut ) const;
   bool  FindValue( const string& key ) const;
   bool  FindValue( const string& key, vector< std::string >& strings ) const;
   bool  SeparateStringIntoKeyValue( const string& inString, string& key, string& value ) const;
   bool  SeparateStringIntoValues( const string& inString, vector< std::string >& listOfStuff, int expectedNum ) const;

   bool  IsKeywordFirst( const string& key ) const;

   int   GetNumKeyValuePairs() const { return static_cast<int> ( m_values.size() ); }
   bool  GetKeyValue( int index, string& key, string& value ) const;
   bool  GetKeyValue( int index, string& key, int& value ) const;

protected:
   struct KeyValuePair
   {
      string key;
      string value;
      bool  IsComplete() { return static_cast< bool >( key.size()>0 && value.size()>0 ); } // if either is 0
   };

   typedef vector< KeyValuePair >         ValuesList;
   typedef ValuesList::iterator           ValuesListIterator;
   typedef ValuesList::const_iterator     ValuesListConstIterator;
   ValuesList                             m_values;

   void  ProcessCommandLine( int num, const char* arguments[], ValuesList& values ) const;
   void  ConvertArrayIntoElements( const string& array, vector< std::string >& strings ) const;
};

//////////////////////////////////////////////////////////////////
