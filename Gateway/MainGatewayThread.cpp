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
#include "../NetworkCommon/Utils/StringUtils.h"
#include "../NetworkCommon/Logging/server_log.h"
//#include "../NetworkCommon/ChainedArchitecture/ChainedInterface.h"

#include "FruitadensGateway.h"
#include "MainGatewayThread.h"
#include "ErrorCodeLookup.h"
#include "ServiceAvailabilityManager.h"

//#define VERBOSE

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

MainGatewayThread::MainGatewayThread( const string& serverName, U32 serverId ) : 
                                          Diplodocus< KhaanGateway > ( serverName, serverId, 0, ServerType_Gateway ), 
                                          StatTrackingConnections(),
                                          m_highestNumSimultaneousUsersWatermark( 0 ),
                                          m_connectionIdTracker( 0 ),
                                          m_connectionIdBeginningRange( 0 ),
                                          m_connectionIdCountIds( 0 ),
                                          m_serviceAvailabilityManager( NULL ),
                                          m_printPacketTypes( false ),
                                          m_printFunctionNames( false ),
                                          m_connectionsRequireAuthentication( true ),
                                          m_isServerDownForMaintenence( false ),
                                          m_hasInformedConnectedClientsThatServerIsDownForMaintenence( false ),
                                          m_serverIsAvaitingLB_Approval( true ),
                                          m_scheduledMaintnenceBegins( 0 ),
                                          m_scheduledMaintnenceEnd( 0 ),                                          
                                          m_reroutePort( 0 )
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
   delete m_serviceAvailabilityManager;
}

void     MainGatewayThread::Init()
{
   OrderOutputs();
   m_serviceAvailabilityManager = new ServiceAvailabilityManager;
   m_serviceAvailabilityManager->SetServiceManager( this );
}

void   MainGatewayThread::NotifyFinishedAdding( IChainedInterface* obj ) 
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, " NotifyFinishedAdding: added obj " );
   }
} 

/////////////////////////////////////////////////////////////////////////////////

U32      MainGatewayThread::GetNextConnectionId()
{
   if( m_serverIsAvaitingLB_Approval == true )
      return 0;

   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   m_mutex.lock();
   U32 returnValue = m_connectionIdTracker;
   m_mutex.unlock();

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
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   if( m_printPacketTypes )
   {
      LogMessage( LOG_PRIO_INFO, "Packet to servers: %d:%d", (int)packet->packetType, (int)packet->packetSubType );
   }

   // just a quick test
   m_inputChainListMutex.lock();
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   m_inputChainListMutex.unlock();
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

         LogMessage( LOG_PRIO_INFO, "to servers Packet: %s %d:%d", packetTypeName, type, (int)packet->packetSubType );
      
        /* string printer = "Packet to servers: ";
         printer += packetTypeName;
         printer += ":";
         printer += (int)packet->packetSubType;
         LogMessage( LOG_PRIO_INFO, printer.c_str() );
         LogMessage( LOG_PRIO_INFO, printer.c_str() );*/
      }

      Threading::MutexLock locker( m_mutex );
      m_packetsToBeSentInternally.push_back( wrapper );
      if( m_printPacketTypes )
      {
         LogMessage( LOG_PRIO_INFO, "    Packet to servers: true" );
      }
      return true;
   }
   else
   {
      PacketFactory factory;
      factory.CleanupPacket( packet );// it dies here. we should log this and try to disconnect the user
      if( m_printPacketTypes )
      {
         LogMessage( LOG_PRIO_INFO, "    Packet to servers: false" );
      }
      return false;
   }
}

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::InputConnected( IChainedInterface * chainedInput )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   KhaanGateway* khaan = static_cast< KhaanGateway* >( chainedInput );
   string currentTime = GetDateInUTC();

   string printer = "Accepted connection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_INFO, printer.c_str() );
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, printer.c_str() );
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

   m_inputChainListMutex.lock();
   m_connectionMap.insert( ConnectionPair( newId, khaan ) );
   m_inputChainListMutex.unlock();

   khaan->SetConnectionId( newId );

   khaan->SetMainOutput( this );
   khaan->SetKeepAliveTimeoutInSeconds( 10 );// all other default to 5, but the gateway...

   if( m_connectionsRequireAuthentication == false )
   {
      khaan->AuthorizeConnection();
   }

   U32 numCurrentConnections = m_connectedClients.size();
   if( numCurrentConnections > m_highestNumSimultaneousUsersWatermark )
   {
      m_highestNumSimultaneousUsersWatermark = numCurrentConnections;
   }
   LogMessage( LOG_PRIO_INFO, "Highest watermark = %u", m_highestNumSimultaneousUsersWatermark );

   //khaan->SendThroughLibEvent( true );
}

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, "MainGatewayThread::InputRemovalInProgress" );
   }
   KhaanGateway* khaan = static_cast< KhaanGateway* >( chainedInput );
   U32 connectionId = khaan->GetConnectionId();
   connectionId = connectionId;

   SetupClientWaitingToBeRemoved( khaan );

   // performed in the main thread
   // deletion here means nothing.. the actual connection is stored in the base class
   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {
      m_connectionMap.erase( it );
   }

   string currentTime = GetDateInUTC();
   string printer = "MainGatewayThread::Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   //LogMessage( LOG_PRIO_ERR, printer.c_str() );
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_ERR, printer.c_str() );
   }

   LogMessage( LOG_PRIO_ERR, "InputRemovalInProgress exit" );

   // send notice to the login server
   // must be done before we clear the lists of ids
   /*PacketLogout* logout = new PacketLogout();
   logout->wasDisconnectedByError = true;
   AddInputChainData( logout, connectionId );*/
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

