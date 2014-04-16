// DiplodocusGateway.cpp

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/StatPacket.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
//#include "../NetworkCommon/ChainedArchitecture/ChainedInterface.h"

#include "FruitadensGateway.h"
#include "DiplodocusGateway.h"
#include "ErrorCodeLookup.h"

//#define VERBOSE


//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

DiplodocusGateway::DiplodocusGateway( const string& serverName, U32 serverId ) : Diplodocus< KhaanGateway > ( serverName, serverId, 0, ServerType_Gateway ), StatTrackingConnections(),
                                          m_connectionIdTracker( 12 ),
                                          m_printPacketTypes( false ),
                                          m_reroutePort( 0 )
{
   SetSleepTime( 16 );// 30 fps
   SetSendHelloPacketOnLogin( true );
   
   time( &m_timestampSendConnectionStatisics );
   m_timestampSendStatServerStatisics = m_timestampSendConnectionStatisics;

   m_orderedOutputPacketHandlers.reserve( PacketType_Num );
}

DiplodocusGateway::~DiplodocusGateway()
{
}

void     DiplodocusGateway::Init()
{
   OrderOutputs();
}

void   DiplodocusGateway::NotifyFinishedAdding( IChainedInterface* obj ) 
{
   //obj->
   cout << " NotifyFinishedAdding: added obj " << endl;
   //m_listOfInputs
} 

//-----------------------------------------------------------------------------------------

U32      DiplodocusGateway::GetNextConnectionId()
{
   Threading::MutexLock locker( m_inputChainListMutex );
   m_connectionIdTracker ++;
   if( m_connectionIdTracker >= ConnectionIdExclusion.low &&  m_connectionIdTracker <= ConnectionIdExclusion.high )
   {
      m_connectionIdTracker = ConnectionIdExclusion.high + 1;
   }
   U32 returnValue = m_connectionIdTracker;

   return returnValue;
}

//-----------------------------------------------------------------------------------------

bool  DiplodocusGateway::AddInputChainData( BasePacket* packet, U32 connectionId ) // coming from the client-side socket
{
   PrintDebugText( "AddInputChainData", 1);
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
     /* if( packet->packetType == PacketType_Base )
      {
         if( packet->packetSubType == BasePacket::BasePacket_RerouteRequest )
         {
            HandleReroutRequest( connectionId );
            return false;// delete the original data
         }
      }*/

      PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
      wrapper->SetupPacket( packet, connectionId );

      if( m_printPacketTypes )
      {
         cout << "Packet to servers: " << (int)packet->packetType << ":" << (int)packet->packetSubType << endl;
      }

      Threading::MutexLock locker( m_inputChainListMutex );
      m_packetsToBeSentInternally.push_back( wrapper );
      return true;
   }
   else
   {
      PacketFactory factory;
      factory.CleanupPacket( packet );// it dies here. we should log this and try to disconnect the user
      return false;
   }
}

