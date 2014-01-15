#pragma once

// DiplodocusGateway.h

#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "KhaanConnector.h"
#include "GatewayCommon.h"
#include <deque>
class BasePacket;

class KhaanConnectorWrapper
{
public:
   KhaanConnectorWrapper( KhaanConnector* connector) : m_connector( connector ), m_markedForDeleteTime( 0 ){}
   KhaanConnector*      m_connector;
   time_t               m_markedForDeleteTime;
public:
   void     MarkForDeletion( time_t& time ) { m_markedForDeleteTime = time; }
   bool     IsMarkedForDeletion() const { return m_markedForDeleteTime != 0; }
   bool     HasDeleteTimeElapsed( time_t& currentTime ) const 
   {
      if( difftime( currentTime, m_markedForDeleteTime ) >= 5 ) // once per second
      {
         return true;
      }
      return false;
   }

private:
   KhaanConnectorWrapper(){}
};

//-----------------------------------------------------------------------------

class DiplodocusGateway : public Diplodocus< KhaanConnector >
{
public: 
   typedef Diplodocus< KhaanConnector > ChainedType;
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

   void           CleanupOldConnections();
   void           SendStatsToLoadBalancer();
   U32            GetNextConnectionId();

   time_t         m_timestampSendConnectionStatisics;
   static const U32 timeoutSendConnectionStatisics = 15;

   //--------------------------------------------------------

   typedef map< int, int >    SocketToConnectionMap;
   typedef pair<int, int>     SocketToConnectionPair;
   typedef SocketToConnectionMap::iterator SocketToConnectionMapIterator;

   typedef map< int, KhaanConnectorWrapper >    ConnectionMap;
   typedef pair< int, KhaanConnectorWrapper >   ConnectionPair;
   typedef ConnectionMap::iterator        ConnectionMapIterator;


   U32                        m_connectionIdTracker;   
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