// DiplodocusContact.cpp
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/ContactPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "DiplodocusContact.h"

#include <iostream>
using namespace std;
#include <time.h>

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusContact::DiplodocusContact( const string& serverName, U32 serverId ): Diplodocus< KhaanContact >( serverName, serverId, 0,  ServerType_Contact )/*, 
                        //m_initializationStage( InitializationStage_Begin ),
                        m_queryPeriodicity( 10000 ),
                        m_isExecutingQuery( false )*/
{
    //time( &m_lastTimeStamp );
}

//---------------------------------------------------------------

void     DiplodocusContact::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_serverId, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   //khaan->AddOutputChainData( packet, 0 );
   

   InputChainType* localKhaan = static_cast< InputChainType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   //m_clientsNeedingUpdate.push_back( localKhaan->GetChainedId() );
   m_serversNeedingUpdate.push_back( localKhaan->GetServerId() );
}

//---------------------------------------------------------------

bool     DiplodocusContact::AddInputChainData( BasePacket* packet, U32 connectionId )
{
  /* packets handled

      PacketType_Contact

      invite friend -> send notification to chat server
      search friends
      clear invite -> send notification to chat server
      accept invite -> send notification to chat server
      remove friend -> send notification to chat server?
      get user profile info
      get friend profile info
      modify user profile  -> push to all friends? Probably not.*/

   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );
      return HandleCommandFromGateway( packet, connectionId );
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32 connectionIdToUse = wrapper->connectionId;
      connectionIdToUse = connectionIdToUse;

      if( unwrappedPacket->packetType == PacketType_Contact )
      {
         UserContactMapIterator found = m_users.find( connectionId );
         if( found == m_users.end() )
         {
            SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Contact_BadLoginKey );
            return false;
         }

         PacketContact* packetContact = static_cast< PacketContact* >( unwrappedPacket );
         bool result = found->second.HandleRequestFromClient( packetContact );
         result = result;
        
         PacketCleaner cleaner( packet );
         return true;
      }
      else
      {
         assert( 0 );
      }

      delete unwrappedPacket;
   }
   return false;
}

//---------------------------------------------------------------

 UserContact* DiplodocusContact::GetUser( U32 userId )
{
   UserIdToContactMapIterator it = m_userLookupById.find( userId );
   if( it == m_userLookupById.end() )
      return NULL;

   UserContactMapIterator found = m_users.find( it->second );
   if( found == m_users.end() )
      return NULL;

   return &found->second;
}

//---------------------------------------------------------------

UserContact* DiplodocusContact::GetUserByUsername( const string& userName )
{
   UserContactMapIterator it = m_users.begin(); //find( it->second );
   while( it != m_users.end() )
   {
      if( it->second.GetUserInfo().userName == userName )
         return &it->second;
      it++;
   }

   return NULL;
}


//---------------------------------------------------------------

UserContact* DiplodocusContact::GetUserByUuid( const string& uuid )
{
   UserContactMapIterator it = m_users.begin(); //find( it->second );
   while( it != m_users.end() )
   {
      if( it->second.GetUserInfo().uuid == uuid )
         return &it->second;
      it++;
   }

   return NULL;
}

//---------------------------------------------------------------


bool     DiplodocusContact::HandleCommandFromGateway( BasePacket* packet, U32 connectionId )
{
   assert( 0 ); // unfinished
   return false;
}


//---------------------------------------------------------------


bool  DiplodocusContact::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   //bool success = false;

   if( unwrappedPacket->packetType == PacketType_Login )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( unwrappedPacket ) );
         return true;
      }
   }
   else if( unwrappedPacket->packetType == PacketType_Contact)
   {
      return HandlePacketRequests( static_cast< PacketContact* >( packet ), connectionId );
   }

   return false;
}

//---------------------------------------------------------------

// make sure to follow the model of the account server regarding queries. Look at CreateAccount
// 
bool     DiplodocusContact::AddOutputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      if( m_connectionIdGateway == 0 )
         return false;

      Threading::MutexLock locker( m_mutex );

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanContact* khaan = static_cast< KhaanContact* >( interfacePtr );
         if( khaan->GetServerId() == m_connectionIdGateway )
         {
            khaan->AddOutputChainData( packet );
            //khaan->Update();// the gateway may not have a proper connection id.

            m_serversNeedingUpdate.push_back( khaan->GetServerId() );
            return true;
         }
      }
      return false;
   }

   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         //bool wasHandled = false;
         PacketDbQueryResult* dbResult = static_cast<PacketDbQueryResult*>( packet );
         return HandleDbQueryResult( dbResult );
      }
   }

   return false;
}

//---------------------------------------------------------------

bool     DiplodocusContact::HandleDbQueryResult( PacketDbQueryResult* dbResult )
{
   //bool success = false;

   if( dbResult->id )
   {
      U32 connectionId = dbResult->id;

      UserContactMapIterator it = m_users.find( connectionId );
      if( it == m_users.end() )
      {
         return false;
      }
      else
      {
         cout << "Db query type:"<< dbResult->lookup << " handed off to UserContact #" << connectionId << endl;
         return it->second.HandleDbQueryResult( dbResult );
      }
   }

   else
      assert( 0 );

 /*  switch( dbResult->lookup )
   {
      cout << "Db query type:"<< dbResult->lookup << ", success=" << dbResult->successfulQuery << endl;
   }*/
   return false;
}

//---------------------------------------------------------------

int      DiplodocusContact::CallbackFunction()
{
   while( m_serversNeedingUpdate.size() )
   {
      U32 serverId = m_serversNeedingUpdate.front();
      m_serversNeedingUpdate.pop_front();

      Threading::MutexLock locker( m_mutex );
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         IChainedInterface* interfacePtr = chainedInput.m_interface;
         KhaanContact* khaan = static_cast< KhaanContact* >( interfacePtr );
         if( khaan->GetServerId() == serverId )
         {
            if( khaan->Update() == false )
            {
               m_serversNeedingUpdate.push_back( serverId );
            }
         }
      }
   }
   UpdateAllConnections();

   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, m_connectedClients.size(), m_listeningPort, m_serverName );

   /*ContinueInitialization();*/
   // check for new friend requests and send a small list of notifications

   return 1;
}

//---------------------------------------------------------------

void     DiplodocusContact::UpdateAllConnections()
{
   m_mutex.lock();
   UserContactMapIterator it = m_users.begin();
   while( it != m_users.end() )
   {
      UserContact& contact = it->second;
      UserContactMapIterator temp = it++;
      if( contact.IsLoggedOut() )
      {
         if( contact.SecondsExpiredSinceLoggedOut() > SecondsBeforeRemovingLoggedOutUser )
         {
            m_userLookupById.erase( contact.GetUserInfo().id );
            
            m_users.erase( temp );
         }
      }
      else 
      {
         contact.Update();
      }
   }
   m_mutex.unlock();

   Parent::UpdateAllConnections();
}

//---------------------------------------------------------------
/*
bool     DiplodocusContact::ContinueInitialization()
{
   if( m_initializationStage >= InitializationStage_Complete )
      return;

   time_t   currentTime;
   time( &currentTime );
   if( m_isExecutingQuery == true || difftime( currentTime, m_lastTimeStamp ) < m_periodicitySeconds ) 
   {
      return;
   }
   m_lastTimeStamp = currentTime;

   switch( m_initializationStage )
   {
      case InitializationStage_Begin:// do nothing.. allows init to continue
         break;
      case InitializationStage_LoadAllUsers:
         SendInitializationQuery( QueryType_AllUsers );
         break;
      case InitializationStage_LoadAllUsersProfiles:
         SendInitializationQuery( QueryType_AllUserProfiles );
         break;
      case InitializationStage_LoadFriends:
         SendInitializationQuery( QueryType_Friends );
         break;
      case InitializationStage_LoadFriendsRequests:
         dbQuery->lookup = QueryType_AllUsers;
         break;
      case InitializationStage_MatchFriends:
         FillUpUserFriendsLists();
         break;
      case InitializationStage_MatchRequestsToUsers:
         FillUpUserFriendsLists();
         break;
      //m_initializationStage = InitializationStage_LoadAllUsers;
   }

}

//---------------------------------------------------------------

void     DiplodocusContact::AdvanceInitializationStep()
{
   if( m_initializationStage >= InitializationStage_Complete )
      return;

   m_initializationStage ++;
}

//---------------------------------------------------------------

bool     DiplodocusContact::SendInitializationQuery( QueryType queryType )
{
   if( queryType < QueryType_UserLoginInfo || queryType >= QueryType_End )
      return false;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->meta =         "";
   dbQuery->lookup =       queryType;
   dbQuery->serverLookup = 0;

   switch( queryType )
   {
      case QueryType_AllUsers:
         dbQuery->query = "SELECT * FROM users WHERE active='1'";
      break;
      case QueryType_AllUserProfiles
         dbQuery->query = "SELECT * FROM user_profile";
      break;
      case QueryType_Friends:
         dbQuery->query = "SELECT * FROM friends";
      break;
      case QueryType_FriendRequest:
         dbQuery->query = "SELECT * FROM friend_pending";
      break;
      //InitializationStage_LoadFriendsRequests
      //m_initializationStage = InitializationStage_LoadAllUsers;
   }

   if( dbQuery->query.size() > 0 )
   {
      AddQueryToOutput( dbQuery );
      m_isExecutingQuery = true;
      return true;
   }
   return false;
}*/


//---------------------------------------------------------------

bool     DiplodocusContact::ConnectUser( PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UserContactMapIterator it = m_users.find( connectionId );
   if( it != m_users.end() )
      return false;

   bool found = false;
   // if the user is already here but relogged, simply 
   m_mutex.lock();
      it = m_users.begin();
      while( it != m_users.end() )
      {
         if( it->second.GetUserInfo().uuid == loginPacket->uuid ) 
         {
            found = true;
            U32 id = it->second.GetUserInfo().id;
            UserIdToContactMapIterator itIdToContact = m_userLookupById.find( id );
            if( itIdToContact != m_userLookupById.end() )
            {
               itIdToContact->second = connectionId;
            }
            it->second.SetConnectionId( connectionId );
            it->second.FinishLoginBySendingUserFriendsAndInvitations();
            
            m_users.insert( UserContactPair( connectionId, it->second ) );
            m_users.erase( it );
            break;
         }
         it++;
      }
   m_mutex.unlock();

   if( found == false )
   {

      UserInfo ui;
      ui.userName =        loginPacket->userName;
      ui.uuid =            loginPacket->uuid;
      ui.apple_id = "";
      ui.connectionId =    connectionId;
      ui.gameProductId =   loginPacket->gameProductId;
      ui.active =          loginPacket->active;
      ui.email =           loginPacket->email;
      ui.passwordHash =    loginPacket->password;
      ui.id =              loginPacket->userId;

      UserContact user( ui, connectionId );
      user.SetServer( this );

      m_mutex.lock();
      m_users.insert( UserContactPair( connectionId, user ) );
      m_userLookupById.insert( UserIdToContactPair( ui.id, connectionId ) );
      m_mutex.unlock();
   }
   return true;
}

//---------------------------------------------------------------

bool     DiplodocusContact::DisconnectUser( PacketPrepareForUserLogout* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UserContactMapIterator it = m_users.find( connectionId );
   if( it == m_users.end() )
      return false;

   it->second.NeedsUpdate();
   it->second.UserLoggedOut();
   // we need to send notifications

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusContact::HandlePacketRequests( PacketContact* packet, U32 connectionId )
{
   UserContactMapIterator it = m_users.find( connectionId );
   if( it == m_users.end() )
      return false;

   bool wasHandled = it->second.HandleRequestFromClient( packet );

   if( wasHandled )
      return true;

   switch( packet->packetSubType )
   {
   case PacketContact::ContactType_GetListOfContacts:
      break;

 /*  case PacketGameToServer::GamePacketType_ListOfGames:
      IsUserAllowedToUseThisProduct( static_cast< PacketListOfGames* >( unwrappedPacket ) );
      delete unwrappedPacket;
      return true;*/
   }

   return false;
}

//---------------------------------------------------------------

bool     DiplodocusContact::AddQueryToOutput( PacketDbQuery* packet )
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


///////////////////////////////////////////////////////////////////
