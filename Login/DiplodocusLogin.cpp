// DiplodocusLogin.cpp

#include "DiplodocusLogin.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"

#include "../NetworkCommon/Logging/server_log.h"

//#include "email.h"
#include <boost/lexical_cast.hpp>

const char* newAccountEmailAddress = "account_create@playdekgames.com";
const char* mailServer = "mail.playdekgames.com";

const bool isMailServiceEnabled = false;

//////////////////////////////////////////////////////////

DiplodocusLogin::DiplodocusLogin( const string& serverName, U32 serverId )  : Diplodocus< KhaanLogin >( serverName, serverId, ServerType_Login ), 
                  m_updateGatewayConnections( false ),
                  m_hasLoadedStringTable( false ),
                  m_hasLoadedWeblinks( false ),
                  m_checkOnBlankUuidTimeoutSeconds( 10 ),
                  m_newAccountTimeoutSeconds( 30 )
{
   SetSleepTime( 30 );
   time( &m_newAccountCreationTimer );
   time( &m_checkOnBlankUuidTimer );
   LogOpen();
   LogMessage( LOG_PRIO_INFO, "Login::Login server created" );
   cout << "Login::Login server created" << endl;
}

//---------------------------------------------------------------

bool     DiplodocusLogin::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   // if packet is a login or a logout packet we'll handle it, otherwise.. no deal.
   // all packets coming in should be from the gateway only 

   LogMessage( LOG_PRIO_INFO, "Login::Data in" );

   cout << "Login::Data in" << endl;

   if( packet->packetType != PacketType_GatewayWrapper )
   {
      string text = "Login server: received junk packets. Type: ";
      text += packet->packetType;
      Log( text, 4 );
      return false;
   }

   PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper * >( packet );
   U32   userConnectionId = wrapper->connectionId;
   BasePacket* actualPacket = wrapper->pPacket;
   delete wrapper;

   switch( actualPacket->packetType )
   {
      case PacketType_Login:
      {
         switch( actualPacket->packetSubType )
         {
         case PacketLogin::LoginType_Login:
            {
               PacketLogin* login = static_cast<PacketLogin*>( actualPacket );
               //login->loginKey;
               LogUserIn( login->username, login->password, login->loginKey, userConnectionId );
            }
            break;
         case PacketLogin::LoginType_Logout:
            {
               //PacketLogout* logout = static_cast<PacketLogout*>( actualPacket );
               UpdateLastLoggedOutTime( userConnectionId );
               LogUserOut( userConnectionId );
            }
            break;
         }
         delete actualPacket;
         return true;
      }
   }
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusLogin::AddQueryToOutput( PacketDbQuery* packet )
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

bool     DiplodocusLogin::LogUserIn( const string& username, const string& password, const string& loginKey, U32 connectionId )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      // should we boot this user for hacking? Or is it bad code?
      Log( "Second attempt was made at login", 4 );
      //InformUserOfSuccessfulLogout();// someone is hacking our server
      ForceUserLogoutAndBlock( connectionId );
   }

   if( username.size() == 0 )
   {
      // should we boot this user for hacking? Or is it bad code?
      Log( "invalid attempt at login: username was empty", 4 );
      return false;
   }

   m_userConnectionMap.insert( UserConnectionPair( connectionId, ConnectionToUser( username, password, loginKey ) ) );
   //*********************************************************************************
   // perhaps some validation here is in order like is this user valid based on the key
   //*********************************************************************************

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = connectionId;
   dbQuery->lookup = QueryType_UserLoginInfo;
   dbQuery->meta = username;

   string queryString = "SELECT * FROM users as user WHERE user_name='" ;
   queryString += username;
   queryString += "'";
   dbQuery->query = queryString;
   
   return AddQueryToOutput( dbQuery );
}
//---------------------------------------------------------------

bool     DiplodocusLogin::LogUserOut( U32 connectionId )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = connectionId;
      dbQuery->lookup = QueryType_UserLoginInfo;
      dbQuery->isFireAndForget = true;// no result is needed
      
      string queryString = "UPDATE users AS user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString += it->second.userUuid;
      queryString += "'";
      dbQuery->query = queryString;

      SendLoginStatusToOtherServers( it->second.username, it->second.userUuid, connectionId, it->second.lastLoginTime, false );

      m_userConnectionMap.erase( it );

      return AddQueryToOutput( dbQuery );
   }
   else
   {
      Log( "Attempt to log user out failed: user record not found", 1 );
      return false;
   }
   return true;
}

//---------------------------------------------------------------

bool  DiplodocusLogin::ForceUserLogoutAndBlock( U32 connectionId )
{
   // send error to client
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->connectionId = connectionId;
   wrapper->pPacket = new PacketErrorReport( PacketErrorReport::ErrorType_UserBadLogin );
   SendPacketToGateway( wrapper, connectionId );

   string username;
   string uuid;
   string lastLoginTime;
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      username = it->second.username;
      uuid = it->second.userUuid;
      lastLoginTime = it->second.lastLoginTime;
      it->second.status = ConnectionToUser::LoginStatus_Invalid;
   }

   // now disconnect him/her
   wrapper = new PacketGatewayWrapper();
   {
      wrapper->connectionId = connectionId;

      PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
      loginStatus->username = username;
      loginStatus->uuid = uuid;

      loginStatus->wasLoginSuccessful = false;

      wrapper->pPacket = loginStatus;
   }
   SendPacketToGateway( wrapper, connectionId );
   SendLoginStatusToOtherServers( username, uuid, connectionId, lastLoginTime, false );

   return true;
}

//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::SendListOfGamesToGameServers( U32 connectionId, const KeyValueVector& kv_array )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId ); // user may have disconnected waiting for the db.
   if( it != m_userConnectionMap.end() )
   {
      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainedInterface<BasePacket*>* outputPtr = (*itOutputs).m_interface;

         PacketListOfGames* packetToSend = new PacketListOfGames;
         packetToSend->games = kv_array;// potentially costly.
         packetToSend->connectionId = connectionId;

         if( outputPtr->AddOutputChainData( packetToSend, m_chainId ) == false )
         {
            delete packetToSend;
         }
         itOutputs++;
      }
   }

   return false;
}

//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::UpdateLastLoggedInTime( U32 connectionId )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = connectionId ;
      dbQuery->lookup = QueryType_UserListOfGame;
      dbQuery->isFireAndForget = true;// no result is needed

      string queryString = "UPDATE users AS user SET user.last_login_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString += it->second.userUuid;
      queryString += "'";
      dbQuery->query = queryString;

      return AddQueryToOutput( dbQuery );
   }
   return false;
}


//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::UpdateLastLoggedOutTime( U32 connectionId )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = connectionId ;
      dbQuery->lookup = QueryType_UserLoginInfo;
      dbQuery->isFireAndForget = true;// no result is needed

      string queryString = "UPDATE users AS user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString += it->second.userUuid;
      queryString += "'";
      dbQuery->query = queryString;

      return AddQueryToOutput( dbQuery );
   }
   return false;
}

//------------------------------------------------------------------------------------------------

bool    DiplodocusLogin::SuccessfulLogin( U32 connectionId )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it == m_userConnectionMap.end() )
   {
      Log( "Login server: major problem with successfull long.", 4 );
      return false;
   }

   const string& username = it->second.username;
   const string& userUuid = it->second.userUuid;
   const string& email = it->second.email;
   const string& lastLoginTime = it->second.lastLoginTime;

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   {
      wrapper->connectionId = connectionId;

      PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
      if( it != m_userConnectionMap.end() )
      {
         loginStatus->username = username;
         loginStatus->uuid = userUuid;
      }
      loginStatus->wasLoginSuccessful = true;

      wrapper->pPacket = loginStatus;
   }

   //************************
   // TEST ONLY, please remove ASAP
   /*if( email.size() )
   {
      CreateAccount( username.c_str(), email.c_str(), it->second.password.c_str(), 75, 10 );
      //const char* linkAddr = "http://10.16.1.80/reset_password_confirm.php?key=8qFUEWV3aYtMzZP1ThmNB9b0C2OdeyLf";
      //sendConfirmationEmail( email.c_str(), newAccountEmailAddress, mailServer, "This is a test of email in the login server", "Playdek confirmation", "Click here to confirm", linkAddr );
   }*/
   // TEST ONLY, please remove ASAP
   //************************

   SendPacketToGateway( wrapper, connectionId );

   RequestListOfGames( connectionId, userUuid );

   return SendLoginStatusToOtherServers( username, userUuid, connectionId, lastLoginTime, true );
}

//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::CreateAccount( const char* username, const char* emailAddress, const char* password, int userId, int gameId )
{
   string tempHash = GenerateUUID( GetCurrentMilliseconds() );
   tempHash += GenerateUUID( GetCurrentMilliseconds() ); // double long text

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0 ;
   dbQuery->lookup = QueryType_UserCreateTempAccount;

   string queryString = "INSERT INTO user_temp_new_user ( user_name, user_password, user_email, lookup_key, user_id, game_id, was_email_sent ) VALUES ( '%s', '%s', '%s', '%s', '";
   queryString += boost::lexical_cast< string >( userId );
   queryString += "', '";
   queryString += boost::lexical_cast< string >( gameId );
   queryString += "', '1')";

   dbQuery->escapedStrings.insert( username );
   dbQuery->escapedStrings.insert( password );
   dbQuery->escapedStrings.insert( emailAddress );
   dbQuery->escapedStrings.insert( tempHash );


   dbQuery->query = queryString;

   bool success = AddQueryToOutput( dbQuery );

   string linkPath = m_linkToAccountCreated;
   linkPath += "?key=";
   linkPath += tempHash;

   //sendConfirmationEmail( emailAddress, newAccountEmailAddress, mailServer, "Thank you for signing up with Playdek. Click this link to confirm your new account.", "Confirmation email" , "Playdek.com", linkPath.c_str() );

   return success;
}

//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::RequestListOfGames( U32 connectionId, const string& userUuid )
{
   if( userUuid.size() && connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = connectionId ;
      dbQuery->lookup = QueryType_UserListOfGame;

      string queryString = "SELECT game.uuid, game.name FROM game INNER JOIN user_join_game AS user_game ON game.uuid=user_game.game_uuid WHERE user_game.user_uuid = '";
      queryString += userUuid;
      queryString += "'";
      dbQuery->query = queryString;

      return AddQueryToOutput( dbQuery );
   }
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusLogin::SendLoginStatusToOtherServers( const string& username, const string& userUuid, U32 connectionId, const string& lastLoginTime, bool isLoggedIn )
{
   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainedInterface<BasePacket*>*  outputPtr = (*itOutputs).m_interface;

      BasePacket* packetToSend = NULL;
      if( isLoggedIn )
      {
         PacketPrepareForUserLogin* prepareForUser = new PacketPrepareForUserLogin;
         prepareForUser->connectionId = connectionId;
         prepareForUser->username = username;
         prepareForUser->uuid = userUuid;
         prepareForUser->lastLoginTime = lastLoginTime;

         packetToSend = prepareForUser;
         
      }
      else
      {
         PacketPrepareForUserLogout* logout = new PacketPrepareForUserLogout;
         logout->connectionId = connectionId;

         packetToSend = logout;
      }

      if( outputPtr->AddOutputChainData( packetToSend, m_chainId ) == false )
      {
         delete packetToSend;
      }
      itOutputs++;
   }

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusLogin::AddOutputChainData( BasePacket* packet, U32 connectionId )
{
   // this should be a DB Query Response only. Lookup the appropriate gateway connection and push the login result back out.
   // If 
   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* dbResult = static_cast<PacketDbQueryResult*>( packet );
         U32 connectionId = dbResult->id;

         UserConnectionMapIterator it = m_userConnectionMap.end ();
         if( connectionId != 0 )
         {
            it = m_userConnectionMap.find( connectionId );
            if( it == m_userConnectionMap.end () )
            {
               string str = "Login server: Something seriously wrong where the db query came back from the server but no record.. ";
               Log( str, 4 );
               str = "was apparently requested or at least it was not stored properly: username was :";
               str += dbResult->meta;
               Log( str, 4 );
               return false;
            }
         }

         switch( dbResult->lookup )
         {
            cout << "Db query type:"<< dbResult->lookup << ", success=" << dbResult->successfulQuery << endl;
            case QueryType_UserLoginInfo:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     //assert( 0 );// begin teardown. Inform gateway that user is not available. Gateway will teardown the connection
                     // and send a reply to this game instance.
                     string str = "User not valid and db query failed, username: ";
                     str += it->second.username;
                     str += ", uuid: ";
                     str += it->second.userUuid;
                     Log( str, 4 );
                     ForceUserLogoutAndBlock( connectionId );
                     return false;
                  }
                  
                  UserTable            enigma( dbResult->bucket );
                  UserTable::row       row = *enigma.begin();
                  string name =             row[ TableUser::Column_name ];
                  string uuid =             row[ TableUser::Column_uuid ];
                  string email =            row[ TableUser::Column_email ];
                  // note that we are using logout for our last login time.
                  string lastLoginTime =    row[ TableUser::Column_last_logout_time ];

                  it->second.userUuid = uuid;
                  it->second.status = ConnectionToUser::LoginStatus_LoggedIn;
                  it->second.lastLoginTime = lastLoginTime;
                  it->second.email = email;

                  SuccessfulLogin( connectionId );
                  UpdateLastLoggedInTime( dbResult->id ); // update the user logged in time
                  

               }
               break;
            case QueryType_UserListOfGame:
               {
                  if( dbResult->successfulQuery == false || dbResult->bucket.bucket.size() == 0 )// no records found
                  {
                     string str = "List of games not valid db query failed, username: ";
                     str += it->second.username;
                     str += ", uuid: ";
                     str += it->second.userUuid;
                     Log( str, 4 );
                     ForceUserLogoutAndBlock( connectionId );
                     return false;
                  }

                  KeyValueVector             key_value_array;

                  SimpleGameTable            enigma( dbResult->bucket );
                  SimpleGameTable::iterator it = enigma.begin();
                  
                  while( it != enigma.end() )
                  {
                     SimpleGameTable::row       row = *it++;
                     string name =              row[ SimpleGame::Column_name ];
                     string uuid =              row[ SimpleGame::Column_uuid ];

                     key_value_array.push_back( KeyValueString ( uuid, name ) );
                  }

                  SendListOfGamesToGameServers( connectionId, key_value_array );
               }
               break;
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
         }
      }
   }
   return true;
}

//---------------------------------------------------------------

void     DiplodocusLogin::PreloadLanguageStrings()
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

         dbQuery->query = "SELECT * FROM string where category='login'";
         AddQueryToOutput( dbQuery );
      }
   }
}


//---------------------------------------------------------------

void     DiplodocusLogin::PreloadWeblinks()
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

void     DiplodocusLogin::SaveStrings( const PacketDbQueryResult* dbResult )
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

      stringhash lookupHash = GenerateUniqueHash( stringName );
      m_stringsTable.insert( StringTableLookupPair( lookupHash, row ) );
   }
}

//---------------------------------------------------------------

void     DiplodocusLogin::HandleWeblinks( const PacketDbQueryResult* dbResult )
{
   cout << "config saved :" << dbResult->bucket.bucket.size() << endl;

   ConfigParser              enigma( dbResult->bucket );
   ConfigParser::iterator    it = enigma.begin();

   string begin = "http://accounts.playdekgames.com";
   string middle = "";
   string end = "account_created.php";
   
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
}

//---------------------------------------------------------------

void     DiplodocusLogin::CheckForNewAccounts()
{
   if( isMailServiceEnabled == false )
      return;

   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_newAccountCreationTimer ) >= m_newAccountTimeoutSeconds ) /// only check once every 55 seconds
   {
      cout << "CheckForNewAccounts..." << endl;
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

void     DiplodocusLogin::FillInUserAccountUUIDs()
{
   if( isMailServiceEnabled == false )
      return;

   time_t testTimer;
   time( &testTimer );

   if( difftime( testTimer, m_checkOnBlankUuidTimer ) >= m_checkOnBlankUuidTimeoutSeconds ) 
   {
      cout << "FillInUserAccountUUIDs..." << endl;

      m_checkOnBlankUuidTimer = testTimer;

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_UserFindBlankUUID;

      dbQuery->query = "SELECT user_id FROM users where uuid is NULL LIMIT 30";

      AddQueryToOutput( dbQuery );
   }
}

//---------------------------------------------------------------

void     DiplodocusLogin::HandleBlankUUIDs( PacketDbQueryResult* dbResult )
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
      string message = "Login::HandleBlankUUIDs some UUIDs were added";
      LogMessage( LOG_PRIO_ERR, message.c_str() );
      //cout << message << endl;
   }
}

//---------------------------------------------------------------

void     DiplodocusLogin::UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId )
{
   if( ( userId.size() == 0 || userId == "0" ) && ( columnId.size() == 0 || columnId == "0" ) )
   {
      string message = "Login::UpdateUuidForUser userId is null";
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

string   DiplodocusLogin::GetString( const string& stringName, int languageId )
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

void     DiplodocusLogin::HandleNewAccounts( const PacketDbQueryResult* dbResult )
{
  /* cout << "HandleNewAccounts..." << endl;
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
   }*/

}

//---------------------------------------------------------------

int      DiplodocusLogin::CallbackFunction()
{
   SendServerIdentification();

   if( isMailServiceEnabled == true )
   {
      FillInUserAccountUUIDs();
      CheckForNewAccounts();
      PreloadLanguageStrings();
      PreloadWeblinks();
   }

   if( m_updateGatewayConnections == false )
      return 0;

   m_mutex.lock();
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainedInterface<BasePacket*>*  inputPtr = itInputs->m_interface;
      InputChainType* connection = static_cast< InputChainType* >( inputPtr );
      connection->Update();
      itInputs++;
   }
   m_updateGatewayConnections = false;

   m_mutex.unlock();
   return 1;
}

//---------------------------------------------------------------

bool     DiplodocusLogin::SendPacketToGateway( BasePacket* packet, U32 connectionId )
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainedInterface<BasePacket*>* inputPtr = itInputs->m_interface;
      InputChainType* connection = static_cast< InputChainType* >( inputPtr );
      if( connection->AddOutputChainData( packet, connectionId ) == true )
      {
         //m_clientsNeedingUpdate.push_back( connection->GetConnectionId() );
         m_updateGatewayConnections = true;
         return true;
      }
   }

   return false;
}
//---------------------------------------------------------------
//////////////////////////////////////////////////////////
