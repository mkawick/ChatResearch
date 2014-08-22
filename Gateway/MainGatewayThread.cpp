// MainGatewayThread.cpp

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/AnalyticsPacket.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Logging/server_log.h"
//#include "../NetworkCommon/ChainedArchitecture/ChainedInterface.h"

#include "FruitadensGateway.h"
#include "MainGatewayThread.h"
#include "ErrorCodeLookup.h"

//#define VERBOSE

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

MainGatewayThread::MainGatewayThread( const string& serverName, U32 serverId ) : Diplodocus< KhaanGateway > ( serverName, serverId, 0, ServerType_Gateway ), StatTrackingConnections(),
                                          m_highestNumSimultaneousUsersWatermark( 0 ),
                                          m_connectionIdTracker( 0 ),
                                          m_printPacketTypes( false ),
                                          m_printFunctionNames( false ),
                                          m_reroutePort( 0 ),
                                          m_isServerDownForMaintenence( false ),
                                          m_hasInformedConnectedClientsThatServerIsDownForMaintenence( false ),
                                          m_serverIsAvaitingLB_Approval( true ),
                                          m_scheduledMaintnenceEnd( 0 ),
                                          m_scheduledMaintnenceBegins( 0 ),
                                          m_connectionIdBeginningRange( 0 ),
                                          m_connectionIdCountIds( 0 )
{
   SetSleepTime( 16 );// 30 fps
   SetSendHelloPacketOnLogin( true );
   
   time( &m_timestampSendConnectionStatisics );
   m_timestampSendStatServerStatisics = m_timestampSendConnectionStatisics;
   m_timestampRequestConnectionIdBlocks = m_timestampSendConnectionStatisics + 5;// wait a few secs

   m_orderedOutputPacketHandlers.reserve( PacketType_Num );
/*
   int dbgFlag = ::_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);  
    dbgFlag |= _CRTDBG_ALLOC_MEM_DF;  
    dbgFlag |= _CRTDBG_CHECK_CRT_DF;  
    dbgFlag |= _CRTDBG_LEAK_CHECK_DF;  
    _CrtSetDbgFlag(dbgFlag); 
*/
}

MainGatewayThread::~MainGatewayThread()
{
}

void     MainGatewayThread::Init()
{
   OrderOutputs();
}

void   MainGatewayThread::NotifyFinishedAdding( IChainedInterface* obj ) 
{
   //obj->
   if( m_printFunctionNames )
   {
      FileLog( " NotifyFinishedAdding: added obj " );
   }
   //m_listOfInputs
} 

/////////////////////////////////////////////////////////////////////////////////

U32      MainGatewayThread::GetNextConnectionId()
{
   if( m_serverIsAvaitingLB_Approval == true )
      return 0;

   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::GetNextConnectionId" );
   }
   m_inputChainListMutex.lock();
   U32 returnValue = m_connectionIdTracker;
   m_inputChainListMutex.unlock();

   m_connectionIdTracker ++;

   if( m_connectionIdTracker >= m_connectionIdBeginningRange + m_connectionIdCountIds )
   {
      if( m_usableConnectionIds.size() == 0 )
      {
         RequestMoreConnectionIdsFromLoadBalancer();
         m_serverIsAvaitingLB_Approval = true;
         m_connectionIdBeginningRange= 0;
         m_connectionIdCountIds = 0;
      }
      else
      {
         m_mutex.lock();
         ConnectionIdStorage& storage = m_usableConnectionIds.front();
         m_usableConnectionIds.pop_front();
         m_mutex.unlock();
         m_connectionIdBeginningRange = storage.id;
         m_connectionIdCountIds = storage.countIds;
      }
   }

   return returnValue;
}

/////////////////////////////////////////////////////////////////////////////////

