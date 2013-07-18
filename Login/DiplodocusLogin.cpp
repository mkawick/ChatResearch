// DiplodocusLogin.cpp

#include "DiplodocusLogin.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Utils/Utils.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"

#include "../NetworkCommon/Logging/server_log.h"

#include <boost/lexical_cast.hpp>


//////////////////////////////////////////////////////////

DiplodocusLogin::DiplodocusLogin( const string& serverName, U32 serverId )  : Diplodocus< KhaanLogin >( serverName, serverId, 0, ServerType_Login ), 
                  m_updateGatewayConnections( false )
{
   SetSleepTime( 30 );
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
               LogUserIn( login->username, login->password, login->loginKey, login->gameProductId, userConnectionId );
            }
            break;
         case PacketLogin::LoginType_Logout:
            {
               PacketLogout* logout = static_cast<PacketLogout*>( actualPacket );
               UpdateLastLoggedOutTime( userConnectionId );
               LogUserOut( userConnectionId, logout->wasDisconnectedByError );
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

bool     DiplodocusLogin::FindUserAlreadyInGame( const string& username, U8 gameProductId )
{
   UserConnectionMapIterator it = m_userConnectionMap.begin();
   while( it != m_userConnectionMap.end() )
   {
      UserConnectionPair pairObj = *it++;
      ConnectionToUser& conn = pairObj.second;
      if( conn.gameProductId == gameProductId &&// optimized for simplest test first
         conn.username == username )
      {
         return true;
      }
   }
   return false;
}

bool     DiplodocusLogin::LogUserIn( const string& username, const string& password, const string& loginKey, U8 gameProductId, U32 connectionId )
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

   // Before we add the user, let's verify that s/he isn't already logged in with a different connectionId. Storing this in a map
   // makes sense, but it's overkill for now.

   if( FindUserAlreadyInGame( username, gameProductId ) == true )
   {
      // should we boot this user for hacking? Or is it bad code?
      Log( "Second login from the same product attempt was made", 4 );
      Log( username.c_str(), 4 );
      ForceUserLogoutAndBlock( connectionId );
      return false;
   }

   ConnectionToUser conn( username, password, loginKey );
   conn.gameProductId = gameProductId;
   m_userConnectionMap.insert( UserConnectionPair( connectionId, conn ) );
   //*********************************************************************************
   // perhaps some validation here is in order like is this user valid based on the key
   //*********************************************************************************

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           connectionId;
   dbQuery->lookup =       QueryType_UserLoginInfo;
   dbQuery->meta =         username;
   dbQuery->serverLookup = gameProductId;

   string queryString = "SELECT * FROM users as user WHERE user_email='%s' and user_pw_hash='" ;
   queryString += boost::lexical_cast< string >( password );
   queryString += "'";
   dbQuery->query = queryString;
   dbQuery->escapedStrings.insert( username );
   
   return AddQueryToOutput( dbQuery );
}
//---------------------------------------------------------------

bool     DiplodocusLogin::LogUserOut( U32 connectionId, bool wasDisconnectedByError )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =              connectionId;
      dbQuery->lookup =          QueryType_UserLoginInfo;
      dbQuery->isFireAndForget = true;// no result is needed
      
      ConnectionToUser& connection = it->second;
      string queryString = "UPDATE users AS user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString += connection.userUuid;
      queryString += "'";
      dbQuery->query =           queryString;

      SendLoginStatusToOtherServers( it->second.username, it->second.userUuid, connectionId, it->second.gameProductId, it->second.lastLoginTime, connection.active, connection.email, connection.passwordHash, connection.id, false, wasDisconnectedByError );

      if( wasDisconnectedByError == false )
      {
         PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
         wrapper->connectionId = connectionId;
         PacketLogoutToClient* logout = new PacketLogoutToClient();
         logout->username = it->second.username;// just for loggin purposes
         logout->uuid = it->second.userUuid;

         wrapper->pPacket = logout;
         SendPacketToGateway( wrapper, connectionId );
      }
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
   string email;
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   bool           active = false;
   string   passwordHash = "0";
   string   userId = "0";

   if( it != m_userConnectionMap.end() )
   {
      ConnectionToUser& user = it->second;
      username = user.username;
      uuid = user.userUuid;
      lastLoginTime = user.lastLoginTime;
      user.status = ConnectionToUser::LoginStatus_Invalid;
      active =          user.active;
      passwordHash =    user.passwordHash;
      userId =          user.id;
      email =           user.email;
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
   SendLoginStatusToOtherServers( username, uuid, connectionId, it->second.gameProductId, lastLoginTime, active, email, passwordHash, userId, false, false );

   return true;
}

//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::SendListOfGamesToGameServers( U32 connectionId, const KeyValueVector& kvArray )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId ); // user may have disconnected waiting for the db.
   if( it != m_userConnectionMap.end() )
   {
      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainedInterface<BasePacket*>* outputPtr = (*itOutputs).m_interface;

         PacketListOfGames* packetToSend = new PacketListOfGames;
         packetToSend->games = kvArray;// potentially costly.
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
      dbQuery->id =              connectionId ;
      dbQuery->lookup =          QueryType_UpdateLastLoggedInTime;
      dbQuery->isFireAndForget = true;// no result is needed

      string queryString = "UPDATE users AS user SET user.last_login_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString += it->second.userUuid;
      queryString += "'";
      dbQuery->query =           queryString;

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
      dbQuery->id =              connectionId ;
      dbQuery->lookup =          QueryType_UpdateLastLoggedOutTime;
      dbQuery->isFireAndForget = true;// no result is needed

      string queryString = "UPDATE users AS user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
      queryString += it->second.userUuid;
      queryString += "'";
      dbQuery->query =           queryString;

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

   ConnectionToUser& connection = it->second;

   const string&  username =        connection.username;
   const string&  userUuid =        connection.userUuid;
   const string&  email =           connection.email;
   const string&  lastLoginTime =   connection.lastLoginTime;
   bool           active =          connection.active;
   const string&  passwordHash =    connection.passwordHash;
   const string&  userId =          connection.id;
   U8 gameProductId =               connection.gameProductId;

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


   SendPacketToGateway( wrapper, connectionId );

   RequestListOfGames( connectionId, userUuid );

   //This is where we inform all of the games that the user is logged in.

   return SendLoginStatusToOtherServers( username, userUuid, connectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, true, false );
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

bool  DiplodocusLogin::RequestListOfGames( U32 connectionId, const string& userUuid )
{
   return false;// not working this way anymore
   if( userUuid.size() && connectionId != 0 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id =     connectionId ;
      dbQuery->lookup = QueryType_UserListOfGame;

      string queryString = "SELECT game.uuid, game.name FROM game INNER JOIN user_join_game AS user_game ON game.uuid=user_game.game_uuid WHERE user_game.user_uuid = '";
      queryString += userUuid;
      queryString += "'";
      dbQuery->query =  queryString;

      return AddQueryToOutput( dbQuery );
   }
   return false;
}

//---------------------------------------------------------------

bool  DiplodocusLogin::SendLoginStatusToOtherServers( const string& username, const string& userUuid, U32 connectionId, U8 gameProductId, const string& lastLoginTime, bool isActive, const string& email, const string& passwordHash, const string& userId, bool isLoggedIn, bool wasDisconnectedByError )
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
         prepareForUser->gameProductId = gameProductId;

         prepareForUser->active = isActive;
         prepareForUser->email= email;
         prepareForUser->userId = boost::lexical_cast<U32>( userId );
         prepareForUser->password = passwordHash;

         packetToSend = prepareForUser;
         
      }
      else
      {
         PacketPrepareForUserLogout* logout = new PacketPrepareForUserLogout;
         logout->connectionId = connectionId;
         logout->wasDisconnectedByError = wasDisconnectedByError;

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
                  string id =               row[ TableUser::Column_id ];
                  string name =             row[ TableUser::Column_name ];
                  string uuid =             row[ TableUser::Column_uuid ];
                  string email =            row[ TableUser::Column_email ];
                  string passwordHash =     row[ TableUser::Column_password_hash ];
                  // note that we are using logout for our last login time.
                  string lastLoginTime =    row[ TableUser::Column_last_logout_time ];
                  string isActive =         row[ TableUser::Column_active];

                  it->second.username = name;
                  it->second.userUuid = uuid;
                  it->second.status = ConnectionToUser::LoginStatus_LoggedIn;
                  it->second.lastLoginTime = lastLoginTime;
                  it->second.email = email;
                  it->second.id =   id;
                  it->second.passwordHash = passwordHash;
                  it->second.active = boost::lexical_cast<bool>( isActive );

                  it->second.gameProductId = dbResult->serverLookup;

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
         }
      }
   }
   return true;
}

//---------------------------------------------------------------

int      DiplodocusLogin::CallbackFunction()
{
   SendServerIdentification();

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
