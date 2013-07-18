
#include <time.h>
#include <iostream>
using namespace std;

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "BlankUUIDQueryHandler.h"

///////////////////////////////////////////////////////////////////////////////////////////

BlankUUIDQueryHandler::BlankUUIDQueryHandler( U32 id, Queryer* parent, string& query ) : QueryHandler( id, 20, parent ), m_isServicingBlankUUID( false )
{
   m_queryString = query;
}

void     BlankUUIDQueryHandler::Update( time_t currentTime )
{
   QueryHandler::Update( currentTime, m_isServicingBlankUUID );
}

//---------------------------------------------------------------

bool     BlankUUIDQueryHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->lookup == m_queryType )
   {
      SetValueOnExit< bool >           setter( m_isServicingBlankUUID, false );// due to multiple exit points...

      bool addedUuids = false;
      IndexTableParser              enigma( dbResult->bucket );
      IndexTableParser::iterator    it = enigma.begin();
      while( it != enigma.end() )
      {
         addedUuids = true;
         IndexTableParser::row      row = *it++;

         string userId =            row[ TableIndexOnly::Column_index ];

         UpdateUuidForUser( userId, true, "0" );
      }

      if( addedUuids )
      {
         //string message = "Accounts::HandleBlankUUIDs some UUIDs were added\n";
         //LogMessage( LOG_PRIO_ERR, message.c_str() );
         //cout << message << endl;
      }

      time( &m_lastTimeStamp );// restart timer
      return true;
   }
   return false;
}

//---------------------------------------------------------------

void     BlankUUIDQueryHandler::UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId )
{
   if( ( userId.size() == 0 || userId == "0" ) && ( columnId.size() == 0 || columnId == "0" ) )
   {
      string message = "Accounts::UpdateUuidForUser userId is null\n";
      //LogMessage( LOG_PRIO_ERR, message.c_str() );
      cout << message << endl;
      return;
   }

   string newUuid = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( userId + columnId ) ) );

   if( userId.size() != 0 && userId != "0" ) 
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = m_queryType;
      dbQuery->isFireAndForget = true;

      dbQuery->query = "UPDATE users SET uuid='";
      dbQuery->query += newUuid;
      dbQuery->query += "' where user_id='";
      dbQuery->query += userId;
      dbQuery->query += "';";

      m_parent->AddQueryToOutput( dbQuery );

      if( updateCreateAccountTableToo )
      {
         dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = m_queryType;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET uuid='";
         queryString += newUuid;
         queryString += "' WHERE user_id='";
         queryString += userId;
         queryString += "';";
         dbQuery->query = queryString;

         m_parent->AddQueryToOutput( dbQuery );
      }
   }
   else
   {
      if( updateCreateAccountTableToo )
      {
         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = m_queryType;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET uuid='";
         queryString += newUuid;
         queryString += "' WHERE id='";
         queryString += columnId;
         queryString += "';";
         dbQuery->query = queryString;

         m_parent->AddQueryToOutput( dbQuery );
      }
   }
}

///////////////////////////////////////////////////////////////////////////////////////////