bool  MainGatewayThread::AddInputChainData( BasePacket* packet, U32 connectionId ) // coming from the client-side socket
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::AddInputChainData" );
   }
   if( m_printPacketTypes )
   {
      cout << "Packet to servers: " << (int)packet->packetType << ":" << (int)packet->packetSubType << endl;
   }
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
         int type = ( packet->packetType );
         const char* packetTypeName = GetPacketTypename( (PacketType)type );

         cout << "to servers Packet: " << packetTypeName << " " << type << ":" << (int)packet->packetSubType << endl;
      
        /* string printer = "Packet to servers: ";
         printer += packetTypeName;
         printer += ":";
         printer += (int)packet->packetSubType;
         FileLog( printer.c_str() );
         cout << printer.c_str() << endl;*/
      }

      Threading::MutexLock locker( m_inputChainListMutex );
      m_packetsToBeSentInternally.push_back( wrapper );
      if( m_printPacketTypes )
      {
         cout << "    Packet to servers: true" << endl;
      }
      return true;
   }
   else
   {
      PacketFactory factory;
      factory.CleanupPacket( packet );// it dies here. we should log this and try to disconnect the user
      if( m_printPacketTypes )
      {
         cout << "    Packet to servers: false" << endl;
      }
      return false;
   }
}

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::InputConnected( IChainedInterface * chainedInput )
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::InputConnected" );
   }
   KhaanGateway* khaan = static_cast< KhaanGateway* >( chainedInput );
   string currentTime = GetDateInUTC();

   string printer = "Accepted connection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   cout << printer << endl;
   if( m_printFunctionNames )
   {
      FileLog( printer.c_str() );
   }
   PrintDebugText( "** InputConnected" , 1 );
   if( IsGatewayReady() == false )
   {
      khaan->DenyAllFutureData();
      if( khaan->HasDisconnected() == false )
      {
         time_t currentTime;
         time( &currentTime );
         khaan->SetTimeForDeletion( currentTime );// todo, add to another list for updating "denied" connections
      }
      return;
   }
   U32 newId = GetNextConnectionId();
   m_connectionMap.insert( ConnectionPair( newId, khaan ) );

   khaan->SetConnectionId( newId );

   khaan->SetMainOutput( this );

   if( m_connectionsRequireAuthentication == false )
   {
      khaan->AuthorizeConnection();
   }

   U32 numCurrentConnections = m_connectedClients.size();
   if( numCurrentConnections > m_highestNumSimultaneousUsersWatermark )
   {
      m_highestNumSimultaneousUsersWatermark = numCurrentConnections;
   }
   cout << "Highest watermark = " << m_highestNumSimultaneousUsersWatermark << endl;

   //khaan->SendThroughLibEvent( true );
}

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::InputRemovalInProgress" );
   }
   KhaanGateway* khaan = static_cast< KhaanGateway* >( chainedInput );
   U32 connectionId = khaan->GetConnectionId();

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   cout << printer << endl;
   if( m_printFunctionNames )
   {
      FileLog( printer.c_str() );
   }

   PrintDebugText( "** InputRemovalInProgress" , 1 );

   // send notice to the login server
   // must be done before we clear the lists of ids
   PacketLogout* logout = new PacketLogout();
   logout->wasDisconnectedByError = true;
   AddInputChainData( logout, connectionId );
}

/////////////////////////////////////////////////////////////////////////////////
/*
void     MainGatewayThread::UpdateRemovedConnections()
{
   if( m_clientsWaitingToBeRemoved.size() == 0 )
      return;

   ClientList::iterator it = m_clientsWaitingToBeRemoved.begin();
   while( it != m_clientsWaitingToBeRemoved.end() )
   {
      KhaanGateway* khaan = static_cast< KhaanGateway* >( *it );
      it++;

      MarkConnectionForDeletion( khaan->GetConnectionId() );
   }
   // the connection will be removed later after a small period of time.
   m_clientsWaitingToBeRemoved.clear();
}*/

//-----------------------------------------------------

