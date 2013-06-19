
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
const char* mailServer = "mail.playdekgames.com";


StatusUpdate::StatusUpdate( const string& serverName, U32 serverId ) : Threading::CChainedThread < BasePacket* >( false, DefaultSleepTime, false ),
                  m_hasLoadedStringTable( false ),
                  m_hasLoadedWeblinks( false ),
                  m_checkOnBlankUuidTimeoutSeconds( 10 ),
                  m_newAccountTimeoutSeconds( 48 ),
                  m_checkOnautoCreateTimeoutSeconds( 60 )
{
   SetSleepTime( 30 );
   time( &m_newAccountCreationTimer );
   time( &m_checkOnBlankUuidTimer );
   time( &m_checkOnautoCreateTimer );
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
      string message = "Accounts::HandleBlankUUIDs some UUIDs were added\n";
      LogMessage( LOG_PRIO_ERR, message.c_str() );
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

   string newUuid = GenerateUUID( GetCurrentMilliseconds() );

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

void     StatusUpdate::ReplaceAllLookupStrings( string& bodyText, int languageId )
{
   vector<string> dictionary = CreateDictionary( bodyText );
   map< string, string > replacements;

   vector<string>::iterator it = dictionary.begin(); 
   while( it != dictionary.end() )
   {
      string lookupString = *it++;

      const string actualString = GetString( lookupString, languageId );
      if( actualString.size() )
      {
         string replaceString = "%";
         replaceString += lookupString;
         replaceString += "%";

        /* stringhash hash = GenerateUniqueHash( lookupString );

         map< stringhash, stringhash >::iterator matchedString = m_replacemetStringsLookup.find( hash );

         if( matchedString == m_replacemetStringsLookup.end() )
         {
            cout << "unfound string in lookups: " << lookupString << endl;
            return;
         }

         GetString(*/
         /*stringhash sought = matchedString->second;
         StringTableLookup::iterator foundStringIter = m_stringsTable.find( sought );
         if( foundStringIter == m_stringsTable.end() )
         {
            cout << "unfound string in lookups: " << lookupString << endl;
            return;
         }
         const string& actualString = foundStringIter->second ;*/

         //bodyText.replace( lookupString, actualString );
         boost::replace_first( bodyText, replaceString, actualString );
      }
   }
}

//---------------------------------------------------------------

void     StatusUpdate::HandleNewAccounts( const PacketDbQueryResult* dbResult )
{
   cout << "HandleNewAccounts..." << endl;
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

      string subjectText = GetString( "email.new_account.welcome.subject", languageId ); //"Confirmation email";
      string bodyText = GetString( "email.new_account.welcome.body_text", languageId );//"Thank you for signing up with Playdek. Click this link to confirm your new account.";
      if( m_confirmationEmailTemplate.size() )
      {
         bodyText = m_confirmationEmailTemplate;
         ReplaceAllLookupStrings( bodyText, languageId );
      }
      

      string userLookupKey = GenerateUUID( GetCurrentMilliseconds() );
      userLookupKey += GenerateUUID( GetCurrentMilliseconds() ); // double long text

      string linkPath = m_linkToAccountCreated;
      linkPath += "?key=";
      linkPath += userLookupKey;

      if( IsValidEmailAddress( email ) )
      {
         // update playdek.user_temp_new_user set was_email_sent='1', lookup_key='lkjasdfhlkjhadfs' where id='4' ;
         sendConfirmationEmail( email.c_str(), newAccountEmailAddress, mailServer, bodyText.c_str(), subjectText.c_str(), "Playdek.com", linkPath.c_str() );

         // it is likely that the new user does not have a UUID yet so we will add it to both tables
         UpdateUuidForUser( userId, true, columnId );

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_UserCheckForNewAccount;
         dbQuery->isFireAndForget = true;

         string queryString = "UPDATE user_temp_new_user SET was_email_sent='1', lookup_key='";
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
         cout << message << endl;

         cout << "Time sent: ";
         PrintCurrentTime();
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
}


//---------------------------------------------------------------

int      StatusUpdate::CallbackFunction()
{
   if( isMailServiceEnabled == true )
   {
      FillInUserAccountUUIDs();
      CheckForNewAccounts();
      LookForFlaggedAutoCreateAccounts();
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
         PacketDbQueryResult* dbResult = static_cast<PacketDbQueryResult*>( packet );
         switch( dbResult->lookup )
         {
            cout << "Db query type:"<< dbResult->lookup << ", success=" << dbResult->successfulQuery << endl;
            case QueryType_UserCheckForNewAccount:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     string str = "New user accounts db query failed";
                     Log( str );
                     return false;
                  }

                  HandleNewAccounts( dbResult );
               }
               break;
            case QueryType_UserFindBlankUUID:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     string str = "Find users with blank UUID failed";
                     Log( str );
                     return false;
                  }

                  HandleBlankUUIDs( dbResult );
               }
               break;
            case QueryType_LoadStrings:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     string str = "String table failed to load";
                     Log( str );
                     return false;
                  }
                  SaveStrings( dbResult );
               }
               break;
            case QueryType_LoadWeblinks:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     string str = "Config table failed to load";
                     Log( str );
                     return false;
                  }
                  HandleWeblinks( dbResult );
               }
               break;
            case QueryType_AutoCreateUsers:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     //string str = "Config table failed to load";
                     //Log( str );
                     return false;
                  }
                  HandleAutoCreateAccounts( dbResult );
               }
               break;
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

         dbQuery->query = "SELECT * FROM string where category='account'";// || category='create_account'";
         AddQueryToOutput( dbQuery );
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

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = QueryType_LoadWeblinks;

         dbQuery->query = "SELECT * FROM config where category='Mber'";
         AddQueryToOutput( dbQuery );
      }
   }
}

//---------------------------------------------------------------

void     StatusUpdate::SaveStrings( const PacketDbQueryResult* dbResult )
{
   cout << "strings saved :" << dbResult->bucket.bucket.size() << endl;

   StringTableParser              enigma( dbResult->bucket );
   StringTableParser::iterator    it = enigma.begin();
   
   while( it != enigma.end() )
   {
      m_hasLoadedStringTable = true;
      StringTableParser::row     row = *it++;

      string id =                row[ StringsTable::Column_id ];
      string stringName =        row[ StringsTable::Column_string ];
      string replacementstring = row[ StringsTable::Column_replaces ];

      stringhash lookupHash = GenerateUniqueHash( stringName );
      m_stringsTable.insert( StringTableLookupPair( lookupHash, row ) );

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
   string end = "account_created.php";
   string pathToConfirmationEmailFile;
   
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
         end = value;
      }
      else if( key == "user_account.account_created.confirmation_email_path" )
      {
         pathToConfirmationEmailFile = value;
      }
   }

   //--------------------------------------------------
   
   // assemble the path
   m_linkToAccountCreated = begin;

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
   if( end.size() < 3 )
   {
      string str = "Config table does not contain a useful value for 'user_account.web_account_created'; db query failed";
      Log( str );
   }
   assert( end.size() > 2 );// minimal string size
   m_linkToAccountCreated += end;

   if( pathToConfirmationEmailFile.size() )
   {
      m_confirmationEmailTemplate = OpenAndLoadFile( pathToConfirmationEmailFile );
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
   }
}

//---------------------------------------------------------------

void     StatusUpdate::LookForFlaggedAutoCreateAccounts()
{
   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_checkOnautoCreateTimer ) >= m_checkOnautoCreateTimeoutSeconds ) /// only check once every 60 seconds
   {
      cout << "LookForFlaggedAutoCreateAccounts..." << endl;
      m_checkOnautoCreateTimer = testTimer;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_AutoCreateUsers;

      dbQuery->query = "SELECT * FROM user_temp_new_user WHERE flagged_auto_create='1'";

      AddQueryToOutput( dbQuery );
   }
}

//---------------------------------------------------------------