////////////////////////////////////////////////////////

void  MainGatewayThread::FinalRemoveInputChain( U32 connectionId )
{
   // this function is normally not invoked inside of a mutex.lock block.
   // We receive this callback when the time has expired and the connection
   // has been broken.
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, "MainGatewayThread::FinalRemoveInputChain" );
   }

   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {
      m_connectionMap.erase( it );
   }
}

////////////////////////////////////////////////////////

bool     MainGatewayThread::SendPacketToServer( BasePacket* packet, ServerType type )
{
   if( type == ServerType_GameInstance )
   {
      LogMessage( LOG_PRIO_ERR, "Gateway SendPacketToServer tried seding to a game server with no game id param " );
   }
   assert( type != ServerType_GameInstance );

   bool  sent = false;
   m_outputChainListMutex.lock();
   ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
   while( itOutput != m_listOfOutputs.end() )
   {
      IChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      itOutput++;
      if( fruity->GetConnectedServerType() == type )
      {
         if( fruity->IsConnected() == true )
         {
            U32 unusedParam = -1;
            sent = fruity->AddOutputChainData( packet, unusedParam );
         }
         break;
      }
   }
   m_outputChainListMutex.unlock();

   return sent;
}

////////////////////////////////////////////////////////

void     MainGatewayThread::CreateFilteredListOfClientConnections( U32 gameId, vector< U32 >& connectionIds )
{ 
   connectionIds.clear();
   connectionIds.reserve( 30 );

   Threading::MutexLock locker( m_inputChainListMutex );
   ConnectionMapIterator nextIt = m_connectionMap.begin();
   while( nextIt != m_connectionMap.end() )
   {
      ConnectionMapIterator connIt = nextIt++;
     // KhaanGatewayWrapper& khaanWrapper = connIt->second;
      KhaanGateway* khaan = connIt->second;
      U32 connectionId = connIt->first;
      if( khaan == NULL )
         continue;

      U32 gameConnected = khaan->GetLastGameConnectedTo ();
      if( gameId == 0 )// match all
      {
         connectionIds.push_back( connectionId );
      }
      else if( gameConnected == gameId ) // match a specific game
      {
         connectionIds.push_back( connectionId );
      }
   }
}

void     MainGatewayThread::CreateListOfClientConnectionsForGame( vector< ClientConnectionForGame >& ccfg )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   ccfg.clear();

   Threading::MutexLock locker( m_inputChainListMutex );
   ConnectionMapIterator nextIt = m_connectionMap.begin();
   while( nextIt != m_connectionMap.end() )
   {
      ConnectionMapIterator connIt = nextIt++;
      KhaanGateway* khaan = connIt->second;
      U32 connectionId = connIt->first;
      if( khaan == NULL )
         continue;
      U32 gameId = khaan->GetLastGameConnectedTo ();

      ccfg.push_back( ClientConnectionForGame( connectionId, gameId ) );

   }
}

////////////////////////////////////////////////////////
/*
void     MainGatewayThread::SendAllServerStateChangesToClients( const vector< QOS_ServiceChange >& listOfchanges )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   vector< QOS_ServiceChange >::const_iterator it = listOfchanges.begin();
   while( it != listOfchanges.end() )
   {
      const QOS_ServiceChange& change = *it++;

      BroadcastPacketToAllUsers( change.text, change.errorTypeMessageToSend, change.serverType, change.gameId, change.gameId, true );
      if( change.forceUsersToDc )
      {
         U32 gameId = change.gameId;
         vector< U32 > connectionIds;

         PacketLogin_LogoutAllUsers* packet = new PacketLogin_LogoutAllUsers;
         CreateFilteredListOfClientConnections( gameId, connectionIds );
         packet->connectionIds.copy( connectionIds );

         PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
         wrapper->SetupPacket( packet, -1 );

         // tell login server that users are being dc'd.. try to send a single message for a game if possible
         SendPacketToServer( wrapper, ServerType_Login );
         
         // tell all users of that game that they are being dc'd         
         vector< U32 >::iterator it = connectionIds.begin();
         while( it != connectionIds.end() )
         {
            CloseConnection( *it++ );
         }
      }
   }
}*/

////////////////////////////////////////////////////////

void  MainGatewayThread::CloseConnection( U32 connectionId )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   
   Threading::MutexLock locker( m_inputChainListMutex );
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      KhaanGateway* khaan = connIt->second;
      if( khaan == NULL )
         return;

      connIt++;

      PacketLogoutToClient logoutPacket;
      //khaan->AddOutputChainDataNoLock( logoutPacket );
      khaan->SendImmediately( &logoutPacket );
      MarkConnectionForDeletion( connectionId );
   }
}

////////////////////////////////////////////////////////
/*
void     MainGatewayThread::GetConnectedServerList( vector< ServerStatus >& servers, bool onlyDisconnected )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   m_outputChainListMutex.lock();
   BaseOutputContainer tempOutputContainer = m_listOfOutputs;
   m_outputChainListMutex.unlock();

   servers.clear();

   vector< QOS_ServiceChange > serviceChanges;
   ChainLinkIteratorType itOutput = tempOutputContainer.begin();
   while( itOutput != tempOutputContainer.end() )
   {
      IChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      itOutput++;

      ServerStatus serverStatus;
      serverStatus.type = fruity->GetConnectedServerType();
      serverStatus.gameId = fruity->GetConnectedGameProductId();
      serverStatus.isConnected = fruity->IsConnected();

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
}*/

////////////////////////////////////////////////////////

void     MainGatewayThread::GetListOfOutputs( list< FruitadensGateway* >& outputs )
{
   m_outputChainListMutex.lock();

   ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
   while( itOutput != m_listOfOutputs.end() )
   {
      IChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      if( fruity->IsReadyToTrack() != false )
      {
         outputs.push_back( fruity );
      }
      itOutput++;
   }

   m_outputChainListMutex.unlock();
}

////////////////////////////////////////////////////////
/*
void     MainGatewayThread::CheckOnServerStatusChanges()
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   m_outputChainListMutex.lock();
   BaseOutputContainer tempOutputContainer = m_listOfOutputs;
   m_outputChainListMutex.unlock();

   vector< QOS_ServiceChange > serviceChanges;
   ChainLinkIteratorType itOutput = tempOutputContainer.begin();
   while( itOutput != tempOutputContainer.end() )
   {
      IChainedInterface* outputPtr = (*itOutput).m_interface;
      FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
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
   if( m_isServerDownForMaintenence == true && 
      m_hasInformedConnectedClientsThatServerIsDownForMaintenence == false)
   {
   }
}*/

////////////////////////////////////////////////////////

