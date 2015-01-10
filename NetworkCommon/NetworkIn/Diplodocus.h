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
#include "../ChainedArchitecture/ChainedInterface.h"
#include "../UserAccount/UserAccountCommon.h"

#include <time.h>

#include <map>
#include <deque>

////////////////////////////////////////////////////////////////////////////////////////////

typedef string HashLookup;
HashLookup CreateHash( const string& );
void  EnableThreadingInLibEvent();

static const int DefaultSleepTimeForPacketHandlers = 33;

class BasePacket;
class PacketDbQueryResult;

typedef Khaan                                      BaseInputChainHandler;

struct DelayedPacket
{
   BasePacket* packet;
   U32         connectionId;
   U32         gatewayId;
   float       delay;
   time_t      beginTime;
};


//-----------------------------------------------------------------------------
// this intermediate class exists solely to share a common lib and simplify inheritance.
class BasePacketChainHandler : public Threading::CChainedThread < BasePacket* >
{
protected:
   BasePacketChainHandler() : Threading::CChainedThread < BasePacket* >( false, DefaultSleepTimeForPacketHandlers, false ) {}
   static   struct event_base*   m_LibEventInstance;
};

void  UpdateConsoleWindow( time_t& timeOfLastTitleUpdate, time_t uptime, int totalConnections, int numCurrentConnections, int listeningPort, const string& serverName );
void  LogConnectionInfo( time_t& timeOfLastTitleUpdate, time_t uptime, int numTotalConnections, int numCurrentConnections, int listeningPort, const string& serverName );


////////////////////////////////////////////////////////////////////////////////////////////


template< typename InputChain = BaseInputChainHandler, typename OutputChain = BasePacketChainHandler >
class Diplodocus: public BasePacketChainHandler
{
public:
   typedef InputChain                                       InputChainType;
   typedef typename std::list< InputChainType* >::iterator  InputChainIteratorType;
   typedef OutputChain                                      OutputChainType;
   typedef typename std::list< OutputChainType* >::iterator OutputChainIteratorType;

public:
   typedef Diplodocus< InputChain, OutputChain >   ChainedType;

public:
	Diplodocus( string serverName, U32 serverId, U8 gameProductId, ServerType type );
	virtual ~Diplodocus();

   virtual void   Init() {}

   //------------------------------------------------------------
   void           SetAsControllerApp( bool isController = true ) { m_isControllerApp = isController ; }
   bool           IsControllerApp() const { return m_isControllerApp; }
   void           SetAsGame( bool isGame = true ) { m_isGame = isGame; }
   bool           IsGameServer() const { return m_isGame; }
   void           SetGatewayType( U8 gateway_type ) { m_gatewayType = gateway_type; }
   U8             GetGatewayType() const { return m_gatewayType; }
   void           SetExternalIpAddress( const string& ipAddr ) { m_externalIpAddress = ipAddr; }
   string         GetExternalIpAddress() { return m_externalIpAddress; }

   void           RequireKeepAlive( bool isRequired ) { m_requiresKeepAlive = isRequired; }// onl works before the first connection

   const string&  GetServerName() const { return m_serverName; }
   U32            GetServerId() const { return m_serverId; }
   U8             GetGameProductId() const { return m_gameProductId; }
   const string&  GetIpAddress() const { return m_localIpAddress; }
   ServerType     GetServerType() const { return m_serverType; }

   void           SetupListening( int port );
   U16            GetPort() const { return m_listeningPort; }
   bool           RequestUpdate( const string& connectionUuid ); // todo, update this
   void           SetSendHelloPacketOnLogin( bool value ) { m_sendHelloPacket = value; }
   //void           SendStatsToStatServer();

   static bool    Run(); // None of the networking starts until this is invoked. This is a blocking call. Be sure to keep a thread running to call the exit.
   static bool    ExitApp();

   bool           PushInputEvent( ThreadEvent* );   

   bool           AddInputChainData( BasePacket* t, U32 filingData );
   bool           AddOutputChainData( BasePacket* t, U32 filingData );
   bool           SendPacketToGateway( BasePacket* packet, U32 connectionId, U32 gatewayId, float delayInSecs = 0 );
   bool           HandlePacketToOtherServer( BasePacket* packet, U32 connectionId );// not thread safe

   bool           SendErrorToClient( U32 connectionId, U32 gatewayId, PacketErrorReport::ErrorType error, int subType = 0 );
   bool           SendErrorToClient( const UserConnectionList& connections, PacketErrorReport::ErrorType error, int subType = 0 );

   void           NotifyFinishedRemoving( IChainedInterface* obj );
   virtual void   FinalRemoveInputChain( U32 connectionId ) {}
   
   void           InputReady( IChainedInterface * outputConnector );
   void           OutputReady( IChainedInterface * outputConnector );

   //---------------------------------------------

   virtual void   ServerWasIdentified( IChainedInterface* khaan ){}

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
   void           MarkConnectionAsNeedingUpdate( U32 connectionId );
   bool           DelayPacketToGateway( BasePacket* packet, U32 connectionId, U32 gatewayId, float delayInSecs );
   OutputChain*   FindNetworkOutLink( U32 serverId );

   virtual void   SetupClientConnectionForDeletion( InputChainType* chain );
   void           SetupClientWaitingToBeRemoved( InputChainType* chainedInput );
   virtual void   CleanupOldClientConnections( const char* connectionName );

   typedef typename std::map< U32, InputChainType* >        ClientMap;// posssibly change this to a hash lookup once the client is logged in
   typedef typename ClientMap::iterator                     ClientMapIterator;
   typedef typename std::pair< U32, InputChainType* >       ClientLookup;
   
   ClientMap                                                m_connectedClients; // this is a duplicate of the chained list, but with mapping
   std::deque< U32 >                                        m_clientsNeedingUpdate;

   deque< PacketDbQueryResult* >    m_dbQueries;
   deque< PacketStorage >           m_inputPacketsToBeProcessed;
   deque< PacketStorage >           m_outputPacketsToBeProcessed;

   list< DelayedPacket >                  m_delayedGatewayPackets;

   bool                                   m_isListeningWorking;
   bool                                   m_hasSentServerIdentification;
   bool                                   m_isControllerApp;
   U8                                     m_gatewayType;
   bool                                   m_isGame;
   //bool                                   m_updateGatewayConnections;
   bool                                   m_sendHelloPacket;
   string                                 m_localIpAddress;
   int                                    m_listeningPort;
   evconnlistener*                        m_listener;// libevent object
   U32                                    m_serverId; // just used for id
   U8                                     m_gameProductId;
   ServerType                             m_serverType;// just used for logging and topology purposes.
   U32                                    m_connectionIdGateway;
   string                                 m_serverName; // just used for id
   string                                 m_externalIpAddress;

   time_t                                 m_timeOfLastTitleUpdate;
   time_t                                 m_timeOfLastLoginInfoUpdate;
   time_t                                 m_uptime;
   int                                    m_numTotalConnections;
   bool                                   m_requiresKeepAlive;


   bool           SetupListeningSocket();
   void	         AddClientConnection( InputChainType* khaan );
   bool	         FindKhaan( const string& connectionName, InputChainType** connection );

   int            MainLoop_InputProcessing();
   int            MainLoop_OutputProcessing();

   void           UpdateInputPacketToBeProcessed();
   void           UpdateOutputPacketToBeProcessed();
   virtual bool   ProcessInboundPacket( PacketStorage& storage ) { return false; }
   virtual bool   ProcessOutboundPacket( PacketStorage& storage ) { return false; }
   int            CommonUpdate();

   void           SendServerIdentification();

   //-------------------------------------------
   void           UpdateQueuedConnections();
   void           UpdateAllConnections( const char* connectionName, bool clearPendingUpdate = true );
   void           RemoveOldConnections();
   void           AddNewConnections();	

   void           UpdatePendingGatewayPackets();
   //-------------------------- OnConnectFeature ----------------------------
public:
   struct OnConnect_PacketToBeSent
   {
      int packetType, packetSubType;
      int toWhichService;
      OnConnect_PacketToBeSent() : packetType( 0 ), packetSubType( 0 ), toWhichService( 0 ) {}
      OnConnect_PacketToBeSent( int type, int subtype, int service ) : packetType( type ), packetSubType( subtype ), toWhichService( service ) {}
   };
   bool           QueueOutboundRequest( int type, int subType, int whichService );
   bool           QueueInboundRequest( int type, int subType, int whichService );
   void           SendOutbound_OnConnectPackets( IChainedInterface * outputConnector, int serviceType );
   void           SendInbound_OnConnectPackets( IChainedInterface * inputConnector, int serviceType );
   vector< OnConnect_PacketToBeSent > m_outboundPacketsForNewConnections;
   vector< OnConnect_PacketToBeSent > m_inboundPacketsForNewConnections;
};


///////////////////////////////////////////////////////////////
#include "Diplodocus.inl"
///////////////////////////////////////////////////////////////