void     MainGatewayThread::CheckOnServerStatusChanges()
{
   m_outputChainListMutex.lock();
   BaseOutputContainer tempOutputContainer = m_listOfOutputs;
   m_outputChainListMutex.unlock();

   ChainLinkIteratorType itOutput = tempOutputContainer.begin();
   while( itOutput != tempOutputContainer.end() )
   {
      IChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      itOutput++;
      if( fruity->IsRecentlyDisconnected() == true )
      {
         fruity->ClearRecentlyDisconnectedFlag();
         ServerType serverType = fruity->GetConnectedServerType();

         // some services are important, some are not
         if( 
            serverType == ServerType_Gateway || 
            serverType == ServerType_Login ||
            //serverType == ServerType_Tournament ||
            serverType == ServerType_Chat ||
            serverType == ServerType_Contact ||
            serverType == ServerType_Purchase ||
            serverType == ServerType_Notification 
            )
         {
            BroadcastPacketToAllUsers( "Server is down", Packet_QOS_ReportToClient::ErrorState_ServerIsNotAvailable, serverType, 0, 0 );
         }
         else if( serverType == ServerType_GameInstance ) // special case.
         {
            BroadcastPacketToAllUsers( "Server is down", Packet_QOS_ReportToClient::ErrorState_GameIsDown, serverType, fruity->GetGameProductId(), fruity->GetGameProductId() );
         }
      }
      else if( fruity->IsRecentlyConnected() == true )
      {
         fruity->ClearRecentlyConnectedFlag();
         ServerType serverType = fruity->GetConnectedServerType();

         if( 
            serverType == ServerType_Gateway || 
            serverType == ServerType_Login ||
            //serverType == ServerType_Tournament ||
            serverType == ServerType_Chat ||
            serverType == ServerType_Contact ||
            serverType == ServerType_Purchase ||
            serverType == ServerType_Notification 
            )
         {
            BroadcastPacketToAllUsers( "Server is up", Packet_QOS_ReportToClient::ErrorState_ServerIsAvailable, serverType, 0, 0 );
         }
         else if( serverType == ServerType_GameInstance ) // special case.
         {
            BroadcastPacketToAllUsers( "Server is up", Packet_QOS_ReportToClient::ErrorState_GameIsUp, serverType, fruity->GetGameProductId(), fruity->GetGameProductId() );
         }
      }
   }

   
   //---------------------------------------------------
   if( m_isServerDownForMaintenence == true && 
      m_hasInformedConnectedClientsThatServerIsDownForMaintenence == false)
   {
   }
}

//-----------------------------------------------------

void     MainGatewayThread::OutputConnected( IChainedInterface * chainPtr )
{
   FruitadensGateway* fruity = static_cast< FruitadensGateway* >( chainPtr );
   ServerType serverType = fruity->GetConnectedServerType();
}

void     MainGatewayThread::OutputRemovalInProgress( IChainedInterface * chainPtr )
{
   FruitadensGateway* fruity = static_cast< FruitadensGateway* >( chainPtr );
   ServerType serverType = fruity->GetConnectedServerType();
}

//-----------------------------------------------------

void     MainGatewayThread::BroadcastPacketToAllUsers( const string& errorText, int errorState, int param1, int param2, U8 matchingGameId )
{
   //m_inputChainListMutex.lock();
   Threading::MutexLock locker( m_inputChainListMutex );
   ConnectionMapIterator nextIt = m_connectionMap.begin();
   while( nextIt != m_connectionMap.end() )
   {
      ConnectionMapIterator connIt = nextIt++;
     // KhaanGatewayWrapper& khaanWrapper = connIt->second;
      KhaanGateway* khaan = connIt->second;
      if( khaan == NULL )
         continue;

      if( matchingGameId != 0 )
      {
         if( matchingGameId != khaan->GetLastGameConnectedTo() )
            continue;
      }

      Packet_QOS_ReportToClient* packet = new Packet_QOS_ReportToClient();
      packet->errorState = errorState;
      packet->errorText = errorText;
      packet->param1 = param1;
      packet->param2 = param2;

      HandlePacketToKhaan( khaan, packet );
   }
}
/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::PrintFunctionNames( bool printingOn ) 
{
   m_printFunctionNames = printingOn; 
   cout << "Gateway side function name printing is ";
   if( m_printFunctionNames == true )
   {
      cout << "enabled" << endl;
   }
   else
   {
      cout << "disabled" << endl;
   }
}