void     MainGatewayThread::OutputConnected( IChainedInterface * chainPtr )
{
   FruitadensGateway* fruity = static_cast< FruitadensGateway* >( chainPtr );
   ServerType serverType = fruity->GetConnectedServerType();
   serverType = serverType;
}

void     MainGatewayThread::OutputRemovalInProgress( IChainedInterface * chainPtr )
{
   FruitadensGateway* fruity = static_cast< FruitadensGateway* >( chainPtr );
   ServerType serverType = fruity->GetConnectedServerType();
   serverType = serverType;
}

////////////////////////////////////////////////////////

void     MainGatewayThread::BroadcastPacketToAllUsers( const string& errorText, int errorState, int param1, int param2, U8 matchingGameId, bool sendImmediate )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
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

      if( sendImmediate == false )
         HandlePacketToKhaan( khaan, packet );
      else
      {
         m_inputChainListMutex.unlock();
         khaan->SendImmediately( packet );
         PacketFactory factory;
         BasePacket* tempPacket = static_cast< BasePacket* >( packet );
         factory.CleanupPacket( tempPacket );
      }
   }
}
/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::PrintFunctionNames( bool printingOn ) 
{
   m_printFunctionNames = printingOn; 
   LogMessage( LOG_PRIO_INFO, "Gateway side function name printing is " );
   if( m_printFunctionNames == true )
   {
      LogMessage( LOG_PRIO_INFO, "enabled" );
   }
   else
   {
      LogMessage( LOG_PRIO_INFO, "disabled" );
   }
}


/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::PrintPacketTypes( bool printingOn ) 
{
   m_printPacketTypes = printingOn; 
   LogMessage( LOG_PRIO_INFO, "Gateway side packet printing is " );
   if( m_printPacketTypes == true )
   {
      LogMessage( LOG_PRIO_INFO, "enabled" );
   }
   else
   {
      LogMessage( LOG_PRIO_INFO, "disabled" );
   }
}

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::SetupReroute( const string& address, U16 port )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
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
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
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

void     MainGatewayThread::LogUserOutIfKeyFeaturesAreUnavailable( ServerType type, U8 gameId, U32 connectionId )
{
   // this user who is logging in cannot go any further..
   Threading::MutexLock locker( m_inputChainListMutex );
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt != m_connectionMap.end() )
   {
      KhaanGateway* khaan = connIt->second;
      if( khaan == NULL )
         return;

      // BroadcastPacketToAllUsers( change.text, change.errorTypeMessageToSend, change.serverType, change.gameId, change.gameId, true );
      Packet_QOS_ReportToClient* packet = new Packet_QOS_ReportToClient();
      packet->errorState = type;
      packet->errorText = "Key services are not available";
      packet->param1 = gameId;
      packet->param2 = 0;
      khaan->SendImmediately( packet );

      PacketLogoutToClient logoutPacket;
      m_inputChainListMutex.unlock();
      khaan->SendImmediately( &logoutPacket );
      MarkConnectionForDeletion( connectionId );
   }
}

/////////////////////////////////////////////////////////////////////////////////

void     MainGatewayThread::InformUserOfMissingFeatures( const vector< ServerStatus >& servers, U32 connectionId, U8 gameId )
{
   if( servers.size() == 0 )
      return;

   LogMessage( LOG_PRIO_INFO, "InformUserOfMissingFeatures <<<" );

   Threading::MutexLock locker( m_inputChainListMutex );
   ConnectionMapIterator connIt = m_connectionMap.find( connectionId );
   if( connIt == m_connectionMap.end() )
   {
      return;
   }

   PacketLogin_ListOfMissingFeatures * packet = new PacketLogin_ListOfMissingFeatures;

   vector< ServerStatus >::const_iterator it = servers.begin();
   while( it != servers.end() )
   {
      ServerType type = it->type;
      if( type == ServerType_GameInstance )
      {
         if( it->gameId == gameId )
            packet->services.push_back( type );
      }
      else 
      {
         packet->services.push_back( type );
      }
      it++;
   }

   if( packet->services.size() )
   {
      m_inputChainListMutex.unlock();
      HandlePacketToKhaan( connIt->second, packet );
   }
   else
   {
      PacketFactory factory;
      BasePacket* tempPacket = static_cast< BasePacket* >( packet );
      factory.CleanupPacket( tempPacket );
   }
   LogMessage( LOG_PRIO_INFO, "InformUserOfMissingFeatures >>>" );
}

