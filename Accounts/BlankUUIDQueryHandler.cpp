
#include <time.h>
#include <iostream>

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "BlankUUIDQueryHandler.h"
#include "StatusUpdate.h"
#include <boost/lexical_cast.hpp>

using namespace std;

struct UserAccountLookup
{
   string   userId;
   //bool     updateCreateAccountTableToo;
   //string   columnId;
   //bool     createUsersEntry;
   string   lastUuidAttempted;
   string   additionalHashText;
};

///////////////////////////////////////////////////////////////////////////////////////////

BlankUUIDQueryHandler::BlankUUIDQueryHandler( U32 id, Queryer* parent, string& query ) : QueryHandler< Queryer* >( id, 20, parent ), m_isServicingBlankUUID( false )
{
   m_queryString = query;
}

void     BlankUUIDQueryHandler::Update( time_t currentTime )
{
   QueryHandler< Queryer* >::Update( currentTime, m_isServicingBlankUUID );
}

//---------------------------------------------------------------

bool     BlankUUIDQueryHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   int resultType = dbResult->lookup;
   if( resultType != m_queryType &&
      resultType != StatusUpdate::QueryType_UserFindBlankUUIDInUsers  )
      return false;

   if( resultType == m_queryType )
   {
      SetValueOnExit< bool >           setter( m_isServicingBlankUUID, false );// due to multiple exit points...

      bool addedUuids = false;
      KeyValueParser              enigma( dbResult->bucket );
      KeyValueParser::iterator    it = enigma.begin();
    /*  if( enigma.m_bucket.size() > 0 )
      {
         cout << "Found user record with UUID blank matching" << endl;
         cout << " Successful query = " << m_queryString << endl;
      }*/
      while( it != enigma.end() )
      {
         addedUuids = true;
         KeyValueParser::row      row = *it++;

         string   userId =        row[ TableKeyValue::Column_key ];
         string   email =         row[ TableKeyValue::Column_value ];

         cout << "User id replaced = " << userId << endl;

         UpdateUuidForTempUser( userId, email );
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

   if( resultType == StatusUpdate::QueryType_UserFindBlankUUIDInUsers )
   {
      UserAccountLookup* lookup = static_cast< UserAccountLookup* >( dbResult->customData );
      IndexTableParser              enigma( dbResult->bucket );
      IndexTableParser::iterator    it = enigma.begin();
      if( enigma.m_bucket.size() > 0 )// we should find no result
      {
         cout << "**************************************" << endl;
         cout << "Found user record with UUID conflict" << endl;
         cout << "UUID in conflict was: " << lookup->lastUuidAttempted << endl;
         cout << "**************************************" << endl;
         //string stringRand = boost::lexical_cast< string >(  );
        /* string stringToHash = lookup->userId + lookup->columnId + lookup->additionalHashText;
         string newUuid = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( stringToHash ) ) + rand());*/
         PrepQueryToLookupUuid( lookup->userId, lookup->additionalHashText );
      }
      else
      {
         /*if( lookup->createUsersEntry && lookup->userId != "0" && lookup->userId.size() > 0 ) 
         {
            PacketDbQuery* dbQuery = new PacketDbQuery;
            dbQuery->id = 0;
            dbQuery->lookup = m_queryType;
            dbQuery->isFireAndForget = true;

            dbQuery->query = "UPDATE users SET uuid='";
            dbQuery->query += lookup->lastUuidAttempted;
            dbQuery->query += "' where user_id='";
            dbQuery->query += lookup->userId;
            dbQuery->query += "';";

            m_parent->AddQueryToOutput( dbQuery );
         }*/
         
         //if( lookup->updateCreateAccountTableToo )
         {
            PacketDbQuery* dbQuery = new PacketDbQuery;
            dbQuery->id = 0;
            dbQuery->lookup = m_queryType;
            dbQuery->isFireAndForget = true;

            string queryString = "UPDATE user_temp_new_user SET uuid='";
            queryString += lookup->lastUuidAttempted;
            queryString += "' WHERE id='";
            queryString += lookup->userId;
            queryString += "';";
            dbQuery->query = queryString;

            m_parent->AddQueryToOutput( dbQuery );
         }
      }

      delete static_cast< UserAccountLookup* >( dbResult->customData );
   }

   return false;
}

//---------------------------------------------------------------

void     BlankUUIDQueryHandler::UpdateUuidForTempUser( const string& recordId, const string additionalHashText )
{
   if( recordId.size() == 0 || recordId == "0" )
   {
      string message = "Accounts::UpdateUuidForUser userId is null\n";
      //LogMessage( LOG_PRIO_ERR, message.c_str() );
      cout << message << endl;
      return;
   }

   PrepQueryToLookupUuid( recordId, additionalHashText );
}

string     BlankUUIDQueryHandler::GenerateUuid( const string& userId, const string& additionalHashText )
{
   U32 ms = GetCurrentMilliseconds();
   U32 randomValue = rand();
   string newUuid = GenerateUUID( ms + static_cast<U32>( GenerateUniqueHash( userId + additionalHashText ) ) + randomValue );
   cout << "Generate UUID:[" << newUuid << "] using: ms:["<< ms << "], rand:[" << randomValue << "], userId:[" << userId << "], email[" << additionalHashText << "]" << endl;

   return newUuid;
}

//---------------------------------------------------------------

void     BlankUUIDQueryHandler::PrepQueryToLookupUuid( const string& userId, const string& additionalHashText )
{
   string uuidToLookup = GenerateUuid( userId, additionalHashText );

   string queryForBothTableLookups = "(SELECT uuid from users where uuid = '";
   queryForBothTableLookups += uuidToLookup;
   queryForBothTableLookups += "') UNION (SELECT uuid from user_temp_new_user where uuid = '";
   queryForBothTableLookups += uuidToLookup;
   queryForBothTableLookups += "')";

   
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = StatusUpdate::QueryType_UserFindBlankUUIDInUsers;

   dbQuery->query = queryForBothTableLookups;

   UserAccountLookup* userAccountLookup = new UserAccountLookup;
   //userAccountLookup->columnId = columnId;
  // userAccountLookup->createUsersEntry = (userId.size() != 0 && userId != "0" );
   //userAccountLookup->updateCreateAccountTableToo = updateCreateAccountTableToo;
   userAccountLookup->userId = userId;
   userAccountLookup->lastUuidAttempted = uuidToLookup;
   userAccountLookup->additionalHashText = additionalHashText;

   dbQuery->customData = userAccountLookup;

   m_parent->AddQueryToOutput( dbQuery );
}

///////////////////////////////////////////////////////////////////////////////////////////
