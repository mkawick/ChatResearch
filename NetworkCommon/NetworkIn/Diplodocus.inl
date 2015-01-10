
#include "Khaan.h"
#include "../NetworkOut/Fruitadens.h"

#include <assert.h>
#include "../NetworkUtils.h"
#include "../Packets/BasePacket.h"
#include "../Packets/PacketFactory.h"
#include "../Packets/PacketFactory.h"

#include "../Logging/server_log.h"

#if PLATFORM != PLATFORM_WINDOWS
#include <arpa/inet.h>
#include <stdarg.h>
#include <errno.h>
#endif

#ifndef cout
#include <iostream>
using namespace std;
#endif

void	   SetupListenAddress( struct sockaddr_in& ListenAddress, U16 ServerPort );

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

// static member initialization

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::m_isNetworkingEnabled = false;

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool        Diplodocus< InputChain, OutputChain >::InitializeNetworking()
{
   if( m_isNetworkingEnabled == true )
   {
     return true;
   }

   if ( InitializeSockets() == false ) {
      LogMessage( LOG_PRIO_ERR, "Socket startup failed with error " );
      return false;
   }

   if( m_LibEventInstance == NULL )
   {
      EnableThreadingInLibEvent();
      // Initialize libevent.
      m_LibEventInstance = event_base_new();

      if( m_LibEventInstance == NULL ) 
      {
         return false;
      }
   }

   m_isNetworkingEnabled = true;

   return true;

}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool        Diplodocus< InputChain, OutputChain >::SetupListeningSocket()
{
   struct sockaddr_in	ListenAddress;

   int socketSize = sizeof( ListenAddress );
   memset( &ListenAddress, 0, socketSize );

	SetupListenAddress( ListenAddress, m_listeningPort );

   const int MaxAddressLen = 256;
   char buffer[ MaxAddressLen ];
   GetLocalIpAddress( buffer, MaxAddressLen );
   m_localIpAddress = buffer;
  
   m_listener = evconnlistener_new_bind( m_LibEventInstance, 
         OnAccept, 
         this,
         LEV_OPT_CLOSE_ON_FREE | LEV_OPT_CLOSE_ON_EXEC | LEV_OPT_REUSEABLE, 
         -1,// no maximum number of backlog connections
         (struct sockaddr*)&ListenAddress, socketSize );

   if (!m_listener) {
          perror("Couldn't create listener");
          return false;
   }
   evconnlistener_set_error_cb( m_listener, OnSystemError );

   m_isListeningWorking = true;

   LogMessage( LOG_PRIO_INFO, " ******************************* " );
   LogMessage( LOG_PRIO_INFO, " >This server is now listening " );
   LogMessage( LOG_PRIO_INFO, "    name:              %s", m_serverName.c_str() );
   LogMessage( LOG_PRIO_INFO, "    id:                %u", m_serverId );
   LogMessage( LOG_PRIO_INFO, "    connection type:   %s  %d", GetServerTypeName( m_serverType ), m_serverType );
   LogMessage( LOG_PRIO_INFO, "    listening on port: %d", m_localIpAddress.c_str() );
   LogMessage( LOG_PRIO_INFO, "    listening on port: %d", m_listeningPort );
   LogMessage( LOG_PRIO_INFO, " ******************************* " );

	return true;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool     Diplodocus< InputChain, OutputChain >::Run()
{
   event_base* libEvent = m_LibEventInstance;

   if( libEvent == NULL )
   {
      LogMessage( LOG_PRIO_ERR, "Uninitialized networking. Please invoke SetupListening before Run" );
      return false;
   }

   LogMessage( LOG_PRIO_INFO, "\nNetworking is setup and working .. begin accepting connections" );
   // Start the event loop. this is a blocking call.
   event_base_dispatch( libEvent );

   return true;// since the above is blocking, this is unnecessary, but fixes the compiler warning
}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool     Diplodocus< InputChain, OutputChain >::ExitApp()
{
   event_base* libEvent = m_LibEventInstance;
   event_base_loopexit( libEvent, NULL );

   event_base_free( libEvent );

   ShutdownSockets();
   return true;
}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::PushInputEvent( ThreadEvent* te )
{
   int id = te->identifier;
   switch( te->type )
   {
   case ThreadEvent_NeedsService:
      MarkConnectionAsNeedingUpdate( id );
      return true;
   case ThreadEvent_DataAvailable:
      //MarkConnectionAsNeedingUpdate( id );
      return true;
   }

   return false;
}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::AddInputChainData( BasePacket* t, U32 filingData )// this should be performed by the derived class
{
   /*ClientMapIterator foundItem = m_connectedClients.find( filingData );
   if( foundItem != m_connectedClients.end() )
   {
      InputChainType* connection = (*foundItem).second;
      // now we know who gave us this item along with the connection info. We can store it, pass it along, etc.
      delete t;
   }*/
   delete t;
   return true;
}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::AddOutputChainData( BasePacket* t, U32 filingData )
{
  /* ClientMapIterator foundItem = m_connectedClients.find( filingData );
   if( foundItem != m_connectedClients.end() )
   {
      InputChainType* connection = (*foundItem).second;
      // now we know who gave us this item along with the connection info. We can store it, pass it along, etc.
      //delete t;
   }*/

   delete t;

   return true;
}

template< typename InputChain, typename OutputChain >
bool     Diplodocus< InputChain, OutputChain >::DelayPacketToGateway( BasePacket* packet, U32 connectionId, U32 gatewayId, float delayInSecs )
{
   DelayedPacket dp;
   dp.packet = packet;
   dp.connectionId = connectionId;
   dp.gatewayId = gatewayId;
   dp.delay = delayInSecs;
   time( &dp.beginTime );
   m_delayedGatewayPackets.push_back( dp );
   return true;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::CleanupOldClientConnections( const char* connectionName )
{
   // cleanup old connections
   time_t currentTime;
   time( &currentTime );

   list< InputChainType* > listOfInputsToBeDeleted;

   m_inputChainListMutex.lock();
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainLinkIteratorType oldConnIt = itInputs++;
      ChainLink & chainedInput = *oldConnIt;
      InputChainType* connection = static_cast< InputChainType* >( chainedInput.m_interface );
      if( connection->DoesNameMatch( connectionName ) )
      {
         //Khaan* khaan = connection;
         if( connection && connection->HasDisconnected() == true )
         {
            if( connection->HasDeleteTimeElapsed( currentTime ) == true )
            {
               listOfInputsToBeDeleted.push_back( connection );
               m_listOfInputs.erase( oldConnIt );
            }
         }
         
      }
   }
   m_inputChainListMutex.unlock();

   // this external loop exists because of the lock within a lock that linux hates
   list< InputChainType* >::iterator itDeletedInputs = listOfInputsToBeDeleted.begin();
   while( itDeletedInputs != listOfInputsToBeDeleted.end() )
   {
      InputChainType* khaan = *itDeletedInputs++;
      FinalRemoveInputChain( khaan->GetConnectionId() );
      
      khaan->RemoveOutputChain( this );
      khaan->CloseConnection();
      delete khaan;
   }
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::SetupClientWaitingToBeRemoved( InputChainType* chainedInput )
{
   SetupClientConnectionForDeletion( chainedInput );
}

//---------------------------------------------------------------

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::SetupClientConnectionForDeletion( InputChainType* chain )
{
   if( chain )
   {
      //Khaan* khaan = static_cast< Khaan* >( chain );
      chain->DenyAllFutureData();
      time_t currentTime;
      time( &currentTime );
      chain->SetTimeForDeletion( currentTime );

   }
}

//---------------------------------------------------------------

// this should always be invoked from within the thread space of the main thread.
template< typename InputChain, typename OutputChain >
bool     Diplodocus< InputChain, OutputChain >::SendPacketToGateway( BasePacket* packet, U32 connectionId, U32 gatewayId, float delayInSecs )
{
   if( delayInSecs > 0 )
   {
      return DelayPacketToGateway( packet, connectionId, gatewayId, delayInSecs );
   }

   PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper*> ( packet );
   if( packet->packetType != PacketType_GatewayWrapper )
   {
      wrapper = new PacketGatewayWrapper();
      wrapper->SetupPacket( packet, connectionId );
   }
 
   { // be very careful here... this extra effort was made to prevent locks inside of locks
      //Threading::MutexLock Locker( m_inputChainListMutex );

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )// only one output currently supported.
      {
         ChainLink & chainedInput = *itInputs++;
         InputChainType* connection = static_cast< InputChainType* >( chainedInput.m_interface );
         if( gatewayId && 
            connection->GetChainedType() == ChainedType_InboundSocketConnector && 
            connection->GetServerId() != gatewayId )
            continue;

         //cout << "Diplodocus::SendPacketToGateway" << endl;
         if( connection->AddOutputChainData( wrapper, connectionId ) == true )
         {
            MarkConnectionAsNeedingUpdate( connection->GetChainedId() );
            return true;
         }
      }
   }

   return false;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::HandlePacketToOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   ChainLinkIteratorType itInputs;
   if( connectionId != 0 )
      itInputs = FindInputConnection( connectionId );

   if( itInputs == m_listOfInputs.end() )
      itInputs = FindInputConnection( ServerToServerConnectionId );

   if( itInputs != m_listOfInputs.end() )
   {
      //cout << "Diplodocus::HandlePacketToOtherServer 1" << endl;
      ChainType* inputPtr = static_cast< ChainType*> ( itInputs->m_interface );
      if( inputPtr->AddOutputChainData( packet, connectionId ) == true )
      {
         return true;
      }
   }


   // going to the outputs should be rare, if ever.... s2s communications are handled entirely through inputs right now. 15Nov2013
   ChainLinkIteratorType itOutputs;
   if( connectionId != 0 )
      itOutputs = FindOutputConnection( connectionId );

   if( itOutputs == m_listOfOutputs.end() )
      itOutputs = FindOutputConnection( ServerToServerConnectionId );

   if( itOutputs != m_listOfOutputs.end() )
   {
      //cout << "Diplodocus::HandlePacketToOtherServer 2" << endl;
      ChainType* outputPtr = static_cast< ChainType*> ( itOutputs->m_interface );
      if( outputPtr->AddOutputChainData( packet, connectionId ) == true )
      {
         return true;
      }
   }
   return false;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::SendErrorToClient( U32 connectionId, U32 gatewayId, PacketErrorReport::ErrorType error, int subType )
{
   SendPacketToGateway( new PacketErrorReport( error, subType ), connectionId, gatewayId );
   return false;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::SendErrorToClient( const UserConnectionList& connectionList, PacketErrorReport::ErrorType error, int subType )
{
   UserConnectionList::const_iterator it = connectionList.begin();
   while( it != connectionList.end() )
   {
      SendPacketToGateway( new PacketErrorReport( error, subType ), it->connectionId, it->gatewayId );
      it++;
   }
   return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
Diplodocus< InputChain, OutputChain >::Diplodocus( string serverName, U32 serverId, U8 gameProductId, ServerType type ) : 
                                    m_isListeningWorking( false ),
                                    m_hasSentServerIdentification( false ),
                                    m_isControllerApp( false ),
                                    m_gatewayType( 0 ),
                                    m_isGame( false ),
                                    m_sendHelloPacket( true ),
                                    m_listeningPort( 0 ),
                                    m_listener( NULL ),
                                    
                                    m_serverId( serverId ),
                                    m_gameProductId( gameProductId ),
                                    m_serverType( type ),
                                    
                                    m_connectionIdGateway( 0 ),
                                    m_serverName( serverName ),
                                    m_numTotalConnections( 0 ),
                                    m_requiresKeepAlive( false )
{
   m_chainedType = ChainedType_MainThreadContainer;
   time( &m_timeOfLastTitleUpdate );
   m_uptime = m_timeOfLastTitleUpdate;
   m_timeOfLastLoginInfoUpdate = m_timeOfLastTitleUpdate;
}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
Diplodocus< InputChain, OutputChain >::~Diplodocus()
{
#if PLATFORM == PLATFORM_WINDOWS
   //closesocket( m_listeningSocket );
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX // is there something to do in linux?

   //close( m_listeningSocket );
#endif

   ExitApp();
}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::SetupListening( int port )
{
   if( m_listeningPort != 0 )
   {
      assert( 0 );
   }
   m_listeningPort = port;
   Pause();

   if( InitializeNetworking() == true )
   {
      if( SetupListeningSocket() == false )
      {
         assert( 0 );
         ExitApp();
      }

      Resume();
   }
   else
   {
      ShutdownSockets();
      event_base_free( m_LibEventInstance );
		return;
   }
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void	Diplodocus< InputChain, OutputChain >::AddClientConnection( InputChainType* client )
{
   //cout << "AddClientConnection:1" << endl;
   LockMutex();
   m_connectedClients.insert(  ClientLookup ( client->GetChainedId(), client ) );
   UnlockMutex();

   //cout << "AddClientConnection:2" << endl;
   AddInputChain( client );   

   client->RegisterToReceiveNetworkTraffic();
   InputChainType* connection = static_cast< InputChainType* >( client );
   connection->RequireKeepAlive( m_requiresKeepAlive );

   //cout << "AddClientConnection:3" << endl;
   InputConnected( client );
   //cout << "AddClientConnection:4" << endl;
   m_numTotalConnections ++;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool	Diplodocus< InputChain, OutputChain >::FindKhaan( const string& connectionName, InputChainType** connection )
{
	*connection = NULL;
	return false;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::OnAccept( evconnlistener* listenerObj, evutil_socket_t newSocketId, sockaddr* ClientAddr, int socklen, void* context )
{
   // We got a new connection! Set up a bufferevent for it. 
   struct event_base*  base = evconnlistener_get_base( listenerObj );
   struct bufferevent* bufferEvent = bufferevent_socket_new( base, newSocketId, BEV_OPT_CLOSE_ON_FREE );

   Diplodocus< InputChain, OutputChain >* This = static_cast< Diplodocus< InputChain, OutputChain > * >( context );

   SocketType socketId = static_cast< SocketType >( newSocketId );
   InputChainType* khaan = new InputChainType( socketId, bufferEvent );
   khaan->SetIPAddress( *((struct sockaddr_in*)ClientAddr) );
   khaan->SetPort( This->m_listeningPort );
   
   if( This->m_sendHelloPacket )
   {
      //PacketGatewayWrapper* wrapper    = new PacketGatewayWrapper;
      //wrapper->SetupPacket( new PacketHello(), -1 );

      khaan->AddOutputChainData( new PacketHello(), -1 );
   }

   This->AddClientConnection( khaan );
}
 
//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void Diplodocus< InputChain, OutputChain >::OnSystemError( evconnlistener* listenerObj, void* context )
{
    struct event_base *base = evconnlistener_get_base( listenerObj );
    int err = EVUTIL_SOCKET_ERROR();
    fprintf(stderr, "Got an error %d (%s) on the listener. "
            "Shutting down.\n", err, evutil_socket_error_to_string(err));
 
    event_base_loopexit( base, NULL );

    Diplodocus< InputChain, OutputChain >* This = (Diplodocus< InputChain, OutputChain > *) context;
    This->ExitApp();
}
 
//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::MarkAllConnectionsAsNeedingUpdate( BaseOutputContainer& listOfClients )
{
   deque<U32> localIds;
   m_inputChainListMutex.lock();
   {
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink & chainedInput = *itInputs++;
	      ChainedInterface< InputChain >* interfacePtr = chainedInput.m_interface;
         localIds.push_back( interfacePtr->GetChainedId() );
      }
   }
   m_inputChainListMutex.unlock();

   LockMutex();
      deque< U32 >::iterator it = localIds.begin();
      while( it != localIds.end() )
      {
         m_clientsNeedingUpdate.push_back( *it++ );
      }
   UnlockMutex();
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::MarkConnectionAsNeedingUpdate( U32 chainId )
{
   Threading::MutexLock locker( m_mutex );

   deque< U32 >::iterator it = m_clientsNeedingUpdate.begin();
   while( it != m_clientsNeedingUpdate.end() )
   {
      if( *it++ == chainId ) 
         return;
   }
   m_clientsNeedingUpdate.push_back( chainId );
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void	Diplodocus< InputChain, OutputChain >::AddNewConnections()
{
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void	Diplodocus< InputChain, OutputChain >::RemoveOldConnections()
{
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void	Diplodocus< InputChain, OutputChain >::UpdateQueuedConnections()
{
   // the potential exists for this queue to be updated while we are working on it
   // this is alright as long as we pull from the front and push onto the back as this list is non-persistent

   deque< U32 > localQueue;
   deque< U32 > updateQueue;

   {// creating local scope
      LockMutex();
      if( m_clientsNeedingUpdate.size() > 0 )// no locking a mutex if you don't need to do it.
      {
         localQueue = m_clientsNeedingUpdate;
         m_clientsNeedingUpdate.clear();
      }
      UnlockMutex();
   }

   
   while( localQueue.size() )// threads can remove themselves.
   {
      U32 id = localQueue.front();
      localQueue.pop_front();

      LockMutex();
      {
         ClientMapIterator it = m_connectedClients.end();
         if( m_connectedClients.size() )// preventing removal crashes.
         {
            it = m_connectedClients.find( id );
         }
         
         if( it != m_connectedClients.end() )
         {
            InputChainType* connection = it->second;
            if( connection->Update() == false )
            {
               //MarkConnectionAsNeedingUpdate( it->first );
               updateQueue.push_back( it->first );
            }
         }
      }
      UnlockMutex();
   }
   LockMutex();
      while( updateQueue.size() )
      {
         U32 id = updateQueue.front();
         updateQueue.pop_front();
         m_clientsNeedingUpdate.push_back( id );
      }
   UnlockMutex();
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void	Diplodocus< InputChain, OutputChain >::UpdateAllConnections( const char* connectionName, bool clearPendingUpdate )
{
   {
      m_inputChainListMutex.lock();

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )// only one output currently supported.
      {
         ChainLink & chainedInput = *itInputs++;
         InputChainType* connection = static_cast< InputChainType* >( chainedInput.m_interface );
         if( connection->IsConnected() == true && connection->DoesNameMatch( connectionName ) == true  )
         {
            connection->Update();
         }
      }
      m_inputChainListMutex.unlock();
   }

   if( clearPendingUpdate )
   {
      LockMutex();
      m_clientsNeedingUpdate.clear();
      UnlockMutex();
   }
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::UpdatePendingGatewayPackets()
{
   time_t currentTime;
   time( &currentTime );
   PacketFactory factory;
   list< DelayedPacket >::iterator it = m_delayedGatewayPackets.begin();
   while( it != m_delayedGatewayPackets.end() )
   {
      list< DelayedPacket >::iterator tempPtr = it++;
      DelayedPacket& dp = *tempPtr;
      
      if( difftime( currentTime, dp.beginTime ) >= dp.delay )
      {
         if( SendPacketToGateway( dp.packet, dp.connectionId, dp.gatewayId, 0 ) == false )
         {
            factory.CleanupPacket( dp.packet );
         }
         m_delayedGatewayPackets.erase( tempPtr );
      }
   }
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::SendServerIdentification()
{
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::NotifyFinishedRemoving( IChainedInterface* chainedOutput )
{
   if( chainedOutput == NULL )
      return;

   InputChainType* connection = static_cast<InputChainType*>( chainedOutput );

   LockMutex();
   {
      U32 id = connection->GetSocketId();
      ClientMapIterator it = m_connectedClients.find( id );
      if( it == m_connectedClients.end() )
      {
         it = m_connectedClients.find( connection->GetChainedId() );
      }
         
      if( it != m_connectedClients.end() )
      {
         m_connectedClients.erase( it );
      }

      // unnecessary.
    /*  int num = static_cast<int>( m_clientsNeedingUpdate.size() );
      for( int i=0; i<num; i++ )
      {
         if( m_clientsNeedingUpdate[i] == id )
         {
            m_clientsNeedingUpdate.erase( m_clientsNeedingUpdate.begin() + i );
            break;
         }
      }*/
   }
   UnlockMutex();
}

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::OutputReady( IChainedInterface * outputConnector ) 
{
   OutputChainType* output = static_cast< OutputChainType* >( outputConnector );
   if( output->GetChainedType() == ChainedType_OutboundSocketConnector )
   {
      Fruitadens* fruity = static_cast< Fruitadens* >( outputConnector );
      int serviceType = fruity->GetConnectedServerType();

      SendOutbound_OnConnectPackets( outputConnector, serviceType );
   }
}

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::InputReady( IChainedInterface * inputConnector ) 
{
   InputChainType* khaan = static_cast< InputChainType* >( inputConnector );
   int serviceType = khaan->GetServerType();

   SendInbound_OnConnectPackets( inputConnector, serviceType );
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::UpdateInputPacketToBeProcessed()
{
   if( m_inputPacketsToBeProcessed.size() > 0 )
   { 
      //LogMessage( LOG_PRIO_INFO, "Diplodocus::UpdateInputPacketToBeProcessed <<<" );
      m_mutex.lock();
      deque< PacketStorage > packetQueue = m_inputPacketsToBeProcessed;
      m_inputPacketsToBeProcessed.clear();
      m_mutex.unlock();

      //LogMessage( LOG_PRIO_INFO, "Diplodocus::UpdateInputPacketToBeProcessed ---" );

      deque< PacketStorage >::iterator it = packetQueue.begin();
      while( it != packetQueue.end() )
      {
         ProcessInboundPacket( *it++ );
      }
      //LogMessage( LOG_PRIO_INFO, "Diplodocus::UpdateInputPacketToBeProcessed >>>" );
   }
}
//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::UpdateOutputPacketToBeProcessed()
{
   if( m_outputPacketsToBeProcessed.size() > 0 )
   { 
      m_mutex.lock();
      deque< PacketStorage > packetQueue = m_outputPacketsToBeProcessed;
      m_outputPacketsToBeProcessed.clear();
      m_mutex.unlock();


      deque< PacketStorage >::iterator it = packetQueue.begin();
      while( it != packetQueue.end() )
      {
         ProcessOutboundPacket( *it++ );
      }
   }
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
int      Diplodocus< InputChain, OutputChain >::CommonUpdate()
{
   if( m_isNetworkingEnabled == false )
      return 1;

   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, static_cast<int>( m_connectedClients.size() ), m_listeningPort, m_serverName );

   LogConnectionInfo( m_timeOfLastLoginInfoUpdate, m_uptime, m_numTotalConnections, static_cast<int>( m_connectedClients.size() ), m_listeningPort, m_serverName );

   SendServerIdentification();

   return 0;
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
int      Diplodocus< InputChain, OutputChain >::MainLoop_InputProcessing()
{
   CleanupOldClientConnections( "khaan" );
   return CommonUpdate();
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
int      Diplodocus< InputChain, OutputChain >::MainLoop_OutputProcessing()
{
   CommonUpdate();

   if( m_isNetworkingEnabled == false )
      return 1;

   UpdateQueuedConnections();

   UpdatePendingGatewayPackets();

   return 0; 
}

////////////////////////////////////////////////////////////////////////

template <typename return_type, typename type >
return_type* PrepConnection( const string& remoteIpaddress, U16 remotePort, const string& remoteServerName, type* localServer, ServerType remoteServerType, bool requiresS2SWrapper, U32 gameProductId = 0 )
{
   string serverOutputText = localServer->GetServerName();
   serverOutputText += " to ";
   if( remoteServerName.size() )
   {
      serverOutputText += remoteServerName;
   }
   else
   {
      serverOutputText += "remote server";
   }
   return_type* serverOut = new return_type( serverOutputText.c_str() );
   serverOut->SetConnectedServerType( remoteServerType );
   serverOut->SetServerUniqueId( localServer->GetServerId() );

   serverOut->AddInputChain( localServer );

   //bool isGame = localServer->IsGameServer();
   //ServerType serverType = localServer->GetServerType();
   //U8 type = static_cast< U8 >( serverType );
   serverOut->NotifyEndpointOfIdentification( localServer->GetServerName(), localServer->GetIpAddress(), localServer->GetServerId(), 
                                             localServer->GetServerType(), localServer->GetPort(), 
                                             gameProductId, localServer->IsGameServer(), localServer->IsControllerApp(), requiresS2SWrapper, 
                                             localServer->GetGatewayType(), localServer->GetExternalIpAddress() );
   LogMessage( LOG_PRIO_INFO, "server (%s): (%s:%d)", remoteServerName.c_str(), remoteIpaddress.c_str(), remotePort );
   serverOut->Connect( remoteIpaddress.c_str(), remotePort );
   serverOut->Resume();

   return serverOut;
}

////////////////////////////////////////////////////////////////////////

class CommandLineParser;
template <typename return_type, typename type >
void  ConnectToMultipleGames( CommandLineParser& parser, type* localServer, bool requiresWrapper = true )
{
   vector< string > gamesConfiguration;
   if( parser.FindValue( "games", gamesConfiguration ) == false )
   {
      LogMessage( LOG_PRIO_ERR, "No games were listed. No connections will be made with any games" );
   }

   //U8    gameProductId = 0;
   LogMessage( LOG_PRIO_INFO, "games found = \n[\n" ); 
   vector< string >::iterator it = gamesConfiguration.begin();
   while( it != gamesConfiguration.end() )
   {
      string str = *it++;
      vector< string > values;
      if( parser.SeparateStringIntoValues( str, values, 3 ) == true )
      {
         LogMessage( LOG_PRIO_INFO, (boost::format("%15s ={ %6s - %-6s }")  % values[0] % values[1] % values[2]).str().c_str() );
         string gameAddress = values[0];
         string gameName = values[2];
         int port = 0;
         bool success = false;
         try
         {
            port = boost::lexical_cast<int>( values[1] );
            success = true;
         } 
         catch( boost::bad_lexical_cast const& ) 
         {
             LogMessage( LOG_PRIO_ERR, "Error: input string was not valid" );
         }
         if( success )
         {
            PrepConnection< return_type, type > ( gameAddress, port, gameName, localServer, ServerType_GameInstance, requiresWrapper );
         }
      }
      else
      {
         LogMessage( LOG_PRIO_ERR, "Not enough gamesConfiguration for this game: %s", str.c_str() );
      }
   }
   LogMessage( LOG_PRIO_INFO, "]" );
}

////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

template< typename InputChain, typename OutputChain >
bool      Diplodocus< InputChain, OutputChain >::QueueOutboundRequest( int type, int subtype, int whichService )
{
   Threading::MutexLock locker( m_mutex );

   int num = (int) m_outboundPacketsForNewConnections.size();
   for( int i=0; i< num; i++ )
   {
      const OnConnect_PacketToBeSent& connectPacketRequest = m_outboundPacketsForNewConnections[i];
      if( connectPacketRequest.packetType == type && // prevent duplicates
         connectPacketRequest.packetSubType == subtype &&
         connectPacketRequest.toWhichService == whichService )
         return false;
   }

   m_outboundPacketsForNewConnections.push_back( OnConnect_PacketToBeSent( type, subtype, whichService ) );
   return true;
}

/////////////////////////////////////////////////////////////////////////////////

template< typename InputChain, typename OutputChain >
bool     Diplodocus< InputChain, OutputChain >::QueueInboundRequest( int type, int subtype, int whichService )
{
   Threading::MutexLock locker( m_mutex );

   int num = (int) m_inboundPacketsForNewConnections.size();
   for( int i=0; i< num; i++ )
   {
      const OnConnect_PacketToBeSent& connectPacketRequest = m_inboundPacketsForNewConnections[i];
      if( connectPacketRequest.packetType == type && // prevent duplicates
         connectPacketRequest.packetSubType == subtype &&
         connectPacketRequest.toWhichService == whichService )
         return false;
   }

   m_outboundPacketsForNewConnections.push_back( OnConnect_PacketToBeSent( type, subtype, whichService ) );
   return true;
}

/////////////////////////////////////////////////////////////////////////////////

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::SendOutbound_OnConnectPackets( IChainedInterface * outputConnector, int serviceType )
{
   PacketFactory factory;

   OutputChainType* connection = static_cast< OutputChainType* >(outputConnector);
   cout << "LoginMainThread::SendOutbound_OnConnectPackets" << endl;
   cout << "serviceType = " << (int) serviceType << endl;

   int num = (int) m_outboundPacketsForNewConnections.size();
   for( int i=0; i< num; i++ )
   {
      const OnConnect_PacketToBeSent& connectPacketRequest = m_outboundPacketsForNewConnections[i];
      if( connectPacketRequest.toWhichService == serviceType || 
         connectPacketRequest.toWhichService == ServerType_All )
      {
         BasePacket* packet = NULL;
         if( factory.Create( connectPacketRequest.packetType, connectPacketRequest.packetSubType, &packet ) == true )
         {
            connection->AddOutputChainDataNoFilter( packet );
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////

template< typename InputChain, typename OutputChain >
void     Diplodocus< InputChain, OutputChain >::SendInbound_OnConnectPackets( IChainedInterface * inputConnector, int serviceType )
{
   PacketFactory factory;

   InputChainType* khaanS2S = static_cast< InputChainType* >( inputConnector );
   cout << "LoginMainThread::SendOutbound_OnConnectPackets" << endl;
   cout << "serviceType = " << (int) serviceType << endl;

   int num = (int) m_outboundPacketsForNewConnections.size();
   for( int i=0; i< num; i++ )
   {
      const OnConnect_PacketToBeSent& connectPacketRequest = m_outboundPacketsForNewConnections[i];
      if( connectPacketRequest.toWhichService == serviceType || 
         connectPacketRequest.toWhichService == ServerType_All )
      {
         BasePacket* packet = NULL;
         if( factory.Create( connectPacketRequest.packetType, connectPacketRequest.packetSubType, &packet ) == true )
         {
            BasePacket* wrapperToSend = NULL;
            PackageForS2S( m_serverId, m_gameProductId, packet, &wrapperToSend );
            khaanS2S->AddOutputChainData( wrapperToSend );
         }
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////

template< typename InputChain, typename OutputChain >
OutputChain *
   Diplodocus< InputChain, OutputChain >::FindNetworkOutLink( U32 serverId )
{
   Threading::MutexLock locker( m_outputChainListMutex );
   ChainLinkIteratorType itOutput = m_listOfOutputs.begin();
   while( itOutput != m_listOfOutputs.end() )
   {
      OutputChainType * fruity = static_cast< OutputChainType* >( (*itOutput).m_interface );
      itOutput++;
      if( fruity->GetConnectedServerId() == serverId )
      {
         return fruity;
      }
   }
   return NULL;
}

/////////////////////////////////////////////////////////////////////////////////