/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::PrintPacketTypes( bool printingOn ) 
{
   m_printPacketTypes = printingOn; 
   cout << "Gateway side packet printing is ";
   if( m_printPacketTypes == true )
   {
      cout << "enabled" << endl;
   }
   else
   {
      cout << "disabled" << endl;
   }
}

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::SetupReroute( const string& address, U16 port )
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::SetupReroute" );
   }
   m_rerouteAddress = address;
   m_reroutePort = port;

   // todo, add support for rerouting to other servers like login

   // todo, add dynamic rerouting allowing an admin to login and 
}

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::TrackCountStats( StatTracking stat, float value, int sub_category )
{
   StatTrackingConnections::TrackCountStats( m_serverName, m_serverId, stat, value, sub_category );
}

bool     MainGatewayThread::IsGatewayReady() const
{
   if( m_serverIsAvaitingLB_Approval )
      return false;
   if( m_isServerDownForMaintenence )
      return false;

   return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool     MainGatewayThread::OrderOutputs()
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::OrderOutputs" );
   }

   if( m_orderedOutputPacketHandlers.size() > 0 )
      return false;

   for( int packetType=PacketType_Base; packetType< PacketType_Num; packetType++ )
   {
      m_orderedOutputPacketHandlers.push_back( OutputConnectorList() );
      OutputConnectorList& listOfOutputs = m_orderedOutputPacketHandlers[ packetType ];

      m_outputChainListMutex.lock();
      BaseOutputContainer tempContainer = m_listOfOutputs;
      m_outputChainListMutex.unlock();

      ChainLinkIteratorType itOutput = tempContainer.begin();
      while( itOutput != tempContainer.end() )
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

/////////////////////////////////////////////////////////////////////////////////

bool     MainGatewayThread::PushPacketToProperOutput( BasePacket* packet )
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::PushPacketToProperOutput" );
   }

   U32 packetType = packet->packetType;
   U32 packetSubType = packet->packetSubType;
   if( packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      packetType = wrapper->pPacket->packetType;
      packetSubType = wrapper->pPacket->packetSubType;
   }

   assert( packetType < m_orderedOutputPacketHandlers.size() );

   OutputConnectorList& listOfOutputs = m_orderedOutputPacketHandlers[ packetType ];
   //assert( listOfOutputs.size() > 0 ); // this should be where we look through our list for a match

   if( listOfOutputs.size() == 0 )
   {
      cout << " *** packet received with which we cannot deal. ***" << endl;
      cout << "     type: " << packetType << endl;
      cout << "     sub type: " << packetSubType << endl;

      
      string typeString = "     type: " + packetType;
      string subTypeString = "     sub type: " + packetSubType;
      FileLog( " *** packet received with which we cannot deal. ***" );
      FileLog( typeString.c_str() );
      FileLog( subTypeString.c_str() );
      return false;
   }

   OutputConnectorList::iterator it = listOfOutputs.begin();
   while( it != listOfOutputs.end() )
   {
      FruitadensGateway* fruity = *it++;
      U32 unusedParam = -1;
      if( fruity->AddOutputChainData( packet, unusedParam ) == true )
      {
         return true;
      }
   }
 
   
   return false;
}

/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::SortOutgoingPackets()
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::SortOutgoingPackets" );
   }

   m_inputChainListMutex.lock();
   PacketQueue localQue = m_packetsToBeSentInternally;
   m_packetsToBeSentInternally.clear();
   m_inputChainListMutex.unlock();

   PacketFactory factory;
   while( localQue.size() )
   {
      BasePacket* packet = localQue.front();
      if( PushPacketToProperOutput( packet ) == false )
      {
         factory.CleanupPacket( packet );
      }
      localQue.pop_front();
   }
}

