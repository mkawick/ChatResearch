// Diplodocus.h

#pragma once

#include <stdio.h>
#include <memory.h>
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
static const int DefaultSleepTimeForPacketHandlers = 33;

class BasePacket;
typedef Khaan                                      BaseInputChainHandler;


//-----------------------------------------------------------------------------
// this intermediate class exiists solely to share a common lib and simplify inheritance.
class BasePacketChainHandler : public Threading::CChainedThread < BasePacket* >
{
protected:
   BasePacketChainHandler() : Threading::CChainedThread < BasePacket* >( false, DefaultSleepTimeForPacketHandlers, false ) {}
   static   struct event_base*   m_LibEventInstance;
};



//-----------------------------------------------------------------------------

template< typename InputChain = BaseInputChainHandler, typename OutputChain = BasePacketChainHandler >
class Diplodocus: public BasePacketChainHandler
{
public:
   typedef InputChain                                       InputChainType;
   typedef typename std::list< InputChainType* >::iterator  InputChainIteratorType;
   typedef OutputChain                                      OutputChainType;
   typedef typename std::list< OutputChainType* >::iterator OutputChainIteratorType;

public:
	Diplodocus( string serverName, U32 serverId, ServerType type );
	virtual ~Diplodocus();
   void           SetAsControllerApp( bool isController = true ) { m_isControllerApp = isController ; }
   void           SetAsGame( bool isGame = true ) { m_isGame = isGame; }

   void           SetupListening( int port );
   bool           RequestUpdate( const string& connectionUuid ); // todo, update this

   static bool    Run(); // None of the networking starts until this is invoked. This is a blocking call. Be sure to keep a thread running to call the exit.
   static bool    ExitApp();

   bool           PushInputEvent( ThreadEvent* );
   void           NotifyFinishedRemoving( InputChainType* obj );

   bool           AddInputChainData( BasePacket* t, U32 filingData );
   bool           AddOutputChainData( BasePacket* t, U32 filingData );

   //---------------------------------------------

   virtual void   ClientConnectionFinishedAdding( InputChainType* khaan ) {}
   virtual void   ClientConnectionIsAboutToRemove( InputChainType* khaan ) {}
   virtual void   ServerWasIdentified( InputChainType* khaan ){}
   void           AddGatewayConnection( U32 id ) { m_connectionIdGateway = id; }
   virtual bool   HandleCommandFromGateway( BasePacket* packet, U32 connectionId ) { return false; }

protected:

   static   bool     m_isNetworkingEnabled;
   static   bool  InitializeNetworking();

public:
   static void    OnAccept( evconnlistener* listenerObj, evutil_socket_t  newConnectionId, sockaddr* address, int socklen, void* context );
   static void    OnSystemError( evconnlistener* listener, void* context );

protected:

   void           MarkAllConnectionsAsNeedingUpdate( BaseOutputContainer& listOfClients );

   typedef typename std::map< U32, InputChainType* >        ClientMap;// posssibly change this to a hash lookup once the client is logged in
   typedef typename ClientMap::iterator                     ClientMapIterator;
   typedef  pair< U32, InputChainType* >                    ClientLookup;
   
   ClientMap                                                m_connectedClients; // this is a duplicate of the chained list, but with mapping
   deque< U32 >                                             m_clientsNeedingUpdate;

   bool              m_isListeningWorking;
   bool              m_hasSentServerIdentification;
   bool              m_isControllerApp;
   bool              m_isGateway;
   bool              m_isGame;
   int               m_listeningPort;
   evconnlistener*   m_listener;// libevent object
   U32               m_serverId; // just used for id
   ServerType        m_serverType;// just used for logging and topology purposes.
   U32               m_connectionIdGateway;
   string            m_serverName; // just used for id


   bool           SetupListeningSocket();
   void	         AddClientConnection( InputChainType* khaan );
   bool	         FindKhaan( const string& connectionName, InputChainType** connection );

   int            ProcessInputFunction();
   int            ProcessOutputFunction();

   void           SendServerIdentification();

   //-------------------------------------------
   void           UpdateAllConnections();
   void           RemoveOldConnections();
   void           AddNewConnections();	
};


///////////////////////////////////////////////////////////////
#include "Diplodocus.inl"
///////////////////////////////////////////////////////////////