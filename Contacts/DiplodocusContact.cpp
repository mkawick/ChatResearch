// DiplodocusContact.cpp

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/Utils/CommandLineParser.h"
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

DiplodocusContact::DiplodocusContact( const string& serverName, U32 serverId ): Diplodocus< KhaanContact >( serverName, serverId, 0,  ServerType_Contact ),
                        StatTrackingConnections(),
                        m_numSearches( 0 ),
                        m_numInvitesSent( 0 ),
                        m_numInvitesAccepted( 0 ),
                        m_numInvitesRejected( 0 )
{
   SetSleepTime( 45 );
   time_t currentTime;
   time( &currentTime );
   m_timestampDailyStatServerStatisics = ZeroOutHours( currentTime );
   m_timestampHourlyStatServerStatisics = ZeroOutMinutes( currentTime );
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

      case PacketLogin::LoginType_UserUpdateProfile:
         UpdateUserProfile( static_cast< PacketUserUpdateProfile* >( unwrappedPacket ) );
         return true;
      }
   }
   else if( unwrappedPacket->packetType == PacketType_Contact )
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
         PacketDbQueryResult* dbResult = static_cast<PacketDbQueryResult*>( packet );

         Threading::MutexLock locker( m_mutex );
         m_dbQueries.push_back( dbResult );
         return true;
      }
   }

   return false;
}

//---------------------------------------------------------------

void  DiplodocusContact::UpdateDbResults()
{   
   PacketFactory factory;
   Threading::MutexLock locker( m_mutex );

   deque< PacketDbQueryResult* >::iterator it = m_dbQueries.begin();
   while( it != m_dbQueries.end() )
   {
      PacketDbQueryResult* dbResult = *it++;
      if( dbResult->customData != NULL )
            cout << "UpdateDbResults: Non-null custom data " << endl;

      U32 connectionId = dbResult->id;
      if( connectionId != 0 )
      {
         UserContactMapIterator it = m_users.find( connectionId );
         if( it != m_users.end() )
         {
            cout << "Db query type: "<< dbResult->lookup << " handed off to UserContact #" << connectionId << endl;
            it->second.HandleDbQueryResult( dbResult );
         }
         else
         {
            cout << "ERROR: DB Result has user that cannot be found" << endl;
         }
      }

      BasePacket* packet = static_cast<BasePacket*>( dbResult );
      PacketCleaner cleaner( packet );
   }
   m_dbQueries.clear();
}

//-----------------------------------------------------------------------------------------

void     DiplodocusContact::TrackCountStats( StatTracking stat, float value, int sub_category )
{
   StatTrackingConnections::TrackCountStats( m_serverName, m_serverId, stat, value, sub_category );
}

//---------------------------------------------------------------

void     DiplodocusContact::RunHourlyStats()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampHourlyStatServerStatisics ) >= timeoutHourlyStatisics ) 
   {
      m_timestampHourlyStatServerStatisics = ZeroOutMinutes( currentTime );
   }
}

//---------------------------------------------------------------

void     DiplodocusContact::RunDailyStats()
{
   time_t currentTime;
   time( &currentTime );
   if( difftime( currentTime, m_timestampDailyStatServerStatisics ) >= timeoutDailyStatisics ) 
   {
      m_timestampDailyStatServerStatisics = ZeroOutHours( currentTime );

      TrackCountStats( StatTracking_ContactNumberSearchesForUserPerformed, static_cast< float >( m_numSearches ), 0 );
      TrackCountStats( StatTracking_ContactNumInvitesSentPerDay, static_cast< float >( m_numInvitesSent ), 0 );
      TrackCountStats( StatTracking_ContactAcceptedInvitesPerDay, static_cast< float >( m_numInvitesAccepted ), 0 );
      TrackCountStats( StatTracking_ContactRejectedInvitesPerDay, static_cast< float >( m_numInvitesRejected ), 0 );
      ClearStats();
   }
}

//---------------------------------------------------------------

int      DiplodocusContact::CallbackFunction()
{
   UpdateDbResults();

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

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   RunHourlyStats();
   RunDailyStats();
   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );
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

bool     DiplodocusContact::ConnectUser( PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UserContactMapIterator it = m_users.find( connectionId );// don't do anything if this user is already logged in.
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

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusContact::UpdateUserProfile( const PacketUserUpdateProfile* profile )
{
   U32 connectionId = profile->connectionId;

   UserContactMapIterator it = m_users.find( connectionId );
   if( it == m_users.end() )
      return false;

   it->second.UpdateProfile( profile );

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

void     DiplodocusContact::ClearStats()
{
   m_numSearches = 0;
   m_numInvitesSent = 0;
   m_numInvitesAccepted = 0;
   m_numInvitesRejected = 0;
}

//---------------------------------------------------------------


///////////////////////////////////////////////////////////////////
