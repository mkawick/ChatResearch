// DiplodocusGateway.cpp

#include "DiplodocusGateway.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/StatPacket.h"

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
}

DiplodocusGateway::~DiplodocusGateway()
{
}

//-----------------------------------------------------------------------------------------

U32      DiplodocusGateway::GetNextConnectionId()
{
   m_inputChainListMutex.lock();
   m_connectionIdTracker ++;
   if( m_connectionIdTracker >= ConnectionIdExclusion.low &&  m_connectionIdTracker <= ConnectionIdExclusion.high )
   {
      m_connectionIdTracker = ConnectionIdExclusion.high + 1;
   }
   U32 returnValue = m_connectionIdTracker;

   m_inputChainListMutex.unlock();
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

      //wrapper->serverType = ServerType_Chat;// we are cheating. we should look at the type of packet and route it appropriately.
      
      m_inputChainListMutex.lock();
      m_packetsToBeSentInternally.push_back( wrapper );
      m_inputChainListMutex.unlock();
      return true;
   }
   else
   {
      delete packet;// it dies here. we should log this and try to disconnect the user
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

   Threading::MutexLock locker( m_outputChainListMutex );
   m_connectionsNeedingUpdate.push_back( newId );
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

bool     DiplodocusGateway::PushPacketToProperOutput( BasePacket* packet )
{
   PrintDebugText( "PushPacketToProperOutput", 1 );

   BaseOutputContainer tempOutput;
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
   }
   
   return false;
}

//-----------------------------------------------------------------------------------------

int  DiplodocusGateway::ProcessInputFunction()
{
   // m_inputChainListMutex.lock   // see CChainedThread<Type>::CallbackFunction()
   CommonUpdate();

   SendStatsToLoadBalancer();
   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   CleanupOldConnections();

   RunHourlyAverages();

   if( m_packetsToBeSentInternally.size() == 0 )
      return 0;

   PrintDebugText( "ProcessInputFunction" );

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
         time_t connectionTime = khaanWrapper.m_connector->GetConnectionTime();

         totalNumSeconds += static_cast<float>( difftime( currentTime, connectionTime ) );
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
      int num = m_connectedClients.size();

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
      m_outputChainListMutex.lock();
      m_outputTempStorage.push_back( packet );
      m_outputChainListMutex.unlock();
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
   khaan->AddOutputChainData( packet );

   // this is a performance improvement to prevent duplicate entries in this deque.
   //Threading::MutexLock locker( m_mutex );
   ConnectionIdQueue::iterator it = m_connectionsNeedingUpdate.begin();
   while( it != m_connectionsNeedingUpdate.end() )
   {
      if( *it++ == connectionId )
         return;
   }
   m_connectionsNeedingUpdate.push_back( connectionId );
}

//-----------------------------------------------------------------------------------------

int   DiplodocusGateway::ProcessOutputFunction()
{
   // mutex is locked already

   // lookup packet info and pass it back to the proper socket if we can find it.
   if( m_connectionsNeedingUpdate.size() || m_outputTempStorage.size() )
   {
      PrintDebugText( "ProcessOutputFunction" );

      PacketFactory factory;
      while( m_outputTempStorage.size() )
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( m_outputTempStorage.front() );
         m_outputTempStorage.pop_front();
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

      ConnectionIdQueue moreTimeNeededQueue;
      while( m_connectionsNeedingUpdate.size() > 0 )// this has the m_outputChainListMutex protection
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
      m_connectionsNeedingUpdate = moreTimeNeededQueue; // copy 
   }
   return 1;
}
//-----------------------------------------------------------------------------------------
