// StringLookup.cpp

#include <iostream>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "StringLookup.h"
#include "../Utils/Utils.h"

#include "../Utils/TableWrapper.h"

//---------------------------------------------------------------

StringLookup::StringLookup( U32 id, ParentQueryerPtr parent, vector< string >& stringCategories ): ParentType( id, 20, parent ), m_isLoadingAllStrings( false ), m_numQueriesReceived( 0 )
{
   assert( stringCategories.size() > 0 );
   m_stringCategories = stringCategories;// straight copy
   OnlyRunOneTime();
}

//---------------------------------------------------------------

void     StringLookup::Update( time_t currentTime )
{

   // request strings first for all of the sales.
   ParentType::Update( currentTime, m_isLoadingAllStrings );
}

//---------------------------------------------------------------

void     StringLookup::SubmitQuery()
{
   vector< string >::iterator it = m_stringCategories.begin();
   while( it != m_stringCategories.end() )
   {
      string query = "SELECT * FROM string WHERE category='";
      query += *it++;
      query += "'";


      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = m_queryType;

      dbQuery->query = query;

      m_parent->AddQueryToOutput( dbQuery );
   }
}

//---------------------------------------------------------------

bool     StringLookup::HandleResult( const PacketDbQueryResult* dbResult )
{
   U32 queryType = static_cast< U32 >( dbResult->lookup );
   if( queryType != m_queryType )
      return false;

   if( queryType == m_queryType )
   {
      SaveStrings( dbResult );
      return true;
   }

   return false;
}

//---------------------------------------------------------------

void     StringLookup::SaveStrings( const PacketDbQueryResult* dbResult )
{
   cout << "StringLookup::strings saved :" << dbResult->bucket.bucket.size() << endl;

   StringTableParser              enigma( dbResult->bucket );
   StringTableParser::iterator    it = enigma.begin();
   int count = 0;   
   m_numQueriesReceived ++;
   
   while( it != enigma.end() )
   {
      StringTableParser::row     row = *it++;

      string id =                row[ StringsTable::Column_id ];
      string stringName =        row[ StringsTable::Column_string ];
      string replacementstring = row[ StringsTable::Column_replaces ];

      stringhash lookupHash = GenerateUniqueHash( stringName );
      m_stringsTable.insert( StringTableLookupPair( lookupHash, row ) );
      count ++;

      if( replacementstring.size() )
      {
         stringhash replacementHash = GenerateUniqueHash( replacementstring );
         m_replacemetStringsLookup.insert( ReplacementPair( replacementHash, lookupHash ) );
      }
   }

   if( m_numQueriesReceived >= (int) m_stringCategories.size () )
   {
      m_hasLoadedStringTable = true;
      m_isLoadingAllStrings = false;
   }
}

//---------------------------------------------------------------

string   StringLookup::GetString( const string& stringName, int languageId ) const
{
   stringhash lookupHash = GenerateUniqueHash( stringName );
   StringTableLookup::const_iterator it = m_stringsTable.find( lookupHash );

   if( it != m_stringsTable.end() )
   {
      string defaultText = it->second[ StringsTable::Column_english ];

      string tempString = "";
      switch( languageId )
      {
      case LanguageList_spanish:
         tempString = it->second[ StringsTable::Column_spanish ];
         break;
      case LanguageList_french:
         tempString = it->second[ StringsTable::Column_french ];
         break;
      case LanguageList_german:
         tempString = it->second[ StringsTable::Column_german ];
         break;
      case LanguageList_italian:
         tempString = it->second[ StringsTable::Column_italian ];
         break;
      case LanguageList_portuguese:
         tempString = it->second[ StringsTable::Column_german ];
         break;
      case LanguageList_russian:
         tempString = it->second[ StringsTable::Column_russian ];
         break;
      case LanguageList_japanese:
         tempString = it->second[ StringsTable::Column_japanese ];
         break;
      case LanguageList_chinese:
         tempString = it->second[ StringsTable::Column_chinese ];
         break;
      }

      if( tempString != "" && tempString != "NULL" )
         return tempString;
      return defaultText;
   }
   else
   {
      return string();
   }
}

//---------------------------------------------------------------
