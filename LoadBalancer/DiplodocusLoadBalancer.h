#pragma once
// DiplodocusLoadBalancer.h

#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "KhaanConnector.h"
#include <deque>
#include <string>
using namespace std;
class BasePacket;

class PacketServerIdentifier;
class PacketServerDisconnect;
class PacketServerConnectionInfo;

///////////////////////////////////////////////////////////////////

struct GatewayInfo
{
   enum Type
   {
      Type_Normal,
      Type_Asset,
      Type_Other
   };

   U32      serverId;
   string   address;
   U16      port;
   Type     type;
   int      currentLoad;
   int      maxLoad;
   int      loadTolerance;
   bool     isConnected;
   bool     isVerified;
   bool     isPreferred;

   GatewayInfo( string addr, U16 p ) : address( addr ), port( p ), type( Type_Normal ), 
                                       serverId( 0 ),
                                       currentLoad( 0 ), 
                                       maxLoad( 450 ), 
                                       loadTolerance( 100 ), 
                                       isConnected( false ), 
                                       isVerified( false ) {}
};

///////////////////////////////////////////////////////////////////

class DiplodocusLoadBalancer :  public Diplodocus< KhaanConnector >
{
public:
   DiplodocusLoadBalancer( const string& serverName, U32 serverId );
   ~DiplodocusLoadBalancer();

   void     AddGatewayAddress( const string& address, U16 port );

   bool     AddInputChainData( BasePacket* packet, U32 socketId );
   void     InputConnected( IChainedInterface * chainedInput );
   void     InputRemovalInProgress( IChainedInterface * chainedInput );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );

private:
   void     HandleRerouteRequest( U32 connectionId );
   void     HandlePacketToKhaan( KhaanConnector* khaan, BasePacket* packet );
   U32      GetNextConnectionId();

   int      ProcessInputFunction();
   int      ProcessOutputFunction();
   void     OutputCurrentStats();
   void     SelectPreferredGateways();

   //void     ServerWasIdentified( ChainType* chainedInput );

   void     NewServerConnection( const PacketServerIdentifier* );
   void     ServerDisconnected( const PacketServerDisconnect* );
   void     ServerInfoUpdate( const PacketServerConnectionInfo* );

   list< GatewayInfo > m_gatewayRoutes;
   list< GatewayInfo >::iterator FindGateway( const string& ipAddress, U16 port, U32 serverId = 0 );

   typedef map< int, int >    SocketToConnectionMap;
   typedef pair<int, int>     SocketToConnectionPair;
   typedef SocketToConnectionMap::iterator SocketToConnectionMapIterator;

   typedef map< int, KhaanConnector* >    ConnectionMap;
   typedef pair< int, KhaanConnector* >   ConnectionPair;
   typedef ConnectionMap::iterator        ConnectionMapIterator;

   typedef std::deque< int >               ConnectionIdQueue;

   ConnectionMap              m_connectionMap;
   ConnectionIdQueue          m_connectionsNeedingUpdate;

   SocketToConnectionMap      m_socketToConnectionMap;
   SocketToConnectionMap      m_connectionToSocketMap;

   U32                        m_connectionIdTracker;
   time_t                     m_timestampStatsPrint;
   time_t                     m_timestampSelectPreferredGateway;
   static const U32           timeoutStatsPrint = 20;
   static const U32           timeoutSelectPreferredGateway = 60;
};

///////////////////////////////////////////////////////////////////