/////////////////////////////////////////////////////////////////////////////////

bool     MainGatewayThread::PreprocessServerBoundPackets( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_Login && 
      packet->packetSubType == PacketLogin::LoginType_Login )
   {
      U8 gameId = packet->gameProductId;

      // as part of login, we'll inform the client about which services are not available.
      m_serviceAvailabilityManager->InformUserAboutAvailableFeatures( gameId, connectionId );
   }
   return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool     MainGatewayThread::PushPacketToProperOutput( BasePacket* packet )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }

   U32 packetType = packet->packetType;
   U32 packetSubType = packet->packetSubType;
   U32 connectionId = 0;
   if( packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* packetToExamine = wrapper->pPacket;
      packetType = wrapper->pPacket->packetType;
      packetSubType = wrapper->pPacket->packetSubType;
      connectionId = wrapper->connectionId;
      if( PreprocessServerBoundPackets( packetToExamine, connectionId ) == false )
      {
         return false;
      }
   }

   assert( packetType < m_orderedOutputPacketHandlers.size() );

   OutputConnectorList& listOfOutputs = m_orderedOutputPacketHandlers[ packetType ];
   //assert( listOfOutputs.size() > 0 ); // this should be where we look through our list for a match

   if( listOfOutputs.size() == 0 )
   {
      LogMessage( LOG_PRIO_INFO, " *** packet received with which we cannot deal. ***" );
      LogMessage( LOG_PRIO_INFO, "     type: %d", packetType );
      LogMessage( LOG_PRIO_INFO, "     sub type: %d", packetSubType );

      //LogMessage( LOG_PRIO_INFO, " *** packet received with which we cannot deal. ***" );
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
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }

   m_mutex.lock();
   PacketQueue localQue = m_packetsToBeSentInternally;
   m_packetsToBeSentInternally.clear();
   m_mutex.unlock();

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
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
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

   UpdatedScheduledOutages();

   MoveClientBoundPacketsFromTempToKhaan();
   UpdateAllConnections( "KhaanGateway" );

   if( m_serviceAvailabilityManager )   
      m_serviceAvailabilityManager->Update();

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
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }

   CleanupOldClientConnections( "KhaanGateway" );
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::RunHourlyAverages()
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }

   Threading::MutexLock locker( m_inputChainListMutex );
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
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
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
      //bool statsSent = false;
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
               //statsSent = true;
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
/*
void  MainGatewayThread::RequestNewConenctionIdsFromLoadBalancer()
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, "MainGatewayThread::RequestNewConenctionIdsFromLoadBalancer" );
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
      //bool statsSent = false;
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
               //statsSent = true;
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
*/
/////////////////////////////////////////////////////////////////////////////////

bool  MainGatewayThread::AddOutputChainData( BasePacket* packetIn, U32 serverType )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }

   //PrintDebugText( "AddOutputChainData" ); 
   PacketFactory factory;

   // pass through only
   if( packetIn->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );
      BasePacket* contentPacket = wrapper->pPacket;
      //U32 id = wrapper->connectionId;

      if( contentPacket->packetType == PacketType_ServerInformation && 
         contentPacket->packetSubType == PacketServerConnectionInfo::PacketServerIdentifier_ServerOutageSchedule )
      {
         PacketServerConnectionInfo_ServerOutageSchedule* outages = 
            static_cast< PacketServerConnectionInfo_ServerOutageSchedule* > ( contentPacket );

         m_mutex.lock();
         m_scheduledOutages.push_back( outages );
         m_mutex.unlock();
         wrapper->pPacket = NULL;
         factory.CleanupPacket( packetIn );
         return true;
      }
      else
      {
      //LogMessage( LOG_PRIO_INFO, "packet to client stored" );
         m_mutex.lock();
         m_clientBoundTempStorage.push_back( packetIn );
         m_mutex.unlock();
      }
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

         LogMessage( LOG_PRIO_INFO, "Connection id block assigned = ( %d:%d )", response->beginningId, response->countId );
         if( m_connectionIdBeginningRange == 0 )// this is our working set
         {
            m_connectionIdBeginningRange = response->beginningId;

            if( m_connectionIdTracker == 0 )
            {
               // this will probably only ever happen once
               m_mutex.lock();
               m_connectionIdTracker = m_connectionIdBeginningRange;
               m_mutex.unlock();
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

      factory.CleanupPacket( packetIn );
      return true;
   }
   else// the following really cannot happen.. but just in case
   {
      LogMessage( LOG_PRIO_INFO, "MainGatewayThread::AddOutputChainData packet not processed" );
      int type = ( packetIn->packetType );
      const char* packetTypeName = GetPacketTypename( (PacketType)type );
      LogMessage( LOG_PRIO_INFO, "To client  packet: %s %d : %d", packetTypeName, type, (int)packetIn->packetSubType );
      
      factory.CleanupPacket( packetIn );
      //assert( 0 );
      //return false;
   }
   return true;
}

void  MainGatewayThread::UpdatedScheduledOutages()
{
   Threading::MutexLock locker ( m_mutex );
   if( m_scheduledOutages.size() )
   {
      std::deque< PacketServerConnectionInfo_ServerOutageSchedule* >::iterator 
            it = m_scheduledOutages.begin();
      while( it != m_scheduledOutages.end() )
      {
         PacketServerConnectionInfo_ServerOutageSchedule* packet = *it++;
         m_serviceAvailabilityManager->ScheduledOutages( packet );
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::CheckOnConnectionIdBlocks()
{
   time_t currentTime;
   time( &currentTime );

   int timeToWait = timeoutCheckOnConnectionIdBlocks;
   if( m_connectionIdBeginningRange == 0 ) // we may need a lock here
      timeToWait = 10;

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
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }

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
         cout << "MainGatewayThread::RequestMoreConnectionIdsFromLoadBalancer" << endl;
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
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
     // LogMessage( LOG_PRIO_INFO, "Packet to client: ", (int)packet->packetType, ":", (int)packet->packetSubType );

   U32 connectionId = khaan->GetConnectionId();
   connectionId = connectionId;

   if( packet->packetType == PacketType_Login )
   { 
      if( packet->packetType == PacketLogin::LoginType_InformClientOfLoginStatus)
      {
         LogMessage( LOG_PRIO_INFO, "Stopping" );
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
         LogMessage( LOG_PRIO_INFO, "To client  packet: %s %d:%d", packetTypeName, type, (int)packet->packetSubType );
      }
      if( packet->packetType == PacketType_ErrorReport )
      {
         PacketErrorReport* error = static_cast< PacketErrorReport* >( packet );
         error->text = ErrorCodeLookup::GetString( error->errorCode );
      }
      khaan->AddOutputChainDataNoLock( packet );
   }
}

/////////////////////////////////////////////////////////////////////////////////

void  MainGatewayThread::MarkConnectionForDeletion( U32 connectionId )
{
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }
   
   TrackCountStats( StatTracking_ForcedDisconnect, 1, 0 );

   ConnectionMapIterator it = m_connectionMap.find( connectionId );
   if( it != m_connectionMap.end() )
   {
      ChainedType::SetupClientConnectionForDeletion( it->second );
   }
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
      khaan->SetLastGameConnectedTo( finishedLogin->gameProductId );
      khaan->SetUserName( finishedLogin->userName );
      khaan->SetUserEmail( finishedLogin->userEmail );
      khaan->SetUserUuid( finishedLogin->uuid );

      PacketLoginToClient* clientNotify = new PacketLoginToClient;
      clientNotify->wasLoginSuccessful = finishedLogin->wasLoginSuccessful;
      clientNotify->uuid = finishedLogin->uuid;
      clientNotify->userName = finishedLogin->userName;
      clientNotify->lastLogoutTime = finishedLogin->lastLogoutTime;
      clientNotify->connectionId = connectionId;
      clientNotify->loginKey = finishedLogin->loginKey;
      packet = clientNotify;

      TrackCountStats( StatTrackingConnections::StatTracking_UserLoginSuccess, 1, 0 );

      U8 gameId = finishedLogin->gameProductId;
      m_serviceAvailabilityManager->InformUserAboutScheduledOutages( gameId, connectionId );
   }
   else
   {
      
      PacketLogoutToClient* logoutPacket = new PacketLogoutToClient;
      packet = logoutPacket;
      khaan->SendImmediately( logoutPacket );
      MarkConnectionForDeletion( connectionId );
      LogMessage( LOG_PRIO_INFO, "------- Login server message User logout ---------" );
      LogMessage( LOG_PRIO_INFO, "LoginType_Logout: %s", khaan->GetUserEmail() );
      LogMessage( LOG_PRIO_INFO, "            name: %s", khaan->GetUserName() );
      LogMessage( LOG_PRIO_INFO, "            uuid: %s", khaan->GetUserUuid() );
      LogMessage( LOG_PRIO_INFO, "--------------------------------------------------" );

      connectionId = 0;
   }

   return packet;
}

/////////////////////////////////////////////////////////////////////////////////

void  LogCertainPacketsBackToClient( BasePacket* packet )
{
  /* if( packet->packetType == PacketType_Chat && 
      packet->packetSubType == PacketChatToServer::ChatType_AddUserToChatChannelResponse )
   {
      PacketChatAddUserToChatChannelResponse* ptr = static_cast< PacketChatAddUserToChatChannelResponse* >( packet );
      LogMessage( LOG_PRIO_INFO, "PacketChatAddUserToChatChannelResponse begin" );
      LogMessage( LOG_PRIO_INFO, " channel name: %s", ptr->channelName.c_str() );
      LogMessage( LOG_PRIO_INFO, " channel uuid: %s", ptr->channelUuid.c_str() );
      LogMessage( LOG_PRIO_INFO, " channel useruuid: %s", ptr->userUuid.c_str() );
      LogMessage( LOG_PRIO_INFO, " channel user name: %s", ptr->userName.c_str() );
      LogMessage( LOG_PRIO_INFO, "PacketChatAddUserToChatChannelResponse end" );
   }*/
}

/////////////////////////////////////////////////////////////////////////////////
/*
void  MainGatewayThread::AddClientConnectionNeedingUpdate( U32 connectionId )
{
   return;
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, "MainGatewayThread::AddClientConnectionNeedingUpdate" );
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
      LogMessage( LOG_PRIO_INFO, __FUNCTION__ );
   }

  /* if( m_inputChainListMutex.IsLocked() )
   {
      LogMessage( LOG_PRIO_INFO, "m_inputChainListMutex is locked" );
   }
   if( m_outputChainListMutex.IsLocked() )
   {
      LogMessage( LOG_PRIO_INFO, "m_outputChainListMutex is locked" );
   }
   if( m_mutex.IsLocked() )
   {
      LogMessage( LOG_PRIO_INFO, "m_mutex is locked" );
   }*/
   m_mutex.lock();
   std::deque< BasePacket* >  localQueue = m_clientBoundTempStorage;
   m_clientBoundTempStorage.clear();
   m_mutex.unlock();

 /*  if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, "MainLoop_OutputProcessing" );
   }*/

   if( localQueue.size() )
   {
      //PrintDebugText( "MainLoop_OutputProcessing" );
      PacketFactory factory;
      while( localQueue.size() )
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( localQueue.front() );
         localQueue.pop_front();
         int connectionId = wrapper->connectionId;
         BasePacket* dataPacket = wrapper->pPacket;
         delete wrapper;
         bool  handled = false;

         LogCertainPacketsBackToClient( dataPacket );

         //SocketToConnectionMapIterator it = m_connectionToSocketMap.find( connectionId );
         //if( it != m_connectionToSocketMap.end() )
         {
            Threading::MutexLock lock( m_inputChainListMutex );
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
         if( handled == false )
         {
            factory.CleanupPacket( dataPacket );
         }
      }
   }
  /* if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, "MainGatewayThread::MoveClientBoundPacketsFromTempToKhaan exit" );
   }*/
}

/////////////////////////////////////////////////////////////////////////////////

