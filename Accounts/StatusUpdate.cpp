
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

const bool isMailServiceEnabled = true;

const char* newAccountEmailAddress = "account_create@playdekgames.com";
const char* resetPasswordEmailAddress = "account_reset@playdekgames.com";
const char* mailServer = "mail.playdekgames.com";
const int OneDay = 3600 * 24;

StatusUpdate::StatusUpdate( const string& serverName, U32 serverId ) : Threading::CChainedThread < BasePacket* >( false, DefaultSleepTime, false ),
                  m_hasLoadedStringTable( false ),
                  m_hasLoadedWeblinks( false ),
                  m_checkOnBlankUuidTimeoutSeconds( 55 ),
                  m_newAccountTimeoutSeconds( 48 ),
                  m_checkOnautoCreateTimeoutSeconds( 60 ),
                  m_checkOnOldEmailsTimeoutSeconds( OneDay ),/// once per day
                  m_expireOldAccountRequestsTimeoutSeconds( OneDay ),
                  m_resetPasswordEmailTimeoutSeconds( 30 )
{
   SetSleepTime( 500 );
   time( &m_newAccountCreationTimer );
   time( &m_checkOnBlankUuidTimer );
   time( &m_checkOnautoCreateTimer );
   time( &m_checkOnOldEmailsTimer );
   time( &m_expireOldAccountRequestsTimer );
   time( &m_resetPasswordEmailTimer );

   m_checkOnOldEmailsTimer -= OneDay; // always check on launch.. no waiting 24 hours.
   m_expireOldAccountRequestsTimer -= OneDay;

   LogOpen();
   LogMessage( LOG_PRIO_INFO, "Accounts::Accounts server created\n" );
   cout << "Accounts::Accounts server created" << endl;
}

StatusUpdate::~StatusUpdate()
{
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

void     StatusUpdate::FillInUserAccountUUIDs()
{
   if( isMailServiceEnabled == false )
      return;

   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_checkOnBlankUuidTimer ) >= m_checkOnBlankUuidTimeoutSeconds ) 
   {
      //cout << "FillInUserAccountUUIDs..." << endl;

      m_checkOnBlankUuidTimer = testTimer;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_UserFindBlankUUID;

      dbQuery->query = "SELECT user_id FROM users where uuid is NULL LIMIT 30";

      AddQueryToOutput( dbQuery );
      //LogMessage( LOG_PRIO_INFO, "Accounts::FillInUserAccountUUIDs\n" );
   }
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
   HandleNewAccounts( dbResult );
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
bool runOnce = true;
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
}

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
}

//---------------------------------------------------------------

// this hack was added once I discovered that multiple users had the same UUID
void  StatusUpdate::DuplicateUUIDSearch()
{
   if( isMailServiceEnabled == false )
      return;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = QueryType_DuplicateUUIDSearch;

   string preparedDate = GetDateInUTC( -3, 0, 0 );// get three days ago
   dbQuery->query = "select id, uuid from user_temp_new_user where uuid in ( select uuid from user_temp_new_user group by uuid having count(uuid) > 1)";
   AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

void     StatusUpdate::DuplicateUUIDSearchResult( PacketDbQueryResult* dbResult )
{
   //HandleNewAccounts( dbResult );
   KeyValueParser              enigma( dbResult->bucket );
   KeyValueParser::iterator    it = enigma.begin();

   U32 counter = 1;
   
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

      dbQuery->query = "Update user_temp_new_user SET uuid='";
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

void     StatusUpdate::HandleBlankUUIDs( PacketDbQueryResult* dbResult )
{
   if( isMailServiceEnabled == false )
      return;

   //cout << "HandleBlankUUIDs..." << endl;

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
}

//---------------------------------------------------------------

void     StatusUpdate::UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId )
{
   if( ( userId.size() == 0 || userId == "0" ) && ( columnId.size() == 0 || columnId == "0" ) )
   {
      string message = "Accounts::UpdateUuidForUser userId is null\n";
      LogMessage( LOG_PRIO_ERR, message.c_str() );
      cout << message << endl;
      return;
   }

   string newUuid = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( userId + columnId ) ) );

   if( userId.size() != 0 && userId != "0" ) 
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_UserFindBlankUUID;
      dbQuery->isFireAndForget = true;

      dbQuery->query = "UPDATE users SET uuid='";
      dbQuery->query += newUuid;
      dbQuery->query += "' where user_id='";
      dbQuery->query += userId;
      dbQuery->query += "';";

      AddQueryToOutput( dbQuery );

      if( updateCreateAccountTableToo )
      {
         dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_UserFindBlankUUID;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET uuid='";
         queryString += newUuid;
         queryString += "' WHERE user_id='";
         queryString += userId;
         queryString += "';";
         dbQuery->query = queryString;

         AddQueryToOutput( dbQuery );
      }
   }
   else
   {
      if( updateCreateAccountTableToo )
      {
         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_UserFindBlankUUID;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET uuid='";
         queryString += newUuid;
         queryString += "' WHERE id='";
         queryString += columnId;
         queryString += "';";
         dbQuery->query = queryString;

         AddQueryToOutput( dbQuery );
      }
   }
}

