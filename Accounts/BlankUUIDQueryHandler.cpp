
#include <time.h>
#include <iostream>
using namespace std;

//#include <boost/system/error_code.hpp> 
#include "AccountServer.h"

#include "../NetworkCommon/Platform.h"

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <errno.h>
#endif

#include "../NetworkCommon/Logging/server_log.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/StringUtils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "BlankUUIDQueryHandler.h"
#include "StatusUpdate.h"

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

BlankUUIDQueryHandler::BlankUUIDQueryHandler( U32 id, Queryer* parent, string& query ) : 
                                             ParentType( id, 20, parent ), 
                                             m_isServicingBlankUUID( false ), 
                                             m_useUserTable( false ),
                                             m_numberPendingUuids( 0 )
{
   m_queryString = query;
}

void     BlankUUIDQueryHandler::Update( time_t currentTime )
{
   if( m_numberPendingUuids == 0 ) // other checks are performed inside of this function
   {
      GenerateAListOfAvailableUUIDS();
   }
   ParentType::Update( currentTime, m_isServicingBlankUUID );
}

//---------------------------------------------------------------

bool     BlankUUIDQueryHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   U32 queryType = static_cast< U32 >( dbResult->lookup );
   if( queryType != m_queryType &&
      //queryType != StatusUpdate::QueryType_UserFindBlankUUIDInUsers &&
      queryType != StatusUpdate::QueryType_DoesUuidExist )
      return false;

   if( queryType == m_queryType )
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

         if( m_useUserTable )
            UpdateUuidForUser( userId, email );
         else
            UpdateUuidForTempUser( userId, email );
      }

      if( addedUuids )
      {
         string message = "Accounts::HandleBlankUUIDs some UUIDs were added\n";
         LogMessage( LOG_PRIO_ERR, message.c_str() );
         //cout << message << endl;
      }

      time( &m_lastTimeStamp );// restart timer
      return true;
   }

  /* if( queryType == StatusUpdate::QueryType_UserFindBlankUUIDInUsers )
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
        // string stringToHash = lookup->userId + lookup->columnId + lookup->additionalHashText;
         //string newUuid = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( stringToHash ) ) + rand());
         PrepQueryToLookupUuid();
      }
      else
      {
         if( lookup->createUsersEntry && lookup->userId != "0" && lookup->userId.size() > 0 ) 
         {
       //     PacketDbQuery* dbQuery = new PacketDbQuery;
       //     dbQuery->id = 0;
       //     dbQuery->lookup = m_queryType;
       //     dbQuery->isFireAndForget = true;

       //     dbQuery->query = "UPDATE users SET uuid='";
       //     dbQuery->query += lookup->lastUuidAttempted;
       //     dbQuery->query += "' where user_id='";
      //      dbQuery->query += lookup->userId;
       //     dbQuery->query += "';";

       //     m_parent->AddQueryToOutput( dbQuery );
         }
         
         //if( lookup->updateCreateAccountTableToo )
         {
            
         }
      }

      delete static_cast< UserAccountLookup* >( dbResult->customData );
   }*/

   if( queryType == StatusUpdate::QueryType_DoesUuidExist )
   {
      if( dbResult->bucket.bucket.size() == 0 )
      {
         m_unusedUuids.push_back( dbResult->meta );
      }
      else
      {
         int total = 0;
         KeyValueParser              enigma( dbResult->bucket );
         KeyValueParser::iterator    it = enigma.begin();
         while( it != enigma.end() )
         {
            KeyValueParser::row      row = *it++;

            string   value1 =        row[ TableKeyValue::Column_key ];
            string   value2 =        row[ TableKeyValue::Column_value ];
            total += boost::lexical_cast< int >( value1 ) + boost::lexical_cast< int >( value2 ) ;// should only ever be one
         }
         if( total == 0 )
         {
            m_unusedUuids.push_back( dbResult->meta );
         }
      }
      m_numberPendingUuids--;
   }

   return false;
}

//---------------------------------------------------------------

void     BlankUUIDQueryHandler::GenerateAListOfAvailableUUIDS()
{
   if( m_unusedUuids.size() < 10 && m_numberPendingUuids < 10 )
   {
      const int numToGenerate = 100;
      for( int i=0; i<numToGenerate; i++ )
      {
         time_t currentTime = time(0);
         string uuid = GenerateUUID( static_cast<U32>( currentTime ) );
         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = StatusUpdate::QueryType_DoesUuidExist;
         dbQuery->meta = uuid;

         /*
         SELECT  
(SELECT count(*) FROM playdek.users where uuid="tom.olsen@wizards.com") as count1,
(SELECT count(*) FROM playdek.user_temp_new_user where uuid="tom.olsen@wizards.com") as count2
         */
         string queryString = "SELECT (SELECT count(*) FROM playdek.users where uuid='";
         queryString += uuid;
         queryString += "' ) as count1, (SELECT count(*) FROM playdek.user_temp_new_user where uuid='";
         queryString += uuid;
         queryString += "') as count2;";
         dbQuery->query = queryString;

         m_parent->AddQueryToOutput( dbQuery );
      }

      m_numberPendingUuids += numToGenerate;
   }
}

//---------------------------------------------------------------

void     BlankUUIDQueryHandler::UpdateUuidForTempUser( const string& recordId, const string additionalHashText )
{
   if( recordId.size() == 0 || recordId == "0" )
   {
      string message = "Accounts::UpdateUuidForTempUser userId is null\n";
      LogMessage( LOG_PRIO_ERR, message.c_str() );
      cout << message << endl;
      return;
   }

   string uuid = GenerateUuid();

   //PrepQueryToLookupUuid();
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = m_queryType;
   dbQuery->isFireAndForget = true;

   string queryString = "UPDATE user_temp_new_user SET uuid='";
   queryString += uuid;
   queryString += "' WHERE id='";
   queryString += recordId;
   queryString += "';";
   dbQuery->query = queryString;

   m_parent->AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

void     BlankUUIDQueryHandler::UpdateUuidForUser( const string& recordId, const string additionalHashText )
{
   if( recordId.size() == 0 || recordId == "0" )
   {
      string message = "Accounts::UpdateUuidForUser userId is null\n";
      LogMessage( LOG_PRIO_ERR, message.c_str() );
      cout << message << endl;
      return;
   }

   string uuid = GenerateUuid();

   //PrepQueryToLookupUuid();
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = m_queryType;
   dbQuery->isFireAndForget = true;

   string queryString = "UPDATE users SET uuid='";
   queryString += uuid;
   queryString += "' WHERE user_id='";
   queryString += recordId;
   queryString += "';";
   dbQuery->query = queryString;

   m_parent->AddQueryToOutput( dbQuery );
}

string     BlankUUIDQueryHandler::GenerateUuid( )
{
   //U32 ms = GetCurrentMilliseconds();
   //U32 randomValue = rand();
   //string newUuid = GenerateUUID( ms + static_cast<U32>( GenerateUniqueHash( userId + additionalHashText ) ) + randomValue );
   //cout << "Generate UUID:[" << newUuid << "] using: ms:["<< ms << "], rand:[" << randomValue << "], userId:[" << userId << "], email[" << additionalHashText << "]" << endl;

   if( m_unusedUuids.size() == 0 )
      return GenerateUUID( 0 );

   string uuid = m_unusedUuids.front();
   m_unusedUuids.pop_front();
   
   return uuid;
}

//---------------------------------------------------------------

//void     BlankUUIDQueryHandler::PrepQueryToLookupUuid()
//{
   /*string uuidToLookup = GenerateUuid();

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

   m_parent->AddQueryToOutput( dbQuery );*/
//}

///////////////////////////////////////////////////////////////////////////////////////////
