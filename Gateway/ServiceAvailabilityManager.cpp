
#include <iostream>
#include <list>

#include <boost/tokenizer.hpp>
//#include <conio.h>
#include <assert.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>

#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/StringUtils.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "ServiceAvailabilityManager.h"
#include "MainGatewayThread.h"
#include "FruitadensGateway.h"

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

ServiceAvailabilityManager::ServiceAvailabilityManager() : m_mainGatewayThread( NULL )
{
}

ServiceAvailabilityManager::~ServiceAvailabilityManager()
{
}


void     ServiceAvailabilityManager::Update()
{
   if( m_mainGatewayThread == NULL )
      return;

   CheckOnServerStatusChanges();
}


////////////////////////////////////////////////////////

void     ServiceAvailabilityManager::CheckOnServerStatusChanges()
{
   std::list< FruitadensGateway* > outputServices;
   m_mainGatewayThread->GetListOfOutputs( outputServices );//m_listOfOutputs;

   vector< QOS_ServiceChange > serviceChanges;
   std::list< FruitadensGateway* >::iterator itOutput = outputServices.begin();
   while( itOutput != outputServices.end() )
   {
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( *itOutput );
      itOutput++;

      QOS_ServiceChange qosChange;
      ServerType serverType = fruity->GetConnectedServerType();
      qosChange.errorTypeMessageToSend = 0;

      if( fruity->IsRecentlyDisconnected() == true )
      {
         fruity->ClearRecentlyDisconnectedFlag();
         qosChange.text = "Server is down";
         qosChange.gameId = 0;
         qosChange.serverType = serverType;
         qosChange.isConnected = false;
         
         if( IsCoreServerType( serverType ) == true )
         {
            qosChange.errorTypeMessageToSend = Packet_QOS_ReportToClient::ErrorState_ServerIsNotAvailable;
            qosChange.forceUsersToDc = IsLoginServerType( serverType );
         }
         else if( IsGameServerType( serverType ) == true ) // special case.
         {
            qosChange.errorTypeMessageToSend = Packet_QOS_ReportToClient::ErrorState_GameIsDown;
            qosChange.forceUsersToDc = true;
            qosChange.gameId = fruity->GetConnectedGameProductId();
         }
         
      }
      else if( fruity->IsRecentlyConnected() == true )
      {
         fruity->ClearRecentlyConnectedFlag();
         qosChange.text = "Server is up";
         qosChange.gameId = 0;
         qosChange.serverType = serverType;
         qosChange.forceUsersToDc = false;
         qosChange.isConnected = true;

         if( IsCoreServerType( serverType ) )
         {
            qosChange.errorTypeMessageToSend = Packet_QOS_ReportToClient::ErrorState_ServerIsAvailable;
            
            //BroadcastPacketToAllUsers( "Server is up", Packet_QOS_ReportToClient::ErrorState_ServerIsAvailable, serverType, 0, 0 );
         }
         else if( IsGameServerType( serverType ) == true ) // special case.
         {
            qosChange.errorTypeMessageToSend = Packet_QOS_ReportToClient::ErrorState_GameIsUp;
            qosChange.forceUsersToDc = false;
            qosChange.gameId = fruity->GetGameProductId();
            //BroadcastPacketToAllUsers( "Server is up", Packet_QOS_ReportToClient::ErrorState_GameIsUp, serverType, fruity->GetGameProductId(), fruity->GetGameProductId() );
         }
      }
      if( qosChange.errorTypeMessageToSend != 0 )
      {
         serviceChanges.push_back( qosChange );
      }
   }

   SendAllServerStateChangesToClients( serviceChanges );
   
   //---------------------------------------------------
  /* if( m_isServerDownForMaintenence == true && 
      m_hasInformedConnectedClientsThatServerIsDownForMaintenence == false)
   {
   }*/
}

////////////////////////////////////////////////////////

void     ServiceAvailabilityManager::SendAllServerStateChangesToClients( const vector< QOS_ServiceChange >& listOfchanges )
{
   vector< QOS_ServiceChange >::const_iterator it = listOfchanges.begin();
   while( it != listOfchanges.end() )
   {
      const QOS_ServiceChange& change = *it++;

      m_mainGatewayThread->BroadcastPacketToAllUsers( change.text, change.errorTypeMessageToSend, change.serverType, change.gameId, change.gameId, true );
      if( change.forceUsersToDc )
      {
         U32 gameId = change.gameId;
         vector< U32 > connectionIds;

         PacketLogin_LogoutAllUsers* packet = new PacketLogin_LogoutAllUsers;
         m_mainGatewayThread->CreateFilteredListOfClientConnections( gameId, connectionIds );
         packet->connectionIds.copy( connectionIds );

         PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
         wrapper->SetupPacket( packet, -1 );

         // tell login server that users are being dc'd.. try to send a single message for a game if possible
         m_mainGatewayThread->SendPacketToServer( wrapper, ServerType_Login );
         
         // tell all users of that game that they are being dc'd         
         vector< U32 >::iterator it = connectionIds.begin();
         while( it != connectionIds.end() )
         {
            m_mainGatewayThread->CloseConnection( *it++ );
         }
      }
   }
}

////////////////////////////////////////////////////////

void     ServiceAvailabilityManager::InformUserAboutAvailableFeatures( U8 gameId, U32 connectionId )
{
   vector< ServerStatus > servers;
   bool  requestDisconnected = true;

   GetConnectedServerList( servers, requestDisconnected );
   if( servers.size() )// let's inform the client about what is not working.
   {
      // first we'll see if the login or the game are invalid.
      vector< ServerStatus >::iterator it = servers.begin();
      while( it != servers.end() )
      {
         ServerType type = it->type;
         if( ( type == ServerType_Login ) ||
            ( type == ServerType_GameInstance && it->gameId == gameId ))
         {
            m_mainGatewayThread->LogUserOutIfKeyFeaturesAreUnavailable( type, gameId, connectionId );
            return;
         }
         it++;
      }

      m_mainGatewayThread->InformUserOfMissingFeatures( servers, connectionId, gameId );
   }
}


////////////////////////////////////////////////////////

void     ServiceAvailabilityManager::GetConnectedServerList( vector< ServerStatus >& servers, bool onlyDisconnected )
{
   std::list< FruitadensGateway* > outputServices;
   m_mainGatewayThread->GetListOfOutputs( outputServices );//m_listOfOutputs;

   servers.clear();

   vector< QOS_ServiceChange > serviceChanges;
   std::list< FruitadensGateway* >::iterator itOutput = outputServices.begin();
   while( itOutput != outputServices.end() )
   {
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( *itOutput );
      itOutput++;

      ServerStatus serverStatus;
      serverStatus.type = fruity->GetConnectedServerType();
      serverStatus.gameId = fruity->GetConnectedGameProductId();
      serverStatus.isConnected = fruity->IsConnected();
      serverStatus.isEnabled = fruity->IsEnabled();
      serverStatus.fruity = fruity;

      if( onlyDisconnected == true )
      {        
         if( serverStatus.isConnected == false )
         {
            servers.push_back( serverStatus );
         }
      }
      else // or push all
      {
         servers.push_back( serverStatus );
      }
   }
}
////////////////////////////////////////////////////////


////////////////////////////////////////////////////////

void     ServiceAvailabilityManager::ScheduledOutages( const PacketServerConnectionInfo_ServerOutageSchedule* outagePacket )
{
   bool  AnyRealChanges = false;
   U32 num = outagePacket->scheduledOutages.size();
   for( U32 i=0; i<num; i++ )
   {
      // keep in mind, that there may be dups
      const ScheduledOutage& so = outagePacket->scheduledOutages[i];
      if( CopyScheduledOutagesToLocalOutages( so ) == true )
         AnyRealChanges = true;
   }

   if( AnyRealChanges == true )
   {
      InformAllConnectedUsersOfScheduledOutages();
      EnableAndDisableServicesBasedOnSchecules();
      RemoveCancelledAndExpiredScheduledOutages();
   }
}

////////////////////////////////////////////////////////

bool     ServiceAvailabilityManager::CopyScheduledOutagesToLocalOutages( const ScheduledOutage& newScheduledOutage )
{
   int num = m_scheduledOutages.size();
   for( int i=0; i< num; i++ )
   {
      ScheduledOutage& outage = m_scheduledOutages[i];
      if( outage.type == newScheduledOutage.type )
      {
         if( outage == newScheduledOutage )
         {
            return false; // simply a duplicate
         }
         else
         {
            outage = newScheduledOutage;// copy operator
         }
         break;
      }
   }

   return true;
}

////////////////////////////////////////////////////////

void     ServiceAvailabilityManager::EnableAndDisableServicesBasedOnSchecules()
{
   bool onlyDisconnected = false;
   vector< ServerStatus > serverList;
   GetConnectedServerList( serverList, onlyDisconnected );
   int numServers = serverList.size();

   std::list< FruitadensGateway* > outputServices;
   m_mainGatewayThread->GetListOfOutputs( outputServices );

   time_t currentTime;
   time( &currentTime );

   int num = m_scheduledOutages.size();
   for( int i=0; i< num; i++ )
   {
      ScheduledOutage& outage = m_scheduledOutages[i];
      
      for( int j=0; j<numServers; j++ )
      {
         if( outage.type == serverList[i].type )
         {
            ServerStatus& statusServer = serverList[i];
            if( outage.gameId != statusServer.gameId )// searching
            {
               continue; // j
            }

            if( statusServer.isEnabled == true )
            {
               if( outage.cancelled == false && /// this is the normal case
                  InCurrentTimeWindow( currentTime, outage.beginTime, outage.downTimeInSeconds ) == true )
               {
                  statusServer.fruity->Disable();
               }
               
            }
            else // currently disabled
            {
               if( outage.cancelled == true ||
                  InCurrentTimeWindow( currentTime, outage.beginTime, outage.downTimeInSeconds ) == false )
               {
                  statusServer.fruity->Enable();
               }
            }
         }
      }
   }
}

////////////////////////////////////////////////////////

void     ServiceAvailabilityManager::InformAllConnectedUsersOfScheduledOutages()
{
   vector< ClientConnectionForGame > ccfg;
   m_mainGatewayThread->CreateListOfClientConnectionsForGame( ccfg );

   PacketFactory factory;
   vector< ClientConnectionForGame >::iterator it = ccfg.begin();
   while( it != ccfg.end() )
   {
      const ClientConnectionForGame& clientConnection = *it++;
      ClientSide_ServerOutageSchedule* packet = new ClientSide_ServerOutageSchedule;

      U32 num = m_scheduledOutages.size();
      for( U32 i=0; i<num; i++ )
      {
         ScheduledOutage& outage = m_scheduledOutages[i];
         if( outage.gameId == 0 || // match each user... all services and matching games are reported
            outage.gameId == clientConnection.gameId )
         {
            ClientSide_ScheduledServiceOutage outageToclient;
            outageToclient.beginTime = outage.beginTime;
            outageToclient.cancelled = outage.cancelled;
            outageToclient.downTimeInSeconds = outage.downTimeInSeconds;
            outageToclient.gameId = outage.gameId;
            outageToclient.type = outage.type;

            packet->scheduledOutages.push_back( outageToclient );
         }
      }

      if( packet->scheduledOutages.size() )
      {
         PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
         wrapper->SetupPacket( packet, clientConnection.connectionId );

         m_mainGatewayThread->AddOutputChainData( wrapper, clientConnection.connectionId );
      }
      else
      {
         BasePacket* tempPacket = static_cast< BasePacket* > ( packet );
         factory.CleanupPacket( tempPacket );
      }
   }   
}

////////////////////////////////////////////////////////


void     ServiceAvailabilityManager::RemoveCancelledAndExpiredScheduledOutages()
{
   time_t currentTime;
   time( &currentTime );

   vector< ScheduledOutage >::iterator it = m_scheduledOutages.begin();
   while( it != m_scheduledOutages.end() )
   {
      ScheduledOutage& outage = *it;
      if( outage.cancelled == true )
      {
         it = m_scheduledOutages.erase( it );
         continue;
      }
      if( InCurrentTimeWindow( currentTime, outage.beginTime, outage.downTimeInSeconds ) == true )
      {
         it = m_scheduledOutages.erase( it );
         continue;
      }

      it++;
   }
}

////////////////////////////////////////////////////////