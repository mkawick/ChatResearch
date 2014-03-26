// StringLookup.h

#pragma once

#include "../Packets/BasePacket.h"
#include "QueryHandler.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

class DiplodocusPurchase;

///////////////////////////////////////////////////////////////////////////////////////////


enum 
{
   StringLookupQuery = 5000
};

class StringLookup : public QueryHandler< Queryer* >
{
public: 
   typedef QueryHandler< Queryer* > ParentType;
public:
   StringLookup( U32 id, ParentQueryerPtr parent, vector< string >& stringCategories );

   void     Update( time_t currentTime );
   bool     HandleResult( const PacketDbQueryResult* dbResult );
   bool     IsReady() const { return m_hasLoadedStringTable; }
   
   string   GetString( const string& stringName, int languageId ) const;

   //void  Init();

protected:

   void     SubmitQuery();
   void     SaveStrings( const PacketDbQueryResult* dbResult );
   
   

   typedef map< stringhash, vector< string > >     StringTableLookup;
   typedef pair< stringhash, vector< string > >    StringTableLookupPair;
   typedef pair< stringhash, stringhash >          ReplacementPair;

   bool                             m_isLoadingAllStrings;
   bool                             m_hasLoadedStringTable;
   int                              m_numQueriesReceived;
   StringTableLookup                m_stringsTable;
   map< stringhash, stringhash >    m_replacemetStringsLookup;
   vector< string >                 m_stringCategories;
};

///////////////////////////////////////////////////////////////////////////////////////////