/////////////////////////////////////////////////////////////////////////////////

int       MainGatewayThread::CallbackFunction()
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::CallbackFunction" );
   }

   

   CommonUpdate();

   SendStatsToLoadBalancer();

   m_outputChainListMutex.lock();
   BaseOutputContainer tempContainer = m_listOfOutputs;
   m_outputChainListMutex.unlock();
   StatTrackingConnections::SendStatsToStatServer( tempContainer, m_serverName, m_serverId, m_serverType );
   

   CleanupOldConnections();
  
   //UpdateRemovedConnections();

   RunHourlyAverages();
   CheckOnConnectionIdBlocks();

   MoveClientBoundPacketsFromTempToKhaan();
   //UpdateAllClientConnections();

   UpdateAllConnections( "KhaanGateway" );

   //CheckOnServerStatusChanges();

   if( m_packetsToBeSentInternally.size() == 0 )
      return 0;

   SortOutgoingPackets();

   return 1;
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::CleanupOldConnections()
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::CleanupOldConnections" );
   }

   CleanupOldClientConnections( "KhaanGateway" );
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::RunHourlyAverages()
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::RunHourlyAverages" );
   }

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
         KhaanGateway* khaan = oldConnIt->second;
         if( khaan )
         {
            time_t connectionTime = khaan->GetConnectionTime();
            totalNumSeconds += static_cast<float>( difftime( currentTime, connectionTime ) );
         }
      }

      float averageNumSeconds = totalNumSeconds / numConnections;
      TrackCountStats( StatTracking_UserAverageTimeOnline, averageNumSeconds, 0 );
      TrackCountStats( StatTracking_UserTotalTimeOnline, totalNumSeconds, 0 );
      TrackCountStats( StatTracking_NumUsersOnline, numConnections, 0 );
   }
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::SendStatsToLoadBalancer()
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::SendStatsToLoadBalancer" );
   }

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampSendConnectionStatisics ) >= timeoutSendConnectionStatisics ) 
   {
      m_timestampSendConnectionStatisics = currentTime;
      int num = static_cast< int >( m_connectedClients.size() );

      TrackCountStats( StatTracking_UserTotalCount, static_cast<float>( num ), 0 );

      m_outputChainListMutex.lock();
      BaseOutputContainer tempContainer = m_listOfOutputs;
      m_outputChainListMutex.unlock();

      // we'll keep this because we may be connected to multiple load balancers
      bool statsSent = false;
      ChainLinkIteratorType itOutput = tempContainer.begin();
      while( itOutput != tempContainer.end() )
      {
         IChainedInterface* outputPtr = (*itOutput).m_interface;
         Fruitadens* fruity = static_cast< Fruitadens* >( outputPtr );
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
               PacketFactory factory;
               BasePacket* pPacket = packet;
               factory.CleanupPacket( pPacket );
            }
         }
         itOutput++;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::RequestNewConenctionIdsFromLoadBalancer()
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::RequestNewConenctionIdsFromLoadBalancer" );
   }

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampSendConnectionStatisics ) >= timeoutSendConnectionStatisics ) 
   {
      m_timestampSendConnectionStatisics = currentTime;
      int num = static_cast< int >( m_connectedClients.size() );

      TrackCountStats( StatTracking_UserTotalCount, static_cast<float>( num ), 0 );

      m_outputChainListMutex.lock();
      BaseOutputContainer tempContainer = m_listOfOutputs;
      m_outputChainListMutex.unlock();

      // we'll keep this because we may be connected to multiple load balancers
      bool statsSent = false;
      ChainLinkIteratorType itOutput = tempContainer.begin();
      while( itOutput != tempContainer.end() )
      {
         IChainedInterface* outputPtr = (*itOutput).m_interface;
         Fruitadens* fruity = static_cast< Fruitadens* >( outputPtr );
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
               PacketFactory factory;
               BasePacket* pPacket = packet;
               factory.CleanupPacket( pPacket );
            }
         }
         itOutput++;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////