//-----------------------------------------------------------------------------------------
/*
void     DiplodocusGateway::HandleReroutRequest( U32 connectionId )
{
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      PacketRerouteRequestResponse* response = new PacketRerouteRequestResponse;
      if( IsRerouoting() == true )
      {
         PacketRerouteRequestResponse::Address address;
         address.address = m_rerouteAddress;
         address.port = m_reroutePort;
         address.name = "gateway";
         address.whichLocationId = PacketRerouteRequestResponse::LocationId_Gateway;

         response->locations.push_back( address );
      }
      KhaanGateway* khaan = connIt->second;
      HandlePacketToKhaan( khaan, response );// all deletion and such is handled lower
   }
}*/

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::InputConnected( IChainedInterface * chainedInput )
{
   KhaanGateway* khaan = static_cast< KhaanGateway* >( chainedInput );
   string currentTime = GetDateInUTC();
   cout << "Accepted connection at time:" << currentTime << " from " << inet_ntoa( khaan->GetIPAddress().sin_addr ) << endl;
   PrintDebugText( "** InputConnected" , 1 );
   U32 newId = GetNextConnectionId();
   m_socketToConnectionMap.insert( SocketToConnectionPair( khaan->GetSocketId(), newId ) );
   m_connectionToSocketMap.insert( SocketToConnectionPair( newId, khaan->GetSocketId() ) );
   m_connectionMap.insert( ConnectionPair( newId, KhaanGatewayWrapper( khaan ) ) );

   khaan->SetConnectionId( newId );

   khaan->SetGateway( this );
   //khaan->SendThroughLibEvent( true );

   Threading::MutexLock locker( m_inputChainListMutex );
   AddClientConnectionNeedingUpdate( newId );
}

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanGateway* khaan = static_cast< KhaanGateway* >( chainedInput );
   string currentTime = GetDateInUTC();
   cout << "Client disconnection at time:" << currentTime << " from " << inet_ntoa( khaan->GetIPAddress().sin_addr ) << endl;

   PrintDebugText( "** InputRemovalInProgress" , 1 );
   int connectionId = khaan->GetConnectionId();
   int socketId = khaan->GetSocketId();

   PacketLogout* logout = new PacketLogout();// must be done before we clear the lists of ids
   logout->wasDisconnectedByError = true;
   //logout->serverType = ServerType_Chat;

   AddInputChainData( logout, connectionId );

   m_socketToConnectionMap.erase( socketId );
   m_connectionToSocketMap.erase( connectionId );

   // this may be invalid - TBR
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      time_t currentTime;
      time( &currentTime );
      connIt->second.MarkForDeletion( currentTime );
      connIt->second.m_connector = NULL;
   }
   //m_connectionMap.erase( connectionId );
}

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::PrintPacketTypes( bool printingOn ) 
{
   m_printPacketTypes = printingOn; 
   cout << "Client side packet printing is ";
   if( m_printPacketTypes == true )
   {
      cout << "enabled" << endl;
   }
   else
   {
      cout << "disabled" << endl;
   }
}

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::SetupReroute( const string& address, U16 port )
{
   m_rerouteAddress = address;
   m_reroutePort = port;

   // todo, add support for rerouting to other servers like login

   // todo, add dynamic rerouting allowing an admin to login and 
}

//-----------------------------------------------------------------------------------------

void     DiplodocusGateway::TrackCountStats( StatTracking stat, float value, int sub_category )
{
   StatTrackingConnections::TrackCountStats( m_serverName, m_serverId, stat, value, sub_category );
}

//-----------------------------------------------------------------------------------------

bool     DiplodocusGateway::OrderOutputs()
{
   if( m_orderedOutputPacketHandlers.size() > 0 )
      return false;

   for( int packetType=PacketType_Base; packetType< PacketType_Num; packetType++ )
   {
      m_orderedOutputPacketHandlers.push_back( OutputConnectorList() );
      OutputConnectorList& listOfOutputs = m_orderedOutputPacketHandlers[ packetType ];

      ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
      while( itOutput != m_listOfOutputs.end() )
      {
         FruitadensGateway* fg = static_cast<FruitadensGateway*>( (*itOutput).m_interface );
         if( fg->AcceptsPacketType( packetType ) == true )
         {
            listOfOutputs.push_back( fg );
         }
         itOutput++;
      }
   }
   return true;
}

//-----------------------------------------------------------------------------------------

