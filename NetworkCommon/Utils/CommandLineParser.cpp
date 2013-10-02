// CommandLineParser.cpp

#include "CommandLineParser.h"

#include <iostream>
#include <boost/tokenizer.hpp>
//#include <conio.h>
#include <assert.h>
#include <boost/lexical_cast.hpp>
#include "Utils.h"

struct AcceptanceTest
{
   static bool shouldRemove( char c )
   {
      char testAlpha = c & 223;
      if( testAlpha >= 'A' && testAlpha <= 'Z' ) return false;
      if( c >= '0' && c <= '9' ) return false;
      if( c == '.' ) return false;
      if( c == '!' || c == '$' || c == '@' || c == '*' || c == '+' || c == '-' ) return false;// a few other characters allowed

      return true;
   }
   string operator()( const string& str ) 
   { 
      string modString = str;
      modString.erase( std::remove_if( modString.begin(), modString.end(), AcceptanceTest::shouldRemove ), modString.end() );
      return modString; 
   }
};

//--------------------------------------------------------------------------
//--------------------------------------------------------------------------

CommandLineParser::CommandLineParser( int num, const char* arguments[] )
{
   ProcessCommandLine( num, arguments, m_values );
}

CommandLineParser::CommandLineParser( const vector< std::string >& strings )
{
   bool isFirstOfPair = true;

   vector< std::string >::const_iterator it = strings.begin();
   while( it != strings.end() )
   {
      const string& str = *it++;
      if( isFirstOfPair == true )
      {
         KeyValuePair   keyValuePair;
         keyValuePair.key = ConvertStringToLower( str );
         m_values.push_back( keyValuePair );
      }
      else
      {
         ( *(m_values.rbegin() ) ).value = str;
      }
      isFirstOfPair = !isFirstOfPair;
   }
}

CommandLineParser::CommandLineParser( const string& textToSeparateAndSort, const vector< std::string >& stringDictionary )
{
   assert( 0 );// todo
}

bool  CommandLineParser::FindValue( const string& key, string& valueOut ) const
{
   string searchKey = ConvertStringToLower( key );

   ValuesListConstIterator it = m_values.begin();
   while( it != m_values.end() )
   {
      if( (*it).key == searchKey )
      {
         valueOut = (*it).value;
         return true;
      }
      it++;
   }
   return false;
}

bool  CommandLineParser::FindValue( const string& key, int& valueOut ) const
{
   string searchKey = ConvertStringToLower( key );

   ValuesListConstIterator it = m_values.begin();
   while( it != m_values.end() )
   {
      if( (*it).key == searchKey )
      {
         try
         {
            valueOut = boost::lexical_cast<int>( (*it).value );
         }
         catch( boost::bad_lexical_cast const&  )
         {
            valueOut = 0;
         }
         return true;
      }
      it++;
   }
   return false;
}

bool  CommandLineParser::FindValue( const string& key ) const
{
   string searchKey = ConvertStringToLower( key );

   ValuesListConstIterator it = m_values.begin();
   while( it != m_values.end() )
   {
      if( (*it).key == searchKey )
      {
         return true;
      }
      it++;
   }
   return false;
}

bool  CommandLineParser::IsKeywordFirst( const string& key ) const
{
   string searchKey = ConvertStringToLower( key );
   ValuesListConstIterator it = m_values.begin();
   if( it->key == searchKey )
      return true;
   return false;
}

bool  CommandLineParser::GetKeyValue( int index, string& key, string& value ) const
{
   key.clear();
   value.clear();

   if( index < 0 || index >= static_cast<int>( m_values.size() ) )
      return false;

   const KeyValuePair& kvp = m_values[ index ];
   key = kvp.key;
   value = kvp.value;

   return true;
}

bool  CommandLineParser::GetKeyValue( int index, string& key, int& value ) const
{
   key.clear();
   value = 0;

   if( index < 0 || index >= static_cast<int>( m_values.size() ) )
      return false;

   const KeyValuePair& kvp = m_values[ index ];
   key = kvp.key;
   string temp = kvp.value;

   try 
   {
       value = boost::lexical_cast<int>( kvp.value );
   } 
   catch( boost::bad_lexical_cast const& ) 
   {
       cout << "Error: input string was not valid" << std::endl;
   }

   return true;
}

//--------------------------------------------------------------------------

void  CommandLineParser::ProcessCommandLine( int num, const char* arguments[], ValuesList& values ) const
{
   // because the first few args are for the app itself, we will not parse those
  /* typedef boost::tokenizer< boost::char_separator< char > > tokenizer;
   boost::char_separator< char > separator( ":,=|" );*/

   string separator1("");//dont let quoted arguments escape themselves
   string separator2("=:");//split on = and :
   string separator3("\"\'");//let it have quoted arguments


   boost::escaped_list_separator<char> els( separator1, separator2, separator3 );

   for( int i=1; i< num; i++ )
   {
      string line = arguments[i];
      vector< string > listOfStuff;

      boost::tokenizer<boost::escaped_list_separator<char> > tokens( line, els );
      for( boost::tokenizer<boost::escaped_list_separator<char> >::iterator i(tokens.begin()); i!=tokens.end(); ++i) 
      {
         listOfStuff.push_back(*i);
      }
     /* tokenizer tok( temp, separator );

      transform( tok.begin(), tok.end(), back_inserter( listOfStuff ), AcceptanceTest() );*/

      if( listOfStuff.size() != 0 )
      {
         int firstItem = 0;// in order to help deal with empty items

         KeyValuePair   keyValuePair;
         const string& potentionalKey = listOfStuff[ firstItem ];
         if( listOfStuff.size() == 2 )// simplest case
         {
            keyValuePair.key = ConvertStringToLower( potentionalKey );
            keyValuePair.value = listOfStuff[1];
            values.push_back( keyValuePair );
         }
         else if( listOfStuff.size() == 1 )
         {
            // there are two cases... the last item could have had a space after it as in "ip 192..."
            // or this could be a new key. in either case we need to look at the last item in the values.
            if( values.size() > 0 && 
               ( *(values.rbegin() )).IsComplete() == false )
            {
               ( *(values.rbegin() ) ).value = potentionalKey;
            }
            else
            {
               keyValuePair.key = ConvertStringToLower( potentionalKey );
               values.push_back( keyValuePair );
            }
         }
         else // no support for 0, 3 or more
         {
            cout << "Command line parser detected too many params" << endl;
            assert( 0 );
         }
         
      }
      listOfStuff.clear();
   }
}

//--------------------------------------------------------------------------
