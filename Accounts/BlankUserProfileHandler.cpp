
#include <time.h>
#include <iostream>
using namespace std;

#include "AccountServer.h"

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "BlankUserProfileHandler.h"


///////////////////////////////////////////////////////////////////////////////////////////

BlankUserProfileHandler::BlankUserProfileHandler( U32 id, Queryer* parent, string& query ) : ParentType( id, 20, parent ), m_isServicingBlankUUID( false )
{
   m_queryString = query;
}

void     BlankUserProfileHandler::Update( time_t currentTime )
{
   ParentType::Update( currentTime, m_isServicingBlankUUID );
}

//---------------------------------------------------------------

bool     BlankUserProfileHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   U32 queryType = static_cast< U32 >( dbResult->lookup );
   if( queryType == m_queryType )
   {
      SetValueOnExit< bool >           setter( m_isServicingBlankUUID, false );// due to multiple exit points...

      bool addedProfiles = false;
      IndexTableParser              enigma( dbResult->bucket );
      IndexTableParser::iterator    it = enigma.begin();
    /*  if( enigma.m_bucket.size() > 0 )
      {
         cout << " Successful query = " << m_queryString << endl;
      }*/
      while( it != enigma.end() )
      {
         addedProfiles = true;
         IndexTableParser::row      row = *it++;

         string   userId =            row[ TableIndexOnly::Column_index ];

         cout << "User id adding blank profile = " << userId << endl;

         CreateBlankProfile( userId, 0 );
      }

      time( &m_lastTimeStamp );// restart timer
      return true;
   }
   return false;
}

//---------------------------------------------------------------

void     BlankUserProfileHandler::CreateBlankProfile( const string& user_id, int productId )
{
   if( user_id.size() == 0 || user_id == "0" )
   {
      string message = "Accounts::CreateBlankProfile userId is 0\n";
      cout << message << endl;
      return;
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = m_queryType;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "INSERT INTO user_profile (user_id, account_create_product_id) VALUES (";
   dbQuery->query += boost::lexical_cast< string >( user_id );
   dbQuery->query += ",";
   dbQuery->query += boost::lexical_cast< string >( productId );
   dbQuery->query += ")";

   m_parent->AddQueryToOutput( dbQuery );
}


///////////////////////////////////////////////////////////////////////////////////////////