//---------------------------------------------------------------

string   StatusUpdate::GetString( const string& stringName, int languageId )
{
   stringhash lookupHash = GenerateUniqueHash( stringName );
   StringTableLookup::iterator it = m_stringsTable.find( lookupHash );

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

void     StatusUpdate::ReplaceAllLookupStrings( string& bodyText, int languageId,  map< string, string >& specialStrings )
{
   vector<string> dictionary = CreateDictionary( bodyText );
   map< string, string > replacements;

   vector<string>::iterator it = dictionary.begin(); 
   while( it != dictionary.end() )
   {
      string lookupString = *it++;

      string actualString;
      map< string, string >::iterator foundIter = specialStrings.find( lookupString );
      if( foundIter != specialStrings.end() )
      {
         actualString = foundIter->second;
         //actualString += "\r\n";// keeping the strings from going too long ... this doesn't work on Apple's email system.
      }
      else
      {
         actualString = GetString( lookupString, languageId );
         actualString += "\r\n";// keeping the strings from going too long
      }
      //if( actualString.size() )
      {
         string replaceString = "%";
         replaceString += lookupString;
         replaceString += "%";
         boost::replace_first( bodyText, replaceString, actualString );
      }
   }
}
/*
emailDomainReplacements [] = 
{
   { "gamil", "gmail.com" },
   { "gmaill", "gmail" },
   { "gmai", "gmail" },

   { "hahoo", "yahoo" },
   { "ahoo", "yahoo" },

   { "otmail", "hotmail" },
   { "hoymail", "hotmail" },
}

tailReplacements [] =
{
   { "co", "com" },
   { "con", "com" },
   { "cob", "com" },
   { "ccom", "com" },
   { "om", "com" },
};*/
//---------------------------------------------------------------

void     StatusUpdate::HandleNewAccounts( const PacketDbQueryResult* dbResult )
{
   //cout << "HandleNewAccounts..." << endl;
   //LogMessage( LOG_PRIO_INFO, "HandleNewAccounts...\n" );
   NewUsersTable              enigma( dbResult->bucket );
   NewUsersTable::iterator    it = enigma.begin();
   
   while( it != enigma.end() )
   {
      NewUsersTable::row         row = *it++;
      string columnId =          row[ TableUserTempNewUser::Column_id ];
      string name =              row[ TableUserTempNewUser::Column_name ];
      string email =             row[ TableUserTempNewUser::Column_email ];
      string userId =            row[ TableUserTempNewUser::Column_user_id ];
      string uuid =              row[ TableUserTempNewUser::Column_uuid ];
      int languageId =           boost::lexical_cast< int >( row[ TableUserTempNewUser::Column_language_id ] );

      string message = "Sending new account confirmation to user: ";
      message += name;
      message += " at email: ";
      message += email;

      LogMessage( LOG_PRIO_INFO, message.c_str() );
      cout << message << endl;

      if( email.size() == 0 )// we can't send an email...
      {
         string message = "User does not have a valid email: ";
         message += name;
         LogMessage( LOG_PRIO_WARN, message.c_str() );
         cout << message << endl;
         continue;
      }

      if( IsValidEmailAddress( email ) )
      {
         string subjectText = GetString( "email.new_account.welcome.subject", languageId ); //"Confirmation email";
         string bodyText = GetString( "email.new_account.welcome.body_text", languageId );//"Thank you for signing up with Playdek. Click this link to confirm your new account.";

         string userLookupKey = GenerateUUID( GetCurrentMilliseconds() + static_cast<U32>( GenerateUniqueHash( name + email ) ) );
         userLookupKey += GenerateUUID( GetCurrentMilliseconds() ); // double long text

         string linkPath = m_linkToAccountCreated;
         linkPath += "?key=";
         linkPath += userLookupKey;


         if( m_confirmationEmailTemplate.size() )
         {
            bodyText = m_confirmationEmailTemplate;
            map< string, string > specialStrings;
            specialStrings.insert( pair<string, string> ( "link-to-confirmation", linkPath ) );
            ReplaceAllLookupStrings( bodyText, languageId, specialStrings );
         }
         else
         {
            bodyText += "<a href='";
            bodyText += linkPath;
            bodyText += "'>Playdek.com</a>";
         }

         // update playdek.user_temp_new_user set was_email_sent=was_email_sent+1, lookup_key='lkjasdfhlkjhadfs' where id='4' ;
         SendConfirmationEmail( email.c_str(), newAccountEmailAddress, mailServer, bodyText.c_str(), subjectText.c_str(), "Playdek.com", linkPath.c_str() );

         // it is likely that the new user does not have a UUID yet so we will add it to both tables
         UpdateUuidForUser( userId, true, columnId );

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_UserCheckForNewAccount;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET was_email_sent=was_email_sent+1, lookup_key='";
         queryString += userLookupKey;
         queryString += "', time_last_confirmation_email_sent='";
         queryString += GetDateInUTC();
         queryString += "' WHERE id='";
         queryString += columnId;
         queryString += "';";
         dbQuery->query = queryString;

         AddQueryToOutput( dbQuery );

         message = "new account confirmation to user: ";
         message += name;
         message += " at email: ";
         message += email;
         message += " ... lookup_key = ";
         message += userLookupKey;

         LogMessage( LOG_PRIO_INFO, message.c_str() );
         //cout << message << endl;

         //cout << "Time sent: ";
         //PrintCurrentTime();
      }
      else
      {
         message = "new account confirmation failed due to invalid email address: ";
         message += name;
         message += " at email: '";
         message += email;
         LogMessage( LOG_PRIO_INFO, message.c_str() );
         cout << message << endl;

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_UserCheckForNewAccount;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET flagged_as_invalid='1' WHERE id='";
         queryString += columnId;
         queryString += "';";
         dbQuery->query = queryString;

         AddQueryToOutput( dbQuery );
      }
   }

   // we update the timer to prevent this firing too often. Email can be slow and when it is, we can end up sending two emails 
   // to the same user which invalidates the first. By resetting the timer here, we put a gap in between the reads and allow the db to 
   // service the writes and prevent multiple emails.
   time( &m_newAccountCreationTimer );

   // fix up any weird UUID problems
   DuplicateUUIDSearch();
}


//---------------------------------------------------------------

int      StatusUpdate::CallbackFunction()
{
   if( isMailServiceEnabled == true )
   {
      if( m_hasLoadedWeblinks && m_hasLoadedStringTable )
      {
         ResendEmailToOlderAccounts();
         CheckForNewAccounts();
         LookForFlaggedAutoCreateAccounts();
         ExpireOldUserAccountRequests();
         Hack();
         CheckForResetPassword();
      }
      
      FillInUserAccountUUIDs();
      PreloadLanguageStrings();
      PreloadWeblinks();
   }
   return 1;
}


//---------------------------------------------------------------

bool     StatusUpdate::AddQueryToOutput( PacketDbQuery* packet )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainedInterface<BasePacket*>* outputPtr = (*itOutputs).m_interface;
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
         switch( dbResult->lookup )
         {
            cout << "Db query type:"<< dbResult->lookup << ", success=" << dbResult->successfulQuery << endl;
            case QueryType_UserCheckForNewAccount:
               {
                  if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                  {
                     HandleNewAccounts( dbResult );
                  }
                  else
                  {
                     string str = "New user accounts db query failed";
                     Log( str );

                  }

                  wasHandled = true;
               }
               break;
            case QueryType_UserFindBlankUUID:
               {
                  if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                  {
                     HandleBlankUUIDs( dbResult );
                  }
                /*  else
                  {
                     string str = "Find users with blank UUID failed";
                     Log( str );
                  }*/
                  
                  wasHandled = true;
               }
               break;
            case QueryType_LoadStrings:
               {
                  if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                  {
                     SaveStrings( dbResult );
                  }
                  else
                  {
                     string str = "String table failed to load";
                     Log( str );
                  }
                  
                  wasHandled = true;
               }
               break;
            case QueryType_LoadWeblinks:
               {
                  if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                  {
                     HandleWeblinks( dbResult );
                  }
                  else
                  {
                     string str = "Config table failed to load";
                     Log( str );
                  }
                 
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
                  
                  wasHandled = true;
               }
               break;
            case QueryType_Hack:
               {
                  if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                  {
                     HackResult( dbResult );
                  }
                  
                  wasHandled = true;
               }
               break;
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
            case QueryType_ResetPasswords:
               {
                  if( dbResult->successfulQuery == true && dbResult->bucket.bucket.size() > 0 )
                  {
                     HandleResetPassword( dbResult );
                  }
                  
                  wasHandled = true;
               }
               break;
         }
         if( wasHandled == true )
         {
            delete dbResult;
         }
      }
       
   }
   return true;
}

