
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "StatusUpdate.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"

#include "../NetworkCommon/Logging/server_log.h"

#include "email.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <iostream>
using namespace std;

const int OneDay = 3600 * 24;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////////////////////////////////////

StatusUpdate::StatusUpdate( const string& serverName, U32 serverId ) : Queryer(),
                  ParentType( false, DefaultSleepTime, false ),
                  m_newAccountTimeoutSeconds( 48 ),
                  m_checkOnautoCreateTimeoutSeconds( 60 ),
                  m_checkOnOldEmailsTimeoutSeconds( OneDay ),/// once per day
                  m_expireOldAccountRequestsTimeoutSeconds( OneDay ),
                  m_enableAddingUserProducts( false ),
                  m_hasRequestedAdminSettings( false ),
                  m_isWaitingForAdminSettings( false ),
                  m_autoCreateUsersInProcess( false ),
                  m_uuidOnlySevice( false )
{
   SetSleepTime( 500 );
   time( &m_newAccountCreationTimer );
   time( &m_checkOnautoCreateTimer );
   time( &m_checkOnOldEmailsTimer );
   time( &m_expireOldAccountRequestsTimer );
   time( &m_timestampCheckDuplicateUUids );

   m_checkOnOldEmailsTimer -= OneDay; // always check on launch.. no waiting 24 hours.
   m_expireOldAccountRequestsTimer -= OneDay;

   string BlankUUIDWhereClause = " WHERE uuid IS NULL OR uuid='0' LIMIT 30";
   string BlankUUIDSelectClause = "SELECT id, user_email FROM user_temp_new_user";
   bool testingUsers = false;// might be commandline controlled... 
   if( testingUsers == true )
   {
      BlankUUIDSelectClause = "SELECT user_id, user_email FROM users";
   }
   string queryForBlankUUIDs =  BlankUUIDSelectClause + BlankUUIDWhereClause;
   m_blankUuidHandler = new BlankUUIDQueryHandler( QueryType_UserFindBlankUUID, this, queryForBlankUUIDs );
   m_blankUuidHandler->SetPeriodicty( timeoutBlankUUIDTimer );
   if( testingUsers == true )
   {
      m_blankUuidHandler->UseUserTable( true );
   }

   string queryForBlankProfiles = "SELECT user_id FROM users WHERE (user_email IS NOT NULL OR user_confirmation_date IS NOT NULL) AND user_id NOT IN (SELECT user_id FROM user_profile) LIMIT 200";
   m_blankUserProfileHandler = new BlankUserProfileHandler( QueryType_UserFindBlankUserProfile, this, queryForBlankProfiles );
   m_blankUserProfileHandler->SetPeriodicty( timeoutBlankUserProfileTimer );

   string queryForNewAccounts = "SELECT * FROM user_temp_new_user WHERE was_email_sent='0' AND flagged_as_invalid='0' AND uuid IS NOT NULL"; // the last part is to prevent conflicts with the blank UUID handler
   m_newAccountHandler = new NewAccountQueryHandler( QueryType_UserCheckForNewAccount, this, queryForNewAccounts );
   m_newAccountHandler->SetPeriodicty( timeoutNewAccount );
   m_newAccountHandler->SetQueryTypeForLoadingStrings( QueryType_LoadStrings );
   m_newAccountHandler->SetQueryTypeForLoadingWeblinks( QueryType_LoadWeblinks );
   m_newAccountHandler->SetQueryTypeForOlderEmailsResent( QueryType_ResendEmailToOlderAccounts );
   m_newAccountHandler->SetBlankUUIDHandler( m_blankUuidHandler );

   m_addProductEntryHandler = new ProductEntryCreateBasedOnPlayHistory( QueryType_ProductEntryCreateBasedOnPlayHistory, this );
   m_addProductEntryHandler->SetPeriodicty( 28 );

   string queryForPasswordReset = "SELECT reset_password_keys.id, users.user_email, users.language_id, reset_password_keys.reset_key FROM users JOIN reset_password_keys ON reset_password_keys.user_account_uuid = users.uuid WHERE reset_password_keys.was_email_sent=0";

   m_resetPasswordHandler = new ResetPasswordQueryHandler( QueryType_ResetPasswords, this, queryForPasswordReset );
   m_resetPasswordHandler->SetPeriodicty( timeoutResetPassword );

   
  /* string queryForChangeEmail = "SELECT id, user_account_uuid, reset_key, new_email, new_username, time_last_email_sent, language_id, user_email, user_name FROM playdek.reset_user_email_name INNER JOIN playdek.users ON reset_user_email_name.user_account_uuid=users.uuid WHERE was_email_sent=0;";

   m_resetUsernameEmailHandler = new ResetUserEmailQueryHandler( QueryType_ChangeUsernameEmail, this, queryForChangeEmail );
   m_resetUsernameEmailHandler->SetPeriodicty( timeoutChangeUsernameEmail );*/

   LogOpen();
   LogMessage( LOG_PRIO_INFO, "Accounts::Accounts server created\n" );
   cout << "Accounts::Accounts server created" << endl;

   srand( GetCurrentMilliseconds() );
}

StatusUpdate::~StatusUpdate()
{
   delete m_blankUuidHandler;
   delete m_blankUserProfileHandler;
   delete m_newAccountHandler;
   delete m_resetPasswordHandler;
   //delete m_resetUsernameEmailHandler;
}

//---------------------------------------------------------------

bool     StatusUpdate::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   LogMessage( LOG_PRIO_INFO, "Accounts::Data in\n" );

   cout << "Accounts::Data in" << endl;

   if( packet->packetType != PacketType_GatewayWrapper )
   {
      string text = "Login server: received junk packets. Type: ";
      text += packet->packetType;
      Log( text, 4 );
      return false;
   }

   /// this service does not accept normal user connections.
   return false;

}

//---------------------------------------------------------------

void  StatusUpdate::ResendEmailToOlderAccounts()
{
   if( isMailServiceEnabled == false )
      return;

   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_checkOnOldEmailsTimer ) >= m_checkOnOldEmailsTimeoutSeconds ) 
   {
      m_checkOnOldEmailsTimer = testTimer;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_ResendEmailToOlderAccounts;

      string preparedDate = GetDateInUTC( -3, 0, 0 );// get three days ago
      dbQuery->query = "SELECT * FROM user_temp_new_user WHERE time_created<'";
      dbQuery->query += preparedDate;
      dbQuery->query += "' AND flagged_as_invalid=0 AND email_returned_as_undeliverable=0 AND was_email_sent<2";
      AddQueryToOutput( dbQuery );

      LogMessage( LOG_PRIO_INFO, "Accounts::ResendEmailToOlderAccounts\n" );
   }
}

//---------------------------------------------------------------

void     StatusUpdate::ResendEmailToOlderAccountsResult( PacketDbQueryResult* dbResult )
{
 //  HandleNewAccounts( dbResult );
   m_newAccountHandler->HandleResult( dbResult );
}

//---------------------------------------------------------------

void     StatusUpdate::ExpireOldUserAccountRequests()
{
   if( isMailServiceEnabled == false )
      return;

   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_expireOldAccountRequestsTimer ) >= m_expireOldAccountRequestsTimeoutSeconds ) 
   {
      m_expireOldAccountRequestsTimer = testTimer;
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_MoveOlderAccountsRequest;
      dbQuery->isFireAndForget = true;

      string preparedDate = GetDateInUTC( -8, 0, 0 );// get three days ago
      dbQuery->query = "INSERT user_pending_expired SELECT * FROM user_temp_new_user WHERE time_created<'";
      dbQuery->query += preparedDate;
      dbQuery->query += "'";
      AddQueryToOutput( dbQuery );

      
      dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_DeleteOlderAccountsRequest;
      dbQuery->isFireAndForget = true;

      dbQuery->query = "DELETE FROM user_temp_new_user WHERE time_created<'";
      dbQuery->query += preparedDate;
      dbQuery->query += "'";
      AddQueryToOutput( dbQuery );


      LogMessage( LOG_PRIO_INFO, "Accounts::ExpireOldUserAccountRequests\n" );

   }
}

//---------------------------------------------------------------

// this hack was added once I discovered that multiple users had the same UUID
/*bool runOnce = true;
void  StatusUpdate::Hack()
{
   if( isMailServiceEnabled == false )
      return;

   if( runOnce )
   {
      runOnce = false;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_Hack;

      string preparedDate = GetDateInUTC( -3, 0, 0 );// get three days ago
      dbQuery->query = "select id from user_temp_new_user where uuid in ( select uuid from user_temp_new_user group by uuid having count(uuid) > 1)";
      AddQueryToOutput( dbQuery );
   }
}*/
/*
void     StatusUpdate::HackResult( PacketDbQueryResult* dbResult )
{
   //HandleNewAccounts( dbResult );
   NewUsersTable              enigma( dbResult->bucket );
   NewUsersTable::iterator    it = enigma.begin();
   
   while( it != enigma.end() )
   {
      NewUsersTable::row         row = *it++;
      string columnId =          row[ TableUserTempNewUser::Column_id ];
      string name =              row[ TableUserTempNewUser::Column_name ];

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_Hack;
      dbQuery->isFireAndForget = true;

      string newUuid = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( columnId + name ) ) );

      // update user_temp_new_user set uuid='12345678' where id='2719'
      dbQuery->query = "Update user_temp_new_user SET uuid='";
      dbQuery->query += newUuid;
      dbQuery->query += "' WHERE id='";
      dbQuery->query += columnId;
      dbQuery->query += "'";
      AddQueryToOutput( dbQuery );
   }
}*/

//---------------------------------------------------------------

// this hack was added once I discovered that multiple users had the same UUID
void  StatusUpdate::DuplicateUUIDSearch()
{
   if( isMailServiceEnabled == false )
      return;

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampCheckDuplicateUUids ) >= timeoutCheckDuplicateUUids ) 
   {
      m_timestampCheckDuplicateUUids = currentTime;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_DuplicateUUIDSearch;
      dbQuery->query = "SELECT id, uuid FROM user_temp_new_user WHERE uuid IN ( SELECT uuid FROM user_temp_new_user GROUP BY uuid HAVING COUNT(uuid) > 1)";
      AddQueryToOutput( dbQuery );

      dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_DuplicateUUIDSearch;
      dbQuery->query = "SELECT user_temp_new_user.id, user_temp_new_user.uuid FROM user_temp_new_user INNER JOIN users ON user_temp_new_user.uuid=users.uuid";
      AddQueryToOutput( dbQuery );
   }
}

//---------------------------------------------------------------

void     StatusUpdate::DuplicateUUIDSearchResult( PacketDbQueryResult* dbResult )
{
   KeyValueParser              enigma( dbResult->bucket );
   KeyValueParser::iterator    it = enigma.begin();

   static U32 counter = 1;
   
   while( it != enigma.end() )
   {
      KeyValueParser::row      row = *it++;
      string columnId =        row[ TableKeyValue::Column_key ];
      string uuid =            row[ TableKeyValue::Column_value ];

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_DuplicateUUIDSearch;
      dbQuery->isFireAndForget = true;

      string newUuid = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( columnId + uuid ) ) + counter );
      counter ++;// guaranteeing incremental values in case the app is too fast for the millisecond timer

      dbQuery->query = "UPDATE user_temp_new_user SET uuid='";
      dbQuery->query += newUuid;
      dbQuery->query += "' WHERE id='";
      dbQuery->query += columnId;
      dbQuery->query += "'";
      AddQueryToOutput( dbQuery );

      string message = "Accounts::DuplicateUUIDSearchResult duplicate UUID changed from ";
      message += uuid;
      message += " to ";
      message += newUuid;
      message += "\n";

      //LogMessage( LOG_PRIO_ERR, message.c_str() );
   }
}

//---------------------------------------------------------------

int      StatusUpdate::CallbackFunction()
{
   if( isMailServiceEnabled == true )
   {
      time_t currentTime;
      time( &currentTime );

      if( m_hasRequestedAdminSettings == false )
      {
         RequestAdminSettings();
         m_hasRequestedAdminSettings = true;
         return 0;
      }
      else if( m_isWaitingForAdminSettings == true )
      {
         return 0;
      }

      

//      LookForFlaggedAutoCreateAccounts();
      //if( m_

      if( m_uuidOnlySevice == false )
      {
         if( m_newAccountHandler->IsReady() )
         {
            ResendEmailToOlderAccounts();
            
            LookForFlaggedAutoCreateAccounts();
            ExpireOldUserAccountRequests();
            //Hack();// no longer needed
            m_resetPasswordHandler->Update( currentTime );

            // we are only testing so this should be moved
            //m_resetUsernameEmailHandler->Update( currentTime );
         }
         
         m_newAccountHandler->Update( currentTime );
      }
      
      m_blankUuidHandler->Update( currentTime );
      //DuplicateUUIDSearch();// no longer needed after-the-fact.

      m_blankUserProfileHandler->Update( currentTime ); 
      if( m_enableAddingUserProducts )
      {
         m_addProductEntryHandler->Update( currentTime );
      }
   }
   return 1;
}


//---------------------------------------------------------------

bool     StatusUpdate::AddQueryToOutput( PacketDbQuery* packet )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( packet, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   delete packet;/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}

//---------------------------------------------------------------

bool     StatusUpdate::AddOutputChainData( BasePacket* packet, U32 connectionId )
{
   // this should be a DB Query Response only. Lookup the appropriate gateway connection and push the login result back out.
   // If 
   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         bool wasHandled = false;
         PacketDbQueryResult* dbResult = static_cast<PacketDbQueryResult*>( packet );
         if( m_blankUuidHandler->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }
         else if( m_blankUserProfileHandler->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }
         else if( m_newAccountHandler->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }
         else if( m_addProductEntryHandler->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }
         else if( m_resetPasswordHandler->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }
        /* else if( m_resetUsernameEmailHandler->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }*/
         else
         {
            switch( dbResult->lookup )
            {
               case QueryType_GetNewUserRecordAndCreateProfileFromIt:
                  {
                     if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                     {
                        InsertNewUserProfile( dbResult );
                     }
                     wasHandled = true;
                  }
                  break;
               case QueryType_AccountAdminSettings:
                  {
                     HandleAdminSettings( dbResult );
                     wasHandled = true;
                  }
                  break;
               case QueryType_AutoCreateUsers:
                  {
                     if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                     {
                        HandleAutoCreateAccounts( dbResult );
                     }
                     else
                     {
                        //string str = "Config table failed to load";
                        //Log( str );
                     }
                     m_autoCreateUsersInProcess = false;
                     wasHandled = true;
                  }
                  break;
               /*case QueryType_Hack:
                  {
                     if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                     {
                        HackResult( dbResult );
                     }
                     
                     wasHandled = true;
                  }
                  break;*/
               case QueryType_ResendEmailToOlderAccounts:
                  {
                     if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                     {
                        ResendEmailToOlderAccountsResult( dbResult );
                     }
                     
                     wasHandled = true;
                  }
                  break;
               case QueryType_DuplicateUUIDSearch:
                  {
                     if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                     {
                        DuplicateUUIDSearchResult( dbResult );
                     }

                     wasHandled = true;
                  }
                  break;
               /*case QueryType_ResetPasswords:
                  {
                     if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                     {
                        HandleResetPassword( dbResult );
                     }
                     
                     wasHandled = true;
                  }
                  break;*/
            }
         }
         if( wasHandled == true )
         {
            delete dbResult;
         }
         return wasHandled;
      }
   }
   return false;
}

//---------------------------------------------------------------

void     StatusUpdate::HandleAutoCreateAccounts( const PacketDbQueryResult* dbResult )
{
   //insert_or_update;
   NewUsersTable              enigma( dbResult->bucket );
   NewUsersTable::iterator    it = enigma.begin();
   
   while( it != enigma.end() )
   {
      NewUsersTable::row         row = *it++;
      string uuid =              row[ TableUserTempNewUser::Column_uuid ]; //<< must be created
      string id =                row[ TableUserTempNewUser::Column_id ];      
      string name =              row[ TableUserTempNewUser::Column_name ];
      string email =             row[ TableUserTempNewUser::Column_email ];
      string userId =            row[ TableUserTempNewUser::Column_user_id ];
      string timeCreated =       row[ TableUserTempNewUser::Column_time_created ];
      string productId =         row[ TableUserTempNewUser::Column_game_id ];
      if( productId == "null" ) // only corrects for bad data.
         productId = "1";

      //string uuid =              row[ TableUserTempNewUser::Column_uuid ]; //<< must be created
      if( uuid == "" || uuid.size() == 0 )
         uuid = m_blankUuidHandler->GenerateUuid();

      string gamekitHash =       row[ TableUserTempNewUser::Column_gamekit_hash ];
      string passwordHash =      row[ TableUserTempNewUser::Column_user_pw_hash ];
      string lowerCaseUserName = row[ TableUserTempNewUser::Column_user_name_match ];
      string languageId =        row[ TableUserTempNewUser::Column_language_id ];

      string query = "INSERT INTO users (user_id, user_name, user_name_match, user_pw_hash, user_email, user_gamekit_hash, user_creation_date, uuid, active, language_id, user_confirmation_date) VALUES ('";
      query += userId;
      query += "', '%s', '%s', ";
      query += passwordHash;
      query += ", '%s', '";
      query += gamekitHash;
      query += "', '";
      query += timeCreated;
      query += "', '";
      query += uuid;
      query += "', ";
      query += "'1',";
      query += languageId;   
      query += ", '";
      query += timeCreated;
      query += "') ON DUPLICATE KEY UPDATE user_name='%s', user_name_match='%s', user_pw_hash='";
      query += passwordHash;
      query += "', user_email='%s', user_gamekit_hash='";
      query += gamekitHash;
      query += "', user_creation_date='";
      query += timeCreated;
      query += "', active='1', language_id=";
      query += languageId;
      query += ";";

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_AutoCreateUsersInsertOrUpdate;
      dbQuery->isFireAndForget = true;
      
      dbQuery->escapedStrings.insert( name );
      dbQuery->escapedStrings.insert( lowerCaseUserName );
      dbQuery->escapedStrings.insert( email );
      dbQuery->escapedStrings.insert( name );
      dbQuery->escapedStrings.insert( lowerCaseUserName );
      dbQuery->escapedStrings.insert( email );

      dbQuery->query = query;
      AddQueryToOutput( dbQuery );

     // remove the temp record.
      query = "DELETE FROM user_temp_new_user WHERE uuid='";
      query += uuid;
      query += "';";

      dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_DeleteTempNewUserRecord;
      dbQuery->isFireAndForget = true;

      dbQuery->query = query;
      AddQueryToOutput( dbQuery );

      LogMessage( LOG_PRIO_INFO, "Accounts::HandleAutoCreateAccounts\n" );

      // create a profile
      dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_GetNewUserRecordAndCreateProfileFromIt;
      dbQuery->meta = productId;

      dbQuery->query = "SELECT user_id FROM users WHERE uuid='";
      dbQuery->query += uuid;
      dbQuery->query += "'";
      AddQueryToOutput( dbQuery );
      
   }
}

//---------------------------------------------------------------

void     StatusUpdate::InsertNewUserProfile( const PacketDbQueryResult* dbResult )
{
   KeyValueParser              enigma( dbResult->bucket );
   KeyValueParser::iterator    it = enigma.begin();
   if( it != enigma.end() )
   {
      KeyValueParser::row      row = *it++;
      string user_id =         row[ TableKeyValue::Column_key ];

      int productId = boost::lexical_cast< int >( dbResult->meta );
      m_blankUserProfileHandler->CreateBlankProfile( user_id, productId );
   }
}

//---------------------------------------------------------------

void     StatusUpdate::HandleAdminSettings( const PacketDbQueryResult* dbResult )
{
    KeyValueParser              enigma( dbResult->bucket );
   KeyValueParser::iterator    it = enigma.begin();

   //U32 counter = 1;
   
   while( it != enigma.end() )
   {
      KeyValueParser::row      row = *it++;
      string setting =         row[ TableKeyValue::Column_key ];
      string value =           row[ TableKeyValue::Column_value ];

      if( setting == "play_history_product_id" )
      {
         m_addProductEntryHandler->SetProductIdStart( boost::lexical_cast< int >( value ) );
      }
      else if( setting == "play_history_user_id" )
      {
         m_addProductEntryHandler->SetUserIdStart( boost::lexical_cast< int >( value ) ); 
      }

   }

   m_isWaitingForAdminSettings = false;
}

//---------------------------------------------------------------

void     StatusUpdate::RequestAdminSettings()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = StatusUpdate::QueryType_AccountAdminSettings;
   dbQuery->query = "SELECT * FROM admin_account";

   m_isWaitingForAdminSettings = true;

   AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

void     StatusUpdate::LookForFlaggedAutoCreateAccounts()
{
   if( m_autoCreateUsersInProcess == true )
      return;

   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_checkOnautoCreateTimer ) >= m_checkOnautoCreateTimeoutSeconds ) /// only check once every 60 seconds
   {
      //cout << "LookForFlaggedAutoCreateAccounts..." << endl;
      m_checkOnautoCreateTimer = testTimer;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_AutoCreateUsers;

      dbQuery->query = "SELECT * FROM user_temp_new_user WHERE flagged_auto_create='1'";

      AddQueryToOutput( dbQuery );

      m_autoCreateUsersInProcess = true;

      //LogMessage( LOG_PRIO_INFO, "Accounts::LookForFlaggedAutoCreateAccounts\n" );
   }
}

//---------------------------------------------------------------
