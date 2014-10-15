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
class PacketServerToServer_GatewayRequestLB_ConnectionIds;

///////////////////////////////////////////////////////////////////

struct GatewayInfo
{
   enum Type
   {
      Type_None,
      Type_Normal,
      Type_Asset,
      Type_Other
   };

   U32      serverId;
   string   address;
   string   externalIpAddress;
   U16      port;
   Type     type;
   int      currentLoad;
   int      maxLoad;
   int      loadTolerance;
   bool     isConnected;
   bool     isVerified;
   bool     isPreferred;

   GatewayInfo( string addr, U16 p ) : serverId( 0 ),
                                       address( addr ), 
                                       port( p ), 
                                       type( Type_Normal ), 
                                       currentLoad( 0 ), 
                                       maxLoad( 450 ), 
                                       loadTolerance( 100 ), 
                                       isConnected( false ), 
                                       isVerified( false ),
                                       isPreferred( false )
                                       {}
};

///////////////////////////////////////////////////////////////////

class DiplodocusLoadBalancer :  public Diplodocus< KhaanConnector >
{
public:
   DiplodocusLoadBalancer( const string& serverName, U32 serverId );
   ~DiplodocusLoadBalancer();
   const char* GetClassName() const { return "DiplodocusLoadBalancer"; }

   void     PrintFunctionNames( bool printingOn = true );

   void     AddGatewayAddress( const string& address, U16 port );

   bool     AddInputChainData( BasePacket* packet, U32 socketId );
   void     InputConnected( IChainedInterface * chainedInput );
   void     InputRemovalInProgress( IChainedInterface * chainedInput );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );

private:
   void     HandleRerouteRequest( U32 connectionId );
   void     HandlePacketToKhaan( KhaanConnector* khaan, BasePacket* packet );
   U32      GetNextConnectionId();

   int      CallbackFunction();
   void     UpdateAllPendingRerouteRequests();
   void     OutputCurrentStats();
   void     SelectPreferredGateways();

   //void     ServerWasIdentified( ChainType* chainedInput );
   int      IncrementRotatingGatewayIndex();
   ChainLinkIteratorType FindInputByConnectionId( U32 connectionId );

   void     NewServerConnection( const PacketServerIdentifier* );
   void     ServerDisconnected( const PacketServerDisconnect* );
   void     ServerInfoUpdate( const PacketServerConnectionInfo* );
   void     RequestConnectionIds( const PacketServerToServer_GatewayRequestLB_ConnectionIds* );
   bool     PackageAndSendToOtherServer( BasePacket* packet, U32 serverId );

   list< GatewayInfo > m_gatewayRoutes;
   list< GatewayInfo >::iterator FindGateway( const string& ipAddress, U16 port, U32 serverId = 0 );
   list< GatewayInfo >::iterator FindGateway( U32 serverId );

   typedef map< int, int >    SocketToConnectionMap;
   typedef pair<int, int>     SocketToConnectionPair;
   typedef SocketToConnectionMap::iterator SocketToConnectionMapIterator;


   typedef std::deque< U32 >  ConnectionIdQueue;
   bool                       m_printFunctionNames;

   ConnectionIdQueue          m_pendingRequestsForReroute;

   SocketToConnectionMap      m_socketToConnectionMap;
   SocketToConnectionMap      m_connectionToSocketMap;

   U32                        m_distributedConnectionIdPoint;
   U16                        m_numConnectionIdsToDistrubute;

   U32                        m_connectionIdTracker;
   time_t                     m_timestampStatsPrint;
   time_t                     m_timestampSelectPreferredGateway;
   static const U32           timeoutStatsPrint = 20;
   static const U32           timeoutSelectPreferredGateway = 60;
};

///////////////////////////////////////////////////////////////////
