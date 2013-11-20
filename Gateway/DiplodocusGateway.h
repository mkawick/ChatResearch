#pragma once

// DiplodocusGateway.h

#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "KhaanConnector.h"
#include "GatewayCommon.h"
#include <deque>
class BasePacket;

//-----------------------------------------------------------------------------

class DiplodocusGateway : public Diplodocus< KhaanConnector >
{
public:
   DiplodocusGateway( const string& serverName, U32 serverId );
   ~DiplodocusGateway();

   bool           AddInputChainData( BasePacket* packet, U32 socketId );
   bool           AddOutputChainData( BasePacket* packet, U32 serverType );

   void           InputRemovalInProgress( IChainedInterface * chainedInput );

   void           PrintPacketTypes( bool printingOn = true );

   void           SetupReroute( const string& address, U16 port );
   bool           IsRerouoting() { return ( m_reroutePort != 0 ) && ( m_rerouteAddress.size() > 0 ); }

   //-----------------------------------------

private:
   
   void           InputConnected( IChainedInterface * );
   bool           PushPacketToProperOutput( BasePacket* packet );

   void           HandlePacketToKhaan( KhaanConnector* khaan, BasePacket* packet );

   void           HandleReroutRequest( U32 connectionId );

   int            ProcessInputFunction();
   int            ProcessOutputFunction();
   U32            GetNextConnectionId();

   U32                        m_connectionIdTracker;
   typedef map< int, int >    SocketToConnectionMap;
   typedef pair<int, int>     SocketToConnectionPair;
   typedef SocketToConnectionMap::iterator SocketToConnectionMapIterator;

   typedef map< int, KhaanConnector* >    ConnectionMap;
   typedef pair< int, KhaanConnector* >   ConnectionPair;
   typedef ConnectionMap::iterator        ConnectionMapIterator;
   std::deque< BasePacket* >  m_outputTempStorage;

   SocketToConnectionMap      m_socketToConnectionMap;
   SocketToConnectionMap      m_connectionToSocketMap;

   ConnectionMap              m_connectionMap;
   ConnectionIdQueue          m_connectionsNeedingUpdate;

   PacketQueue                m_packetsToBeSentInternally;

   bool                       m_printPacketTypes;

   string                     m_rerouteAddress;
   U16                        m_reroutePort;
};

//-----------------------------------------------------------------------------