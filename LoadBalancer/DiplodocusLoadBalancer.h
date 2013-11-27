#pragma once
// DiplodocusLoadBalancer.h

#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "KhaanConnector.h"
#include <deque>
#include <string>
using namespace std;
class BasePacket;

///////////////////////////////////////////////////////////////////

struct GatewayInfo
{
   string address;
   U16 port;

   GatewayInfo( string addr, U16 p ): address( addr ), port( p ) {}
};

///////////////////////////////////////////////////////////////////

class DiplodocusLoadBalancer :  public Diplodocus< KhaanConnector >
{
public:
   DiplodocusLoadBalancer( const string& serverName, U32 serverId );
   ~DiplodocusLoadBalancer();

   void  AddGatewayAddress( const string& address, U16 port );

   bool  AddInputChainData( BasePacket* packet, U32 socketId );
   void  InputConnected( IChainedInterface * chainedInput );
   void  InputRemovalInProgress( IChainedInterface * chainedInput );
   bool  HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );

private:
   void  HandleRerouteRequest( U32 connectionId );
   void  HandlePacketToKhaan( KhaanConnector* khaan, BasePacket* packet );
   U32   GetNextConnectionId();

   list< GatewayInfo > m_gatewayRoutes;

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
};

///////////////////////////////////////////////////////////////////