//---------------------------------------------------------------

void     StatusUpdate::PreloadLanguageStrings()
{
   if( m_hasLoadedStringTable == false )
   {
      time_t testTimer;
      time( &testTimer );
      int timeToTest = 10;
      if( difftime( testTimer, m_newAccountCreationTimer ) >= timeToTest )// wait until after we launch 10 seconds
      {
         m_newAccountCreationTimer = testTimer;// so that we don't check too often

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_LoadStrings;

         dbQuery->query = "SELECT * FROM string where category='account' or category='reset_password'";
         AddQueryToOutput( dbQuery );

         LogMessage( LOG_PRIO_INFO, "Accounts::PreloadLanguageStrings\n" );
      }
   }
}

//---------------------------------------------------------------

void     StatusUpdate::PreloadWeblinks()
{
   if( m_hasLoadedWeblinks == false )
   {
      time_t testTimer;
      time( &testTimer );
      int timeToTest = 10;
      if( difftime( testTimer, m_newAccountCreationTimer ) >= timeToTest )// wait until after we launch 10 seconds
      {
         m_hasLoadedWeblinks = true; // only ever load once.
         // because this table may not exist, we will set the default here

         m_linkToAccountCreated = "http://accounts.playdekgames.com/account_created.php";
         m_linkToResetPasswordConfirm = "http://accounts.playdekgames.com/reset_password_confirm.php";

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_LoadWeblinks;

         dbQuery->query = "SELECT * FROM config where category='Mber'";
         AddQueryToOutput( dbQuery );

         LogMessage( LOG_PRIO_INFO, "Accounts::PreloadWeblinks\n" );
      }
   }
}

//---------------------------------------------------------------

void     StatusUpdate::SaveStrings( const PacketDbQueryResult* dbResult )
{
   cout << "strings saved :" << dbResult->bucket.bucket.size() << endl;

   StringTableParser              enigma( dbResult->bucket );
   StringTableParser::iterator    it = enigma.begin();
   int count = 0;
   
   while( it != enigma.end() )
   {
      m_hasLoadedStringTable = true;
      StringTableParser::row     row = *it++;

      string id =                row[ StringsTable::Column_id ];
      string stringName =        row[ StringsTable::Column_string ];
      string replacementstring = row[ StringsTable::Column_replaces ];

      stringhash lookupHash = GenerateUniqueHash( stringName );
      m_stringsTable.insert( StringTableLookupPair( lookupHash, row ) );
      count ++;
      if( count == 53 )
      {
         id = id;
      }

      if( replacementstring.size() )
      {
         stringhash replacementHash = GenerateUniqueHash( replacementstring );
         m_replacemetStringsLookup.insert( ReplacementPair( replacementHash, lookupHash ) );
      }
   }
}

//---------------------------------------------------------------

void     StatusUpdate::HandleWeblinks( const PacketDbQueryResult* dbResult )
{
   cout << "config saved :" << dbResult->bucket.bucket.size() << endl;

   ConfigParser              enigma( dbResult->bucket );
   ConfigParser::iterator    it = enigma.begin();

   string begin = "http://accounts.playdekgames.com";
   string middle = "";
   string accountCreated = "account_created.php";
   string resetPassword = "reset_password_confirm.php";

   string pathToConfirmationEmailFile;
   string pathToResetPasswordEmailFile;
   
   while( it != enigma.end() )
   {
      StringTableParser::row     row = *it++;

      string key =          row[ ConfigTable::Column_key ];
      string value =        row[ ConfigTable::Column_value ];

      if( key == "user_account.web_root" )
      {
         begin = value;
      }
      else if( key == "user_account.web_subdir" )
      {
         middle = value;
      }
      else if( key == "user_account.web_account_created" )
      {
         accountCreated = value;
      }
      else if( key == "user_account.web_reset_password_confirmed" )
      {
         resetPassword = value;
      }
      else if( key == "user_account.account_created.confirmation_email_path" )
      {
         pathToConfirmationEmailFile = value;
      }
      else if( key == "user_account.password_reset.reset_email_path" )
      {
         pathToResetPasswordEmailFile = value;
      }
   }

   //--------------------------------------------------
   
   // assemble the path
   m_linkToAccountCreated = begin;
   m_linkToResetPasswordConfirm = begin;

   char character = *m_linkToAccountCreated.rbegin();

   if( character != '/' )
   {
      m_linkToAccountCreated += "/";
   }
   if( middle.size() > 0 )
   {
      character = *middle.rbegin();
      m_linkToAccountCreated += middle;
      if( character != '/' )
      {
         m_linkToAccountCreated += "/";
      }
   }
   if( accountCreated.size() < 3 )
   {
      string str = "Config table does not contain a useful value for 'user_account.web_account_created'; db query failed";
      Log( str );
   }
   assert( accountCreated.size() > 2 );// minimal string size
   m_linkToResetPasswordConfirm = m_linkToAccountCreated;
   m_linkToAccountCreated += accountCreated;
   m_linkToResetPasswordConfirm += resetPassword;

   if( pathToConfirmationEmailFile.size() )
   {
      m_confirmationEmailTemplate = OpenAndLoadFile( pathToConfirmationEmailFile );
   }
   else
   {
      cout << "Error email: pathToConfirmationEmailFile not specified" << endl;
   }
   if( m_confirmationEmailTemplate.size() == 0 )
   {
      cout << "Error email: confirmation email file not found" << endl;
   }

   if( pathToResetPasswordEmailFile.size() )
   {
      m_passwordResetEmailTemplate = OpenAndLoadFile( pathToResetPasswordEmailFile );
   }
   else
   {
      cout << "Error email: pathToResetPasswordEmailFile not specified" << endl;
   }
   if( m_passwordResetEmailTemplate.size() == 0 )
   {
      cout << "Error email: reset password email file not found" << endl;
   }
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

      if( uuid.size() == 0 || uuid == "NULL" )
      {
         //continue; // wait until next round until it has the uuid in place
         //uuid = GenerateUUID( GetCurrentMilliseconds() );
         UpdateUuidForUser( "0", true, id );
         continue;
      }

      //http://dev.mysql.com/doc/refman/5.0/en/insert-on-duplicate.html

      
      string name =              row[ TableUserTempNewUser::Column_name ];
      string email =             row[ TableUserTempNewUser::Column_email ];
      string userId =            row[ TableUserTempNewUser::Column_user_id ];
      string timeCreated =       row[ TableUserTempNewUser::Column_time_created ];

      //string uuid =              row[ TableUserTempNewUser::Column_uuid ]; //<< must be created
      string gamekitHash =       row[ TableUserTempNewUser::Column_gamekit_hash ];
      string passwordHash =      row[ TableUserTempNewUser::Column_user_pw_hash ];
      string lowerCaseUserName = row[ TableUserTempNewUser::Column_user_name_match ];
      string languageId =        row[ TableUserTempNewUser::Column_language_id ];

      string query = "INSERT INTO users (user_id, user_name, user_name_match, user_pw_hash, user_email, user_gamekit_hash, user_creation_date, uuid, active, language_id) VALUES ('";
      query += userId;
      query += "', '";
      query += name;
      query += "', '";
      query += lowerCaseUserName;
      query += "', ";
      query += passwordHash;
      query += ", '";
      query += email;
      query += "', '";
      query += gamekitHash;
      query += "', '";
      query += timeCreated;
      query += "', '";
      query += uuid;
      query += "', ";
      query += "'1',";
      query += languageId;      
      query += ") ON DUPLICATE KEY UPDATE user_name='";
      query += name;
      query += "', user_name_match='";
      query += lowerCaseUserName;
      query += "', user_pw_hash='";
      query += passwordHash;
      query += "', user_email='";
      query += email;
      query += "', user_gamekit_hash='";
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
   }
}

//---------------------------------------------------------------

void     StatusUpdate::CheckForNewAccounts()
{
   if( isMailServiceEnabled == false )
      return;

   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_newAccountCreationTimer ) >= m_newAccountTimeoutSeconds ) /// only check once every 55 seconds
   {
      //cout << "CheckForNewAccounts..." << endl;
      m_newAccountCreationTimer = testTimer;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_UserCheckForNewAccount;
      //dbQuery->isFireAndForget = false;

      dbQuery->query = "SELECT * FROM user_temp_new_user WHERE was_email_sent='0' AND flagged_as_invalid='0'";

      AddQueryToOutput( dbQuery );

      //LogMessage( LOG_PRIO_INFO, "Accounts::CheckForNewAccounts\n" );
   }
}


//---------------------------------------------------------------

void     StatusUpdate::CheckForResetPassword()
{
   if( isMailServiceEnabled == false )
      return;

   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_resetPasswordEmailTimer ) >= m_resetPasswordEmailTimeoutSeconds ) 
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_ResetPasswords;

      dbQuery->query = "SELECT reset_password_keys.id, users.user_email, users.language_id, reset_password_keys.reset_key FROM users JOIN reset_password_keys ON reset_password_keys.user_account_uuid = users.uuid WHERE reset_password_keys.was_email_sent=0";

      AddQueryToOutput( dbQuery );

      //LogMessage( LOG_PRIO_INFO, "Accounts::CheckForNewAccounts\n" );
   }
}


//---------------------------------------------------------------

void     StatusUpdate::HandleResetPassword( const PacketDbQueryResult* dbResult )
{
   //cout << "HandleNewAccounts..." << endl;
   //LogMessage( LOG_PRIO_INFO, "HandleNewAccounts...\n" );
   PasswordResetParser              enigma( dbResult->bucket );
   PasswordResetParser::iterator    it = enigma.begin();
   
   while( it != enigma.end() )
   {
      PasswordResetParser::row      row = *it++;
      string columnId =             row[ EmailAddressesOfPasswordsToResetTable::Column_id ];
      string email =                row[ EmailAddressesOfPasswordsToResetTable::Column_email ];
      string languageString =        row[ EmailAddressesOfPasswordsToResetTable::Column_language_id ];
      string Column_reset_key =     row[ EmailAddressesOfPasswordsToResetTable::Column_reset_key ];
      
      int languageId = 1;// english
      if( languageString != "NULL" )
      {
         languageId = boost::lexical_cast<int>( languageString );
      }

      if( IsValidEmailAddress( email ) )
      {
         string subjectText = GetString( "email.reset_password.subject", languageId ); //"Confirmation email";
         string bodyText = GetString( "email.reset_password.body_text", languageId );//"Thank you for signing up with Playdek. Click this link to confirm your new account.";

         string linkPath = m_linkToResetPasswordConfirm;
         linkPath += "?key=";
         linkPath += Column_reset_key;

         if( m_passwordResetEmailTemplate.size() )
         {
            string bodyText = m_passwordResetEmailTemplate;
            map< string, string > specialStrings;
            specialStrings.insert( pair<string, string> ( "link-to-confirmation", linkPath ) );
            ReplaceAllLookupStrings( bodyText, languageId, specialStrings );
         }
         else
         {
            bodyText += "<a href='";
            bodyText += linkPath;
            bodyText += "'>Playdek.com</a>";
         }

         // update playdek.user_temp_new_user set was_email_sent=was_email_sent+1, lookup_key='lkjasdfhlkjhadfs' where id='4' ;
         SendConfirmationEmail( email.c_str(), resetPasswordEmailAddress, mailServer, bodyText.c_str(), subjectText.c_str(), "Playdek.com", linkPath.c_str() );

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_ResetPasswords;

         dbQuery->query = "UPDATE reset_password_keys SET was_email_sent=1 WHERE reset_key='";
         dbQuery->query += Column_reset_key;
         dbQuery->query += "'";

         AddQueryToOutput( dbQuery );
      }
   }

   time( &m_resetPasswordEmailTimer );
}

//---------------------------------------------------------------

void     StatusUpdate::LookForFlaggedAutoCreateAccounts()
{
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

      //LogMessage( LOG_PRIO_INFO, "Accounts::LookForFlaggedAutoCreateAccounts\n" );
   }
}

//---------------------------------------------------------------