bool     DiplodocusGateway::PushPacketToProperOutput( BasePacket* packet )
{
   U32 packetType = packet->packetType;
   if( packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      packetType = wrapper->pPacket->packetType;
   }

   assert( packetType < m_orderedOutputPacketHandlers.size() );

   const OutputConnectorList& listOfOutputs = m_orderedOutputPacketHandlers[ packetType ];
   assert( listOfOutputs.size() > 0 ); // this should be where we look through our list for a match

   OutputConnectorList::const_iterator it = listOfOutputs.begin();
   while( it != listOfOutputs.end() )
   {
      FruitadensGateway* fruity = *it++;
      U32 unusedParam = -1;
      if( fruity->AddOutputChainData( packet, unusedParam ) == true )
      {
         return true;
      }
   }
   //








/*

   PrintDebugText( "PushPacketToProperOutput", 1 );

   U32 packetType = packet->packetType;
   if( packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      packetType = wrapper->pPacket->packetType;
   }

   std::vector< FruitadensGateway* > tempOutput;
   // create new scope
   {
      
      Threading::MutexLock    locker( m_outputChainListMutex );// quickly copy the list before doing more serious work
      ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
      while( itOutput != m_listOfOutputs.end() )
      {
         FruitadensGateway* fg = static_cast<FruitadensGateway*>( (*itOutput).m_interface );
         if( fg->AcceptsPacketType( packetType ) == true )
         {
            tempOutput.push_back( fg );
         }
         itOutput++;
      }
   }

   std::vector< FruitadensGateway* >::iterator itOutput = tempOutput.begin();
   while( itOutput != tempOutput.end() )
   {
      FruitadensGateway* fruity = *itOutput;
      U32 unusedParam = -1;
      if( fruity->AddOutputChainData( packet, unusedParam ) == true )
      {
         return true;
      }
      itOutput++;
   }*/
  /* BaseOutputContainer tempOutput;
   // create new scope
   {
      Threading::MutexLock    locker( m_outputChainListMutex );// quickly copy the list before doing more serious work
      ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
      while( itOutput != m_listOfOutputs.end() )
      {
         tempOutput.push_back( *itOutput++ );
      }
   }

   ChainLinkIteratorType itOutput = tempOutput.begin();
   while( itOutput != tempOutput.end() )
   {
      IChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      U32 unusedParam = -1;
      if( fruity->AddOutputChainData( packet, unusedParam ) == true )
      {
         return true;
      }
      itOutput++;
   }*/
   
   return false;
}

//-----------------------------------------------------------------------------------------

int  DiplodocusGateway::MainLoop_InputProcessing()
{
   // m_inputChainListMutex.lock   // see CChainedThread<Type>::CallbackFunction()
   CommonUpdate();

   SendStatsToLoadBalancer();
   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   CleanupOldConnections();

   RunHourlyAverages();

   MoveClientBoundPacketsFromTempToKhaan();
   UpdateAllClientConnections();

   if( m_packetsToBeSentInternally.size() == 0 )
      return 0;

   PrintDebugText( "MainLoop_InputProcessing" );

   PacketFactory factory;
   while( m_packetsToBeSentInternally.size() )
   {
      BasePacket* packet = m_packetsToBeSentInternally.front();
      if( PushPacketToProperOutput( packet ) == false )
      {
         factory.CleanupPacket( packet );
      }
      m_packetsToBeSentInternally.pop_front();
   }

   return 1;
}

//-----------------------------------------------------------------------------------------

void  DiplodocusGateway::CleanupOldConnections()
{
   // cleanup old connections
   time_t currentTime;
   time( &currentTime );

   ConnectionMapIterator nextIt = m_connectionMap.begin();
   while( nextIt != m_connectionMap.end() )
   {
      ConnectionMapIterator oldConnIt = nextIt++;
      KhaanGatewayWrapper& khaanWrapper = oldConnIt->second;
      if( khaanWrapper.IsMarkedForDeletion () == true )
      {
         if( khaanWrapper.HasDeleteTimeElapsed( currentTime ) == true )
         {
            m_connectionMap.erase( oldConnIt );
         }
      }
   }
}


//-----------------------------------------------------------------------------------------

void  DiplodocusGateway::RunHourlyAverages()
{
   if( m_connectionMap.size() == 0 )
      return;

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampSendStatServerStatisics ) >= timeoutSendStatServerStatisics ) 
   {
      m_timestampSendStatServerStatisics = currentTime;

      float numConnections = static_cast<float>( m_connectionMap.size() );
      float totalNumSeconds = 0;

      ConnectionMapIterator nextIt = m_connectionMap.begin();
      while( nextIt != m_connectionMap.end() )
      {
         ConnectionMapIterator oldConnIt = nextIt++;
         KhaanGatewayWrapper& khaanWrapper = oldConnIt->second;
         if( khaanWrapper.m_connector )
         {
            time_t connectionTime = khaanWrapper.m_connector->GetConnectionTime();
            totalNumSeconds += static_cast<float>( difftime( currentTime, connectionTime ) );
         }

         
      }

      float averageNumSeconds = totalNumSeconds / numConnections;
      TrackCountStats( StatTracking_UserAverageTimeOnline, averageNumSeconds, 0 );
      TrackCountStats( StatTracking_UserTotalTimeOnline, totalNumSeconds, 0 );
      TrackCountStats( StatTracking_NumUsersOnline, numConnections, 0 );
   }
}

//-----------------------------------------------------------------------------------------

void  DiplodocusGateway::SendStatsToLoadBalancer()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampSendConnectionStatisics ) >= timeoutSendConnectionStatisics ) 
   {
      m_timestampSendConnectionStatisics = currentTime;
      int num = static_cast< int >( m_connectedClients.size() );

      TrackCountStats( StatTracking_UserTotalCount, static_cast<float>( num ), 0 );

      bool statsSent = false;
      ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
      while( itOutput != m_listOfOutputs.end() )
      {
         IChainedInterface* outputPtr = (*itOutput).m_interface;
         FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
         if( fruity->GetConnectedServerType() == ServerType_LoadBalancer )
         {
            PacketServerConnectionInfo* packet = new PacketServerConnectionInfo;
            packet->currentLoad = num;
            packet->serverAddress = GetIpAddress();
            packet->serverId = GetServerId();

            U32 unusedParam = -1;
            if( fruity->AddOutputChainData( packet, unusedParam ) == true )
            {
               statsSent = true;
               PrintDebugText( "SendStatsToLoadBalancer" ); 
               return;
            }
            else
            {
               delete packet;
            }
         }
         itOutput++;
      }
   }
}

//-----------------------------------------------------------------------------------------

bool  DiplodocusGateway::AddOutputChainData( BasePacket* packet, U32 serverType )
{
   PrintDebugText( "AddOutputChainData" ); 

   // pass through only
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      U32 id = wrapper->connectionId;

      //cout << "packet to client stored" << endl;
      Threading::MutexLock locker( m_inputChainListMutex );
      m_clientBoundTempStorage.push_back( packet );
      AddClientConnectionNeedingUpdate( id );
      
   }
   else
   {
      assert( 0 );
      return false;
   }
   return true;
}

//-----------------------------------------------------------------------------------------

// assuming that everything is thread protected at this point
void  DiplodocusGateway::HandlePacketToKhaan( KhaanGateway* khaan, BasePacket* packet )
{
   PrintDebugText( "HandlePacketToKhaan" );
   U32 connectionId = khaan->GetConnectionId();
   if( packet->packetType == PacketType_Login && 
       packet->packetSubType == PacketLogin::LoginType_InformGatewayOfLoginStatus )
   {
      PacketLoginToGateway* finishedLogin = static_cast< PacketLoginToGateway* >( packet );
      if( finishedLogin->wasLoginSuccessful )
      {
         khaan->AuthorizeConnection();
         khaan->SetAdminLevelOperations( finishedLogin->adminLevel );
         khaan->SetLanguageId( finishedLogin->languageId );
      }
      else
      {
         PrintDebugText( "HandlePacketToKhaan:: MarkForDeletion", 2 );
         khaan->DenyAllFutureData();
         TrackCountStats( StatTracking_ForcedDisconnect, 1, 0 );

         ConnectionMapIterator it = m_connectionMap.find( connectionId );
         if( it != m_connectionMap.end() )
         {
            KhaanGatewayWrapper& khaanWrapper = it->second;
            time_t currentTime;
            time( &currentTime );
            khaanWrapper.MarkForDeletion( currentTime );
         }
         
         connectionId = 0;
      }

      PacketLoginToClient* clientNotify = new PacketLoginToClient;
      clientNotify->wasLoginSuccessful = finishedLogin->wasLoginSuccessful;
      clientNotify->uuid = finishedLogin->uuid;
      clientNotify->userName = finishedLogin->userName;
      clientNotify->lastLogoutTime = finishedLogin->lastLogoutTime;
      clientNotify->connectionId = connectionId;
      clientNotify->loginKey = finishedLogin->loginKey;

      delete finishedLogin;
      packet = clientNotify;
   }

   if( m_printPacketTypes )
   {
      cout << "Packet to client: " << (int)packet->packetType << ":" << (int)packet->packetSubType << endl;
   }
   if( packet->packetType == PacketType_ErrorReport )
   {
      PacketErrorReport* error = static_cast< PacketErrorReport* >( packet );
      error->text = ErrorCodeLookup::GetString( error->errorCode );
   }
   khaan->AddOutputChainData( packet );

   Threading::MutexLock locker( m_inputChainListMutex );
   AddClientConnectionNeedingUpdate( connectionId );
}


