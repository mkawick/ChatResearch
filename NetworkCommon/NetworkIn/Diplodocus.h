// Diplodocus.h

#pragma once

// needs
// main init socket should take an ipaddress and a port and begin listening for connection requests
// should route traffic to an outside ipaddress and port or another internal server... so we need a server_connection object
// connection objects need to allow a formatter object to reformat packets coming in

// needs a template definition
// concept of processing thread
// messaging between client connections and main thread
#include "Khaan.h"
#include "../DataTypes.h"
#include "../ServerType.h"
#include "../ChainedArchitecture/ChainedThread.h"
#include <map>

typedef string HashLookup;
HashLookup CreateHash( const string& );

class BasePacket;
typedef Khaan                                      BaseInputChainHandler;


//-----------------------------------------------------------------------------
// this intermediate class exiists solely to share a common lib and simplify inheritance.
class BasePacketChainHandler : public Threading::CChainedThread < BasePacket* >
{
protected:
   BasePacketChainHandler() : CChainedThread( false, 33, false ) {}
   static   struct event_base*   m_LibEventInstance;
};



//-----------------------------------------------------------------------------

template< typename InputChain = BaseInputChainHandler, typename OutputChain = BasePacketChainHandler >
class Diplodocus: public BasePacketChainHandler
{
public:
   typedef typename InputChain                              InputChainType;
   typedef typename std::list< InputChainType* >::iterator  InputChainIteratorType;

public:
	Diplodocus( string serverName, ServerType type );
	virtual ~Diplodocus();

   void           SetupListening( int port );
   bool           RequestUpdate( const string& connectionUuid ); // todo, update this

   static bool    Run(); // None of the networking starts until this is invoked. This is a blocking call. Be sure to keep a thread running to call the exit.
   static bool    ExitApp();

   bool           PushInputEvent( ThreadEvent* );
   void           NotifyFinishedRemoving( ChainedInterface* obj );

   bool           AddInputChainData( BasePacket* t, U32 filingData );
   bool           AddOutputChainData( BasePacket* t, U32 filingData );

   //---------------------------------------------

   virtual void   ClientConnectionFinishedAdding( InputChainType* khaan ) {}
   virtual void   ClientConnectionIsAboutToRemove( InputChainType* khaan ) {}

protected:

   static   bool     m_isNetworkingEnabled;
   static   bool  InitializeNetworking();

public:
   static void    OnAccept( evconnlistener* listenerObj, evutil_socket_t  newConnectionId, sockaddr* address, int socklen, void* context );
   static void    OnSystemError( evconnlistener* listener, void* context );

protected:

   void           MarkAllConnectionsAsNeedingUpdate( BaseOutputContainer& listOfClients );

   deque< int >                                             m_clientsNeedingUpdate;

   typedef typename std::map< int, InputChainType* >        ClientMap;// posssibly change this to a hash lookup once the client is logged in
   typedef typename ClientMap::iterator                     ClientMapIterator;
   ClientMap                                                m_connectedClients; // this is a duplicate of the chained list, but with mapping
   typedef typename pair< int, InputChainType* >            ClientLookup;

   bool              m_isListeningWorking;
   int               m_listeningPort;
   evconnlistener*   m_listener;// libevent object
   string            m_serverName;// just used for id
   ServerType        m_serverType;// jus used for logging and topology purposes.


   bool           SetupListeningSocket();
   void	         AddClientConnection( InputChainType* khaan );
   bool	         FindKhaan( const string& connectionName, InputChainType** connection );

   int            ProcessInputFunction();
   int            ProcessOutputFunction();

   //-------------------------------------------
   void           UpdateAllConnections();
   void           RemoveOldConnections();
   void           AddNewConnections();	
};


///////////////////////////////////////////////////////////////
#include "Diplodocus.inl"
///////////////////////////////////////////////////////////////