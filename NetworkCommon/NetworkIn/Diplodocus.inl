
#include "Khaan.h"

#include <assert.h>
//#include "Platform.h"
#include "../Packets/BasePacket.h"
#include "../Packets/PacketFactory.h"
#include "../Packets/PacketFactory.h"

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
      std::cout << "Socket startup failed with error " << endl;
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

   cout << " ******************************* " << endl;
   cout << "New server instance listening " << endl;
   cout << "Server name: " << m_serverName << endl;
   cout << "Server id: " << m_serverId << endl;
   cout << "Server type: " << m_serverType << endl;
   cout << "Currently listening on port: " << m_listeningPort << endl;
   cout << " ******************************* " << endl;

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
      cout << "Uninitialized networking. Please invoke SetupListening before Run" << endl;
      return false;
   }

   cout << endl << "Networking is setup and working .. begin accepting connections" << endl;
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
      LockMutex();
      m_clientsNeedingUpdate.push_back( id );
      UnlockMutex();
      return true;
   case ThreadEvent_DataAvailable:
      //m_clientsNeedingUpdate.push_back( id );
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
bool     Diplodocus< InputChain, OutputChain >::DelayPacketToGateway( BasePacket* packet, U32 connectionId, float delayInSecs )
{
   DelayedPacket dp;
   dp.packet = packet;
   dp.connectionId = connectionId;
   dp.delay = delayInSecs;
   time( &dp.beginTime );
   m_delayedGatewayPackets.push_back( dp );
   return true;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool     Diplodocus< InputChain, OutputChain >::SendPacketToGateway( BasePacket* packet, U32 connectionId, float delayInSecs )
{
   if( delayInSecs > 0 )
   {
      return DelayPacketToGateway( packet, connectionId, delayInSecs );
   }
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   wrapper->SetupPacket( packet, connectionId );

   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainLink & chainedInput = *itInputs++;
      InputChainType* connection = static_cast< InputChainType* >( chainedInput.m_interface );
      if( connection->AddOutputChainData( wrapper, connectionId ) == true )
      {
         LockMutex();
         m_clientsNeedingUpdate.push_back( connection->GetChainedId() );
         UnlockMutex();
         return true;
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
bool  Diplodocus< InputChain, OutputChain >::SendErrorToClient( U32 connectionId, PacketErrorReport::ErrorType error, int subType )
{
   SendPacketToGateway( new PacketErrorReport( error, subType ), connectionId );
   return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
Diplodocus< InputChain, OutputChain >::Diplodocus( string serverName, U32 serverId, U8 gameProductId, ServerType type ) : 
                                    m_isListeningWorking( false ),
                                    m_hasSentServerIdentification( false ),
                                    m_isControllerApp( false ),
                                    m_isGateway( false ),
                                    m_isGame( false ),
                                    m_sendHelloPacket( true ),
                                    m_listeningPort( 0 ),
                                    m_listener( NULL ),
                                    
                                    m_serverId( serverId ),
                                    m_gameProductId( gameProductId ),
                                    m_serverType( type ),
                                    
                                    m_connectionIdGateway( 0 ),
                                    m_serverName( serverName ),
                                    m_numTotalConnections( 0 )
{
   time( &m_timeOfLastTitleUpdate );
   m_uptime = m_timeOfLastTitleUpdate;
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
   LockMutex();
   m_connectedClients.insert(  ClientLookup ( client->GetChainedId(), client ) );
   AddInputChain( client );
   UnlockMutex();

   client->RegisterToReceiveNetworkTraffic();

   InputConnected( client );
   m_numTotalConnections ++;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool	Diplodocus< InputChain, OutputChain >::FindKhaan( const string& connectionName, InputChainType** connection )
{
	*connection = NULL;
	/*list <Khaan*>::iterator it = KhaanList.begin();
	while( it != KhaanList.end() )
	{
		if( (*it )->GetName() == connectionName )
		{
			*connection = *it;
			return true;
		}
		it++;
	}*/
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

   InputChainType* khaan = new InputChainType( newSocketId, bufferEvent );
   khaan->SetIPAddress( *((struct sockaddr_in*)ClientAddr) );
   khaan->SetPort( This->m_listeningPort );
   
   if( This->m_sendHelloPacket )
   {
      PacketGatewayWrapper* wrapper    = new PacketGatewayWrapper;
      wrapper->SetupPacket( new PacketHello(), -1 );

      khaan->AddOutputChainData( wrapper, -1 );
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
   LockMutex();

   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainLink & chainedInput = *itInputs++;
	   ChainedInterface< InputChain >* interfacePtr = chainedInput.m_interface;
      Khaan* khaan = static_cast< Khaan* >( interfacePtr );

      m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
   }

   UnlockMutex();
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::MarkConnectionAsNeedingUpdate( int chainId )
{
   LockMutex();

   deque< U32 >::iterator it = m_clientsNeedingUpdate.begin();
   while( it != m_clientsNeedingUpdate.end() )
   {
      if( *it == chainId ) 
         return;
   }
   m_clientsNeedingUpdate.push_back( chainId );
   UnlockMutex();
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void	Diplodocus< InputChain, OutputChain >::AddNewConnections()
{
	/*KhaanIteratorType addIt = m_KhaansToAddList.begin();
	while( addIt != m_KhaansToAddList.end() )
	{
      Khaan* conn = *addIt;
      KhaanIteratorType temp = addIt++;// advance the pointer and save it 
      
      if( conn->GetName().size() != 0 )// once the user has logged in, we'll move this on
      {
         MutexLock();
		   KhaanList.push_back( conn );
         MutexUnlock();
         m_KhaansToAddList.erase( temp );
      }
	}

	m_KhaansToAddList.clear();*/
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void	Diplodocus< InputChain, OutputChain >::RemoveOldConnections()
{
  /* MutexLock();
	list <Khaan*>::iterator removeIt = m_KhaanToRemoveList.begin();
	while( removeIt != m_KhaanToRemoveList.end() )
	{
		bool wasRemoved = false; //It could be the case that a connection was added and instantly removed meaning
		// that we add it to both lists... we need to deal with this.
		list <Khaan*>::iterator it = m_KhaanList.begin();
		while( it != m_KhaanList.end() )
		{
			if( *it == *removeIt )
			{
				m_KhaanList.erase( it );
				wasRemoved = true;
				break;
			}
			it++;
		}
		if( wasRemoved )
		{
			delete *removeIt;
		}
		else
		{
			assert(0);// we need to remove these from the m_KhaansToAddList too
			list <Khaan*>::iterator addIt = m_KhaansToAddList.begin();
			while( addIt != m_KhaansToAddList.end() )
			{
				if( *addIt == *removeIt )
				{
					delete *addIt;// remove this rom both lists
					delete *removeIt;
					m_KhaansToAddList.erase( addIt );

					break;
				}
			}
		}
		removeIt ++;
	}
   MutexUnlock();

	m_KhaanToRemoveList.clear();*/
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void	Diplodocus< InputChain, OutputChain >::UpdateAllConnections()
{
   // the potential exists for this queue to be updated while we are working on it
   // this is alright as long as we pull from the front and push onto the back as this list is non-persistent

   Threading::MutexLock locker( m_mutex );
   if( m_clientsNeedingUpdate.size() == 0 )// no locking a mutex if you don't need to do it.
      return;

   
   while( m_clientsNeedingUpdate.size() )// threads can remove themselves.
   {
      U32 id = m_clientsNeedingUpdate.front();
      m_clientsNeedingUpdate.pop_front();

      ClientMapIterator it = m_connectedClients.end();
      if( m_connectedClients.size() )// preventing removal crashes.
      {
         it = m_connectedClients.find( id );
      }
      
      if( it != m_connectedClients.end() )
      {
         InputChainType* connection = it->second;
         connection->Update();
      }
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
         if( SendPacketToGateway( dp.packet, dp.connectionId, 0 ) == false )
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
   if( m_listOfOutputsNeedingToSendServerId.size() )
   {
      LockMutex();
      ChainLinkIteratorType   itOutputs = m_listOfOutputsNeedingToSendServerId.begin();
      while( itOutputs != m_listOfOutputsNeedingToSendServerId.end() )
      {
         ChainLink& chainedOutput = *itOutputs++;
         OutputChain* interfacePtr = static_cast<OutputChain*>( chainedOutput.m_interface );

         BasePacket* packet = NULL;
         PackageForServerIdentification( m_serverName, m_localIpAddress, m_serverId, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_isGateway, &packet );
         bool  accepted = interfacePtr->AddOutputChainData( packet, m_chainId );

         if( accepted == false )
         {
            delete packet;
         }
      }
      m_listOfOutputsNeedingToSendServerId.clear();
      UnlockMutex();
   }
}

//------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::OutputConnected( IChainedInterface * chainedOutput )
{
   if( chainedOutput == NULL )
      return;

   LockMutex();
   ChainType* ptr = static_cast< ChainType* >( chainedOutput );
   m_listOfOutputsNeedingToSendServerId.push_back( ptr );
   UnlockMutex();
}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::NotifyFinishedRemoving( IChainedInterface* chainedOutput )
{
   if( chainedOutput == NULL )
      return;

   InputChainType* connection = static_cast<InputChainType*>( chainedOutput );

   LockMutex();
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

   int num = m_clientsNeedingUpdate.size();
   for( int i=0; i<num; i++ )
   {
      if( m_clientsNeedingUpdate[i] == id )
      {
         m_clientsNeedingUpdate.erase( m_clientsNeedingUpdate.begin() + i );
         break;
      }
   }
   UnlockMutex();
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
int      Diplodocus< InputChain, OutputChain >::CommonUpdate()
{
   if( m_isNetworkingEnabled == false )
      return 1;

   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, m_connectedClients.size(), m_listeningPort, m_serverName );

   SendServerIdentification();

   return 0;
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
int      Diplodocus< InputChain, OutputChain >::ProcessInputFunction()
{
   return CommonUpdate();
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
int      Diplodocus< InputChain, OutputChain >::ProcessOutputFunction()
{
   CommonUpdate();

   if( m_isNetworkingEnabled == false )
      return 1;

   UpdateAllConnections();

   UpdatePendingGatewayPackets();

   return 0; 
}

//------------------------------------------------------------------------------------------

template< typename PacketType, typename Processor >
bool  SendRawData( const U8* data, int size, int dataType, int maxPacketSize, U32 serverId, U8 productId, const string& identifier, U32 connectionId, Processor* sender ) // diplodocus supposedly
{
   PacketFactory factory;
   const U8* workingData = data;
   int remainingSize = size;
   int numSends = remainingSize / maxPacketSize + 1;

   while( numSends > 0 )
   {
      int sizeToSend = remainingSize;
      if( sizeToSend > maxPacketSize )
      {
         sizeToSend = maxPacketSize;
      }

      PacketType* responsePacket = new PacketType();
            
      responsePacket->Prep( sizeToSend, workingData, numSends );
      responsePacket->identifier       = identifier;
      responsePacket->gameInstanceId   = serverId;
      responsePacket->gameProductId    = productId;
      responsePacket->dataType         = dataType;
      
      PacketGatewayWrapper* wrapper    = new PacketGatewayWrapper;
      wrapper->SetupPacket( responsePacket, connectionId );

      if( sender->AddOutputChainData( wrapper, connectionId ) == false )
      {
         BasePacket* tempPack = static_cast< BasePacket* >( wrapper );
         factory.CleanupPacket( tempPack );
         
         return false;
      }

      remainingSize -= sizeToSend;
      workingData += sizeToSend;// offset the pointer
      numSends --;
   }
   return true;
}

//------------------------------------------------------------------------------------------
