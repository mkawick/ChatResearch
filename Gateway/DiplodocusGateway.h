#pragma once

// DiplodocusGateway.h
#include <deque>
#include <list>
using namespace std;

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Stat/StatTrackingConnections.h"

#include "KhaanGateway.h"
#include "GatewayCommon.h"

class BasePacket;
class PacketStat;
class Fruitadens;
class FruitadensGateway;
typedef vector< FruitadensGateway* > OutputConnectorList;
typedef vector< OutputConnectorList > ListOfOutputLists;

class KhaanGatewayWrapper
{
public:
   KhaanGatewayWrapper( KhaanGateway* connector) : m_connector( connector ), m_markedForDeleteTime( 0 ){}
   KhaanGateway*      m_connector;
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
   KhaanGatewayWrapper(){}
};

////////////////////////////////////////////////////////////////////////////////////

class DiplodocusGateway : public Diplodocus< KhaanGateway >, public StatTrackingConnections
{
public: 
   typedef Diplodocus< KhaanGateway > ChainedType;

   //-----------------------------------------

public:
   DiplodocusGateway( const string& serverName, U32 serverId );
   ~DiplodocusGateway();
   void           Init();

   void           AllowUnauthenticatedConnections() { m_connectionsRequireAuthentication = false; }
   bool           AddInputChainData( BasePacket* packet, U32 socketId );
   bool           AddOutputChainData( BasePacket* packet, U32 serverType );
   void           NotifyFinishedAdding( IChainedInterface* obj );

   void           InputRemovalInProgress( IChainedInterface * chainedInput );

   void           PrintPacketTypes( bool printingOn = true );
   void           PrintFunctionNames( bool printingOn = true );

   void           SetupReroute( const string& address, U16 port );
   bool           IsRerouoting() { return ( m_reroutePort != 0 ) && ( m_rerouteAddress.size() > 0 ); }

   void           TrackCountStats( StatTracking stat, float value, int sub_category );

   //-----------------------------------------------------
private:
   
   void           InputConnected( IChainedInterface * );
   bool           PushPacketToProperOutput( BasePacket* packet );
   void           SortOutgoingPackets();

   void           HandlePacketToKhaan( KhaanGateway* khaan, BasePacket* packet );
   
   void           MoveClientBoundPacketsFromTempToKhaan();

   void           HandleReroutRequest( U32 connectionId );
   void           UpdateAllClientConnections();
   void           AddClientConnectionNeedingUpdate( U32 id );

   //int            MainLoop_InputProcessing();
   //int            MainLoop_OutputProcessing();
   int            CallbackFunction();

   void           CleanupOldConnections();
   void           SendStatsToLoadBalancer();
   void           RunHourlyAverages();
   U32            GetNextConnectionId();
   bool           OrderOutputs();

   time_t         m_timestampSendConnectionStatisics;
   static const U32 timeoutSendConnectionStatisics = 61*2; // 2 minutes
   time_t         m_timestampSendStatServerStatisics;
   static const U32 timeoutSendStatServerStatisics = 10*60;// ten minutes

   //--------------------------------------------------------

   typedef map< int, int >    SocketToConnectionMap;
   typedef pair<int, int>     SocketToConnectionPair;
   typedef SocketToConnectionMap::iterator SocketToConnectionMapIterator;

   typedef map< int, KhaanGatewayWrapper >    ConnectionMap;
   typedef pair< int, KhaanGatewayWrapper >   ConnectionPair;
   typedef ConnectionMap::iterator        ConnectionMapIterator;

   ListOfOutputLists          m_orderedOutputPacketHandlers;


   U32                        m_connectionIdTracker;   
   std::deque< BasePacket* >  m_clientBoundTempStorage;

   SocketToConnectionMap      m_socketToConnectionMap;
   SocketToConnectionMap      m_connectionToSocketMap;

   ConnectionMap              m_connectionMap;
   ConnectionIdQueue          m_connectionsNeedingUpdate;

   PacketQueue                m_packetsToBeSentInternally;

   bool                       m_printPacketTypes;
   bool                       m_printFunctionNames;
   bool                       m_connectionsRequireAuthentication;

   string                     m_rerouteAddress;
   U16                        m_reroutePort;
};

//-----------------------------------------------------------------------------