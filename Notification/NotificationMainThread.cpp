#include "NotificationMainThread.h"

#include <iostream>
#include <time.h>
#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/NotificationPacket.h"

#include "../NetworkCommon/Database/Deltadromeus.h"// breaking rules to make things easier for GARY
#include <mysql/mysql.h> // MySQL Include File


#include "NotificationMainThread.h"

#include "../NetworkCommon/Database/StringLookup.h"

#include "server_notify.h"


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

NotificationMainThread::NotificationMainThread( const string& serverName, U32 serverId ): Queryer(), Diplodocus< KhaanServerToServer >( serverName, serverId, 0,  ServerType_Notification ), m_database( NULL )
{
   time( &m_lastNotificationCheck_TimeStamp );
   m_lastNotificationCheck_TimeStamp = ZeroOutMinutes( m_lastNotificationCheck_TimeStamp );
   SetSleepTime( 100 );

   NotifyIosInit(); // this does not work. I discovered a failure.. 
   NotifyAndroidInit(); 
}

NotificationMainThread :: ~NotificationMainThread()
{
   NotifyIosUninit();
   NotifyAndroidUninit();
}

///////////////////////////////////////////////////////////////////

void     NotificationMainThread::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_serverId, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( localKhaan->GetServerId() );
}

///////////////////////////////////////////////////////////////////

bool     NotificationMainThread::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );// we do not accept any data from the gateway
      HandleCommandFromGateway( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_ServerJobWrapper )// login and such
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_GatewayWrapper ) 
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromGateway( packet, connectionId );
      return true;
   }
   
   return false;
}

//---------------------------------------------------------------

bool   NotificationMainThread::AddOutputChainData( BasePacket* packet, U32 connectionId ) // coming from downsteam in
{
   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast<PacketDbQueryResult*>( packet );
         Threading::MutexLock locker( m_mutex );
         m_dbQueries.push_back( result );
        /* if( result->customData != NULL )
            cout << "AddOutputChainData: Non-null custom data " << endl;*/
      }
      return true;
   }
   return false;
}
//---------------------------------------------------------------

bool     NotificationMainThread::SendMessageToClient( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )// this is already wrapped up and ready for the gateway... send it on.
   {
      Threading::MutexLock locker( m_mutex );

      ClientMapIterator itInputs = m_connectedClients.begin();
      if( itInputs != m_connectedClients.end() )// only one output currently supported.
      {
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( itInputs->second );
         khaan->AddOutputChainData( packet );
         m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
         itInputs++;
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool  NotificationMainThread::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   U8 packetType = unwrappedPacket->packetType;

   if( packetType == PacketType_Login )
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
   else if( packetType == PacketType_Notification )
   {
      if( unwrappedPacket->packetSubType == PacketNotification::NotificationType_SendNotification )
      {
         HandleNotification( static_cast< PacketNotification_SendNotification* >( unwrappedPacket ) );
      }
   }

   return false;
}

//---------------------------------------------------------------

bool     NotificationMainThread::HandlePacketFromGateway( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType != PacketType_GatewayWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   U8 packetType = unwrappedPacket->packetType;
   
   {// local scope
      Threading::MutexLock locker( m_mutex );
      UserConnectionIterator item = m_userConnectionMap.find( connectionId );
      if( item != m_userConnectionMap.end() )
      {
         UserConnection& user = item->second;
         user.HandleRequestFromClient( unwrappedPacket );
      }
   }
   return false;
}

//---------------------------------------------------------------

bool     NotificationMainThread::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UserConnectionIterator it = m_userConnectionMap.find( connectionId );// don't do anything if this user is already logged in.
   if( it != m_userConnectionMap.end() )
      return false;

   bool found = false;
   // if the user is already here but relogged, simply 
   m_mutex.lock();
      it = m_userConnectionMap.begin();
      while( it != m_userConnectionMap.end() )
      {
         if( it->second.GetUserInfo().uuid == loginPacket->uuid ) 
         {
            found = true;
            //U32 id = it->second.GetUserInfo().id;
           /* UserIdToContactMapIterator itIdToContact = m_userLookupById.find( id );
            if( itIdToContact != m_userLookupById.end() )
            {
               itIdToContact->second = connectionId;
            }*/
            it->second.SetConnectionId( connectionId );
            
            m_userConnectionMap.insert( UserConnectionPair( connectionId, it->second ) );
            m_userConnectionMap.erase( it );
            break;
         }
         it++;
      }
   m_mutex.unlock();

   if( found == false )
   {
      UserConnection user( loginPacket );
      user.SetServer( this );

      m_mutex.lock();
      m_userConnectionMap.insert( UserConnectionPair( connectionId, user ) );
      m_mutex.unlock();
   }
   return true;
}


//---------------------------------------------------------------

bool     NotificationMainThread::DisconnectUser( const PacketPrepareForUserLogout* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UserConnectionIterator it = m_userConnectionMap.find( connectionId );
   if( it == m_userConnectionMap.end() )
      return false;

   //it->second.NeedsUpdate();
   it->second.UserLoggedOut();

   return true;
}

//---------------------------------------------------------------

bool     NotificationMainThread::HandleNotification( const PacketNotification_SendNotification* unwrappedPacket )
{
   // it's very unlikely that this user is loaded already. It's probably best to just look up the user's devices and send notifications.

   if( m_database )
   {
      st_mysql* dbHandle = m_database->GetDbHandle();
   }
   //
   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

void     NotificationMainThread::PeriodicCheckForNewNotifications()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_lastNotificationCheck_TimeStamp ) >= timeoutNotificationSend ) 
   {
      m_lastNotificationCheck_TimeStamp = ZeroOutMinutes( currentTime );// advance the hour.
   
      // hooks for sending notifications
   }
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     NotificationMainThread::AddQueryToOutput( PacketDbQuery* query )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( query, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   BasePacket* packet = static_cast<BasePacket*>( query );
   PacketFactory factory;
   factory.CleanupPacket( packet );/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}

//---------------------------------------------------------------

void     NotificationMainThread::RemoveExpiredConnections()
{
   m_mutex.lock();
   UserConnectionIterator it = m_userConnectionMap.begin();
   while( it != m_userConnectionMap.end() )
   {
      UserConnection& contact = it->second;
      UserConnectionIterator temp = it++;
      if( contact.IsLoggedOut() )
      {
         if( contact.SecondsExpiredSinceLoggedOut() > SecondsBeforeRemovingLoggedOutUser )
         {
            //m_userLookupById.erase( contact.GetUserInfo().id );
            
            m_userConnectionMap.erase( temp );
         }
      }
      else 
      {
         contact.Update();
      }
   }
   m_mutex.unlock();
}

//---------------------------------------------------------------
//---------------------------------------------------------------

int     NotificationMainThread::MainLoop_InputProcessing()
{
   PacketFactory factory;

   m_mutex.lock();
   deque< PacketDbQueryResult* > tempQueue = m_dbQueries;
   m_dbQueries.clear();
   m_mutex.unlock();

   deque< PacketDbQueryResult* >::iterator it = tempQueue.begin();
   while( it != tempQueue.end() )
   {
      PacketDbQueryResult* dbResult = *it++;

      U32 connectionId = dbResult->id;
      if( connectionId != 0 )
      {
         UserConnectionIterator it = m_userConnectionMap.find( connectionId );
         if( it != m_userConnectionMap.end() )
         {
            it->second.HandleDbQueryResult( dbResult );
         }
         else // user must have disconnected
         {
            BasePacket* packet = static_cast<BasePacket*>( dbResult );
            factory.CleanupPacket( packet );
         }
      }
      else
      {
         // locally handled query
         // LOOK HERE GARY FOR async queries
      }
   }
   return 1;
}

//---------------------------------------------------------------

int      NotificationMainThread::MainLoop_OutputProcessing()
{
   UpdateAllConnections();

   time_t currentTime;
   time( &currentTime );

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   PeriodicCheckForNewNotifications();
   RemoveExpiredConnections();

   if( m_database == NULL )
   {
      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
      {
         ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
         Database::Deltadromeus* db = dynamic_cast< Database::Deltadromeus*> ( outputPtr );
         if( db != NULL )
         {
            m_database = db;
            break;
         }
         itOutputs++;
      }
   }

   return 1;
}


//---------------------------------------------------------------
//---------------------------------------------------------------