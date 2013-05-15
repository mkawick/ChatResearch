
#include "Khaan.h"

#include <assert.h>

#include <event2/event.h>
#include <event2/event_struct.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/thread.h>
#include "../Packets/BasePacket.h"

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
      cout << "Socket startup failed with error " << endl;
      return false;
   }

   if( m_LibEventInstance == NULL )
   {
      // Initialize libevent.
      m_LibEventInstance = event_base_new();

      if( m_LibEventInstance == NULL ) 
      {
         return false;
      }
   }

#if PLATFORM == PLATFORM_WINDOWS
      // signal to the libevent system that we may be sending threaded signals to it.
	   // see http://stackoverflow.com/questions/7645217/user-triggered-event-in-libevent
	   evthread_use_windows_threads();
#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
      evthread_use_pthreads();
#endif

   m_isNetworkingEnabled = true;

   return true;

}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool        Diplodocus< InputChain, OutputChain >::SetupListeningSocket()
{
   struct sockaddr_in	ListenAddress;

   int socketSize = sizeof( ListenAddress);
   memset( &ListenAddress, 0, socketSize );

	SetupListenAddress( ListenAddress, m_listeningPort );
  
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
void  Diplodocus< InputChain, OutputChain >::NotifyFinishedRemoving( ChainedInterface* obj )
{
   InputChainType* connection = reinterpret_cast<InputChainType*>( obj );
   if( connection == NULL )
      return;

   LockMutex();
   int id = connection->GetSocketId();
   ClientMapIterator it = m_connectedClients.find( id );
   if( it == m_connectedClients.end() )
   {
      it = m_connectedClients.find( connection->GetConnectionId() );
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

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::AddInputChainData( BasePacket* t, U32 filingData )// this should be performed by the derived class
{
   ClientMapIterator foundItem = m_connectedClients.find( filingData );
   if( foundItem != m_connectedClients.end() )
   {
      InputChainType* connection = (*foundItem).second;
      // now we know who gave us this item along with the connection info. We can store it, pass it along, etc.
      delete t;
   }
   return false;
}

//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool  Diplodocus< InputChain, OutputChain >::AddOutputChainData( BasePacket* t, U32 filingData )
{
   ClientMapIterator foundItem = m_connectedClients.find( filingData );
   if( foundItem != m_connectedClients.end() )
   {
      InputChainType* connection = (*foundItem).second;
      // now we know who gave us this item along with the connection info. We can store it, pass it along, etc.
      //delete t;
   }

   delete t;

   return false;
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
Diplodocus< InputChain, OutputChain >::Diplodocus( string serverName, ServerType type ) : 
                                    m_isListeningWorking( false ),
                                    m_listeningPort( 0 ),
                                    m_serverName( serverName ), 
                                    m_serverType( type )
{
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
      bool success = true;
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
   m_connectedClients.insert(  ClientLookup ( client->GetConnectionId(), client ) );
   AddInputChain( client );
   UnlockMutex();

   client->RegisterToReceiveNetworkTraffic();

   ClientConnectionFinishedAdding( client );
   cout << "Accepted connection from " << inet_ntoa( client->GetIPAddress().sin_addr ) << endl;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
bool	Diplodocus< InputChain, OutputChain >::FindKhaan( const string& connectionName, InputChainType** connection )
{
	*connection = NULL;
	list <Khaan*>::iterator it = KhaanList.begin();
	while( it != KhaanList.end() )
	{
		if( (*it )->GetName() == connectionName )
		{
			*connection = *it;
			return true;
		}
		it++;
	}
	return false;
}

//---------------------------------------------------------------

template< typename InputChain, typename OutputChain >
void  Diplodocus< InputChain, OutputChain >::OnAccept( evconnlistener* listenerObj, evutil_socket_t newSocketId, sockaddr* ClientAddr, int socklen, void* context )
{
    // We got a new connection! Set up a bufferevent for it. 
    struct event_base*  base = evconnlistener_get_base( listenerObj );
    struct bufferevent* bufferEvent = bufferevent_socket_new( base, newSocketId, BEV_OPT_CLOSE_ON_FREE );

   InputChainType* khaan = new InputChainType( newSocketId, bufferEvent );
   khaan->SetIPAddress( *((struct sockaddr_in*)ClientAddr) );

   Diplodocus< InputChain, OutputChain >* This = (Diplodocus< InputChain, OutputChain > *) context;
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
      ChainLink& chainedInput = *itInputs++;
	   ChainedInterface* interfacePtr = chainedInput.m_interface;
      KhaanChat* khaan = reinterpret_cast< KhaanChat* >( interfacePtr );

      m_clientsNeedingUpdate.push_back( khaan->GetConnectionId() );
   }

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

   LockMutex();
   while( m_clientsNeedingUpdate.size() )// threads can remove themselves.
   {
      int id = m_clientsNeedingUpdate.front();      
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
   UnlockMutex();
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
int		Diplodocus< InputChain, OutputChain >::ProcessInputFunction()
{
   if( m_isNetworkingEnabled == false )
      return 1;

   return 0; 
}

//------------------------------------------------------------------------------------------

template< typename InputChain, typename OutputChain >
int      Diplodocus< InputChain, OutputChain >::ProcessOutputFunction()
{
   if( m_isNetworkingEnabled == false )
      return 1;

   UpdateAllConnections();
   //

   return 0; 
}

//------------------------------------------------------------------------------------------