bool  MainGatewayThread::AddOutputChainData( BasePacket* packetIn, U32 serverType )
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::AddOutputChainData" );
   }

   //PrintDebugText( "AddOutputChainData" ); 

   // pass through only
   if( packetIn->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );
      U32 id = wrapper->connectionId;

      //cout << "packet to client stored" << endl;
      //Threading::MutexLock locker( m_inputChainListMutex );
      m_inputChainListMutex.lock();
      m_clientBoundTempStorage.push_back( packetIn );
      m_inputChainListMutex.unlock();

      //AddClientConnectionNeedingUpdate( id );
      
   }
   else if( packetIn->packetType == PacketType_ServerToServerWrapper )
   {
      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packetIn );
      BasePacket* contentPacket = wrapper->pPacket;
      if( contentPacket->packetType == PacketType_ServerInformation && 
         contentPacket->packetSubType == PacketServerConnectionInfo::PacketServerIdentifier_GatewayRequestLB_ConnectionIdsResponse )
      {
         PacketServerToServer_GatewayRequestLB_ConnectionIdsResponse* response = 
            static_cast< PacketServerToServer_GatewayRequestLB_ConnectionIdsResponse* > ( contentPacket );

         cout << "Connection id block assigned = ( " << response->beginningId << ":" << response->countId << " )" << endl;
         if( m_connectionIdBeginningRange == 0 )// this is our working set
         {
            m_connectionIdBeginningRange = response->beginningId;

            if( m_connectionIdTracker == 0 )
            {
               // this will probably only ever happen once
               m_inputChainListMutex.lock();
               m_connectionIdTracker = m_connectionIdBeginningRange;
               m_inputChainListMutex.unlock();
            }
            m_connectionIdCountIds = response->countId;
         }
         else // this is our storage set.
         {
            m_mutex.lock();
            m_usableConnectionIds.push_back( ConnectionIdStorage( response->beginningId, response->countId ) );
            m_mutex.unlock();
         }
         m_serverIsAvaitingLB_Approval = false;
      }
      PacketFactory factory;
      factory.CleanupPacket( packetIn );
      return true;
   }
   else// the following really cannot happen.. but just in case
   {
      cout << "MainGatewayThread::AddOutputChainData packet not processed" << endl;
      int type = ( packetIn->packetType );
      const char* packetTypeName = GetPacketTypename( (PacketType)type );
      cout << "To client  packet: " << packetTypeName << " " << type << " :" << (int)packetIn->packetSubType << endl;
      PacketFactory factory;
      factory.CleanupPacket( packetIn );
      //assert( 0 );
      //return false;
   }
   return true;
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::CheckOnConnectionIdBlocks()
{
   time_t currentTime;
   time( &currentTime );

   int timeToWait = timeoutCheckOnConnectionIdBlocks;
   if( m_connectionIdBeginningRange == 0 ) // we may need a lock here
      timeToWait = 15;

   if( difftime( currentTime, m_timestampRequestConnectionIdBlocks ) >= timeToWait ) 
   {
      m_timestampRequestConnectionIdBlocks = currentTime;

      bool  requestMade = false;
      if( m_usableConnectionIds.size() < 2 )// we always want at least two blocks
      {
         requestMade = RequestMoreConnectionIdsFromLoadBalancer();
      }
      // apparently, we are not yet connected to the load balancer... set the time back so we can try again immediately.
      if( requestMade == false )
      {
         m_timestampRequestConnectionIdBlocks = 0;
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////

bool  MainGatewayThread::RequestMoreConnectionIdsFromLoadBalancer()
{
   m_outputChainListMutex.lock();
   BaseOutputContainer tempContainer = m_listOfOutputs;
   m_outputChainListMutex.unlock();

   ChainLinkIteratorType itOutput = tempContainer.begin();
   while( itOutput != tempContainer.end() )
   {
      IChainedInterface* outputPtr = (*itOutput).m_interface;
      Fruitadens* fruity = static_cast< Fruitadens* >( outputPtr );
      if( fruity->GetConnectedServerType() == ServerType_LoadBalancer )
      {
         PacketServerToServer_GatewayRequestLB_ConnectionIds* packet = new PacketServerToServer_GatewayRequestLB_ConnectionIds;
         packet->serverAddress = GetIpAddress();
         packet->serverId = GetServerId();

         U32 unusedParam = -1;
         if( fruity->AddOutputChainData( packet, unusedParam ) == false )
         {
            PacketFactory factory;
            BasePacket* pPacket = packet;
            factory.CleanupPacket( pPacket );
         }
         return true;
      }
      itOutput++;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////

// assuming that everything is thread protected at this point
void  MainGatewayThread::HandlePacketToKhaan( KhaanGateway* khaan, BasePacket* packet )
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::HandlePacketToKhaan" );
   }
     // cout << "Packet to client: " << (int)packet->packetType << ":" << (int)packet->packetSubType << endl;

   PrintDebugText( "HandlePacketToKhaan" );
   U32 connectionId = khaan->GetConnectionId();
   if( packet->packetType == PacketType_Login )
   { 
      if( packet->packetType == PacketLogin::LoginType_InformClientOfLoginStatus)
      {
         cout << "Stopping" << endl;
      }
      if( packet->packetSubType == PacketLogin::LoginType_InformGatewayOfLoginStatus )
      {
         BasePacket* tempPacket = HandlePlayerLoginStatus( khaan, packet );  
         PacketCleaner cleaner( packet );
         packet = tempPacket;
      }
      else if( packet->packetSubType == PacketLogin::LoginType_ThrottleUsersConnection )
      {
         PacketLoginThrottlePackets* throttler = static_cast< PacketLoginThrottlePackets* >( packet );
         khaan->ThrottleConnection( throttler->delayBetweenPacketsMs );
         PacketCleaner cleaner( packet );
      }
   }

   if( packet ) // this may have been cleaned up.
   {
      if( m_printPacketTypes )
      {
         int type = ( packet->packetType );
         const char* packetTypeName = GetPacketTypename( (PacketType)type );
         cout << "To client  packet: " << packetTypeName << " " << type << " :" << (int)packet->packetSubType << endl;
      }
      if( packet->packetType == PacketType_ErrorReport )
      {
         PacketErrorReport* error = static_cast< PacketErrorReport* >( packet );
         error->text = ErrorCodeLookup::GetString( error->errorCode );
      }
      khaan->AddOutputChainData( packet );
   }
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::MarkConnectionForDeletion( U32 connectionId )
{
   PrintDebugText( "HandlePacketToKhaan:: MarkForDeletion", 2 );
   
   TrackCountStats( StatTracking_ForcedDisconnect, 1, 0 );

   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {

      ChainedType::SetupClientConnectionForDeletion( it->second );
   }
/*
   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {
      //PacketLogoutToClient* logoutPacket = new PacketLogoutToClient;
      //logoutPacket->wasDisconnectedByError = true;
      //packet = logoutPacket;

      Khaan* khaan = it->second;
      if( khaan )
      {
         khaan->DenyAllFutureData();//
         //khaanWrapper.m_connector->AddOutputChainData( logoutPacket );

         int connectionId = khaan->GetConnectionId();
         int socketId = khaan->GetSocketId();
         khaan->RemoveOutputChain( this );

         if( khaan->HasDisconnected() == false )
         {
            time_t currentTime;
            time( &currentTime );
            khaan->SetTimeForDeletion( currentTime );
         }
      }      
   }*/
}

/////////////////////////////////////////////////////////////////////////////////

BasePacket*  MainGatewayThread::HandlePlayerLoginStatus( KhaanGateway* khaan, BasePacket* packet )
{
   U32 connectionId = khaan->GetConnectionId();
   
   PacketLoginToGateway* finishedLogin = static_cast< PacketLoginToGateway* >( packet );
   if( finishedLogin->wasLoginSuccessful )
   {
      khaan->AuthorizeConnection();
      khaan->SetAdminLevelOperations( finishedLogin->adminLevel );
      khaan->SetLanguageId( finishedLogin->languageId );

      PacketLoginToClient* clientNotify = new PacketLoginToClient;
      clientNotify->wasLoginSuccessful = finishedLogin->wasLoginSuccessful;
      clientNotify->uuid = finishedLogin->uuid;
      clientNotify->userName = finishedLogin->userName;
      clientNotify->lastLogoutTime = finishedLogin->lastLogoutTime;
      clientNotify->connectionId = connectionId;
      clientNotify->loginKey = finishedLogin->loginKey;
   /*   clientNotify->junk1 = 21;
      clientNotify->junk2 = "this is s version number test.";*/
      packet = clientNotify;

      TrackCountStats( StatTrackingConnections::StatTracking_UserLoginSuccess, 1, 0 );
      //packetHandled = true;
   }
   else
   {
      PacketLogoutToClient* logoutPacket = new PacketLogoutToClient;
      packet = logoutPacket;
      MarkConnectionForDeletion( connectionId );
      //packet = NULL;

      connectionId = 0;
   }

   return packet;
}


/////////////////////////////////////////////////////////////////////////////////

void  LogCertainPackets( BasePacket* packet )
{
   if( packet->packetType == PacketType_Chat && 
      packet->packetSubType == PacketChatToServer::ChatType_AddUserToChatChannelResponse )
   {
      PacketChatAddUserToChatChannelResponse* ptr = static_cast< PacketChatAddUserToChatChannelResponse* >( packet );
      cout << "PacketChatAddUserToChatChannelResponse begin" << endl;
      cout << " channel name: " << ptr->channelName << endl;
      cout << " channel uuid: " << ptr->channelUuid << endl;
      cout << " channel useruuid: " << ptr->userUuid << endl;
      cout << " channel user name: " << ptr->userName << endl;
      cout << "PacketChatAddUserToChatChannelResponse end" << endl;
   }
}

/////////////////////////////////////////////////////////////////////////////////
/*
void  MainGatewayThread::AddClientConnectionNeedingUpdate( U32 connectionId )
{
   return;
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::AddClientConnectionNeedingUpdate" );
   }

   ConnectionIdQueue::iterator it = m_connectionsNeedingUpdate.begin();
   while( it != m_connectionsNeedingUpdate.end() )
   {
      if( *it++ == connectionId )
         return;
   }
   m_connectionsNeedingUpdate.push_back( connectionId );
}*/

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::MoveClientBoundPacketsFromTempToKhaan()
{
   if( m_printFunctionNames )
   {
      FileLog( "MainGatewayThread::MoveClientBoundPacketsFromTempToKhaan" );
      FileLog( "MainLoop_OutputProcessing" );
   }

   m_inputChainListMutex.lock();
   std::deque< BasePacket* >  localQueue = m_clientBoundTempStorage;
   m_clientBoundTempStorage.clear();
   m_inputChainListMutex.unlock();

   if( localQueue.size() )
   {
      PrintDebugText( "MainLoop_OutputProcessing" );
      PacketFactory factory;
      while( localQueue.size() )
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( localQueue.front() );
         localQueue.pop_front();
         int connectionId = wrapper->connectionId;
         BasePacket* dataPacket = wrapper->pPacket;
         delete wrapper;
         bool  handled = false;

         LogCertainPackets( dataPacket );

         m_inputChainListMutex.lock();
         //SocketToConnectionMapIterator it = m_connectionToSocketMap.find( connectionId );
         //if( it != m_connectionToSocketMap.end() )
         {
            ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
            if( connIt != m_connectionMap.end() )
            {
               KhaanGateway* khaan = connIt->second;
               if( khaan && khaan->IsConnected() == true )
               {
                  HandlePacketToKhaan( khaan, dataPacket );// all deletion and such is handled lower
               }
               handled = true;
            }
         }
         m_inputChainListMutex.unlock();
         if( handled == false )
         {
            factory.CleanupPacket( dataPacket );
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////

