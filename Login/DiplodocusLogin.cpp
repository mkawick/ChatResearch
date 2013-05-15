#include "DiplodocusLogin.h"
#include "../NetworkCommon/Utils/TableWrapper.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"


//////////////////////////////////////////////////////////

DiplodocusLogin::DiplodocusLogin()  : Diplodocus< KhaanLogin >( "login", ServerType_Login ), m_updateGatewayConnections( false )
{
}

//---------------------------------------------------------------

bool     DiplodocusLogin::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   // if packet is a login or a logout packet we'll handle it, otherwise.. no deal.
   // all packets coming in should be from the gateway only 

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
               login->loginKey;
               LogUserIn( login->username, login->password, login->loginKey, userConnectionId );
            }
            break;
         case PacketLogin::LoginType_Logout:
            {
               PacketLogout* logout = static_cast<PacketLogout*>( actualPacket );
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
      ChainedInterface* outputPtr = (*itOutputs).m_interface;
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

   string queryString = "SELECT * FROM USER where name='" ;
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
      
      string queryString = "UPDATE user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
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

bool  DiplodocusLogin::UpdateLastLoggedInTime( U32 connectionId )
{
   UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
   if( it != m_userConnectionMap.end() )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = connectionId ;
      dbQuery->lookup = QueryType_UserLoginInfo;
      dbQuery->isFireAndForget = true;// no result is needed

      string queryString = "UPDATE user SET user.last_login_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
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

      string queryString = "UPDATE user SET user.last_logout_timestamp=CURRENT_TIMESTAMP WHERE uuid = '";
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

   SendPacketToGateway( wrapper, connectionId );

   return SendLoginStatusToOtherServers( username, userUuid, connectionId, lastLoginTime, true );
}

//---------------------------------------------------------------

bool  DiplodocusLogin::SendLoginStatusToOtherServers( const string& username, const string& userUuid, U32 connectionId, const string& lastLoginTime, bool isLoggedIn )
{
   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainedInterface* outputPtr = (*itOutputs).m_interface;

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
         PacketDbQueryResult* dbResult = reinterpret_cast<PacketDbQueryResult*>( packet );
         U32 connectionId = dbResult->id;
         UserConnectionMapIterator it = m_userConnectionMap.find( connectionId );
         if( it == m_userConnectionMap.end () )
         {
            string str = "Login server: Something seriously wrong where the db query came back from the server but no record.. ";
            Log( str, 4 );
            str = "was apparently requested or at least it was not stored properly: username was :";
            str += dbResult->meta;
            Log( str, 4 );
            return false;
         }

         switch( dbResult->lookup )
         {
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
                  // note that we are using logout for our last login time.
                  string lastLoginTime =    row[ TableUser::Column_last_logout_time ];

                  it->second.userUuid = uuid;
                  it->second.status = ConnectionToUser::LoginStatus_LoggedIn;
                  it->second.lastLoginTime = lastLoginTime;

                  SuccessfulLogin( connectionId );
                  UpdateLastLoggedInTime( dbResult->id ); // update the user logged in time
                  

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
   if( m_updateGatewayConnections == false )
      return 0;

   m_mutex.lock();
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainedInterface* inputPtr = itInputs->m_interface;
      InputChainType* connection = static_cast< InputChainType* >( inputPtr );
      connection->Update();
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
      ChainedInterface* inputPtr = itInputs->m_interface;
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