//-----------------------------------------------------------------------------------------

void  DiplodocusGateway::AddClientConnectionNeedingUpdate( U32 connectionId )
{
   ConnectionIdQueue::iterator it = m_connectionsNeedingUpdate.begin();
   while( it != m_connectionsNeedingUpdate.end() )
   {
      if( *it++ == connectionId )
         return;
   }
   m_connectionsNeedingUpdate.push_back( connectionId );
}

//-----------------------------------------------------------------------------------------

void  DiplodocusGateway::MoveClientBoundPacketsFromTempToKhaan()
{
   if( m_clientBoundTempStorage.size() )
   {
      PrintDebugText( "MainLoop_OutputProcessing" );

      PacketFactory factory;
      while( m_clientBoundTempStorage.size() )
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( m_clientBoundTempStorage.front() );
         m_clientBoundTempStorage.pop_front();
         int connectionId = wrapper->connectionId;
         BasePacket* dataPacket = wrapper->pPacket;
         delete wrapper;
         bool  handled = false;

         SocketToConnectionMapIterator it = m_connectionToSocketMap.find( connectionId );
         if( it != m_connectionToSocketMap.end() )
         {
            ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
            if( connIt != m_connectionMap.end() )
            {
               KhaanGateway* khaan = connIt->second.m_connector;
               if( khaan )
               {
                  HandlePacketToKhaan( khaan, dataPacket );// all deletion and such is handled lower
               }
               handled = true;
            }
         }
         if( handled == false )
         {
            factory.CleanupPacket( dataPacket );
         }
      }

   /*   while( m_connectionsNeedingUpdate.size() > 0 )// this has the m_outputChainListMutex protection
      {
         int connectionId = m_connectionsNeedingUpdate.front();
         m_connectionsNeedingUpdate.pop_front();
         ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
         if( connIt != m_connectionMap.end() )
         {
            KhaanGateway* khaan = connIt->second.m_connector;
            if( khaan )
            {
               bool didFinish = khaan->Update();
               if( didFinish == false )
               {
                  moreTimeNeededQueue.push_back( connectionId );
               }
            }
         }
      }
      //
      m_connectionsNeedingUpdate = moreTimeNeededQueue; // copy */
   }
}

void  DiplodocusGateway::UpdateAllClientConnections()
{
   /*ConnectionIdQueue moreTimeNeededQueue;
   while( m_connectionsNeedingUpdate.size() > 0 )// this has the m_outputChainListMutex protection
   {
      int connectionId = m_connectionsNeedingUpdate.front();
      m_connectionsNeedingUpdate.pop_front();
      ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
      if( connIt != m_connectionMap.end() )
      {
         cout << "client updated" << endl;
         KhaanGateway* khaan = connIt->second.m_connector;
         if( khaan )
         {
            bool didFinish = khaan->Update();
            if( didFinish == false )
            {
               moreTimeNeededQueue.push_back( connectionId );
            }
            khaan->
         }
      }
   }
   //
   m_connectionsNeedingUpdate = moreTimeNeededQueue; // copy */

   if( m_connectionsNeedingUpdate.size() != 0 )
   {
      m_connectionsNeedingUpdate.clear();
   }

   ConnectionMapIterator connIt = m_connectionMap.begin();
   while( connIt != m_connectionMap.end() )
   {
      KhaanGateway* khaan = connIt->second.m_connector;
      if( khaan )
      {
         if( khaan->NeedsUpdate() == true )
         {
            bool didFinish = khaan->Update();
         }
      }
      connIt++;
   }
}

//-----------------------------------------------------------------------------------------

int   DiplodocusGateway::MainLoop_OutputProcessing()
{
   // mutex is locked already

   // lookup packet info and pass it back to the proper socket if we can find it.
   
   
   return 1;
}
//-----------------------------------------------------------------------------------------
