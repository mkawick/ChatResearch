#pragma once

// DiplodocusGateway.h

#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "KhaanConnector.h"
#include "GatewayCommon.h"

//-----------------------------------------------------------------------------

class DiplodocusGateway : public Diplodocus< KhaanConnector >
{
public:
   DiplodocusGateway( const string& serverName, U32 serverId );
   ~DiplodocusGateway();

   bool           AddInputChainData( BasePacket* packet, U32 socketId );
   bool           AddOutputChainData( BasePacket* packet, U32 serverType );

   void           ClientConnectionIsAboutToRemove( KhaanConnector* khaan );

   //-----------------------------------------

private:

   void           ClientConnectionFinishedAdding( KhaanConnector* khaan );
   bool           PushPacketToProperOutput( BasePacket* packet );

   void           HandlePacketToKhaan( KhaanConnector* khaan, BasePacket* packet );

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

   SocketToConnectionMap      m_socketToConnectionMap;
   SocketToConnectionMap      m_connectionToSocketMap;

   ConnectionMap              m_connectionMap;
   ConnectionIdQueue          m_connectionsNeedingUpdate;

   PacketQueue                m_packetsToBeSentInternally;
};

//-----------------------------------------------------------------------------