#pragma once

// MainGatewayThread.h
#include <deque>
#include <list>
using namespace std;

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Stat/StatTrackingConnections.h"

#include "KhaanGateway.h"
#include "GatewayCommon.h"


class BasePacket;
class PacketStat;
class PacketServerConnectionInfo_ServerOutageSchedule;

class Fruitadens;
class FruitadensGateway;
class ServiceAvailabilityManager;

////////////////////////////////////////////////////////////////////////////////////

typedef vector< FruitadensGateway* >  OutputConnectorList;
typedef vector< OutputConnectorList > ListOfOutputLists;


////////////////////////////////////////////////////////////////////////////////////

class MainGatewayThread : public Diplodocus< KhaanGateway >, public StatTrackingConnections
{
public:
   MainGatewayThread( const string& serverName, U32 serverId );
   ~MainGatewayThread();
   const char*    GetClassName() const { return "MainGatewayThread"; }
   void           Init();

   void           AllowUnauthenticatedConnections() { m_connectionsRequireAuthentication = false; }
   bool           AddInputChainData( BasePacket* packet, U32 socketId );
   bool           AddOutputChainData( BasePacket* packet, U32 serverType );
   void           NotifyFinishedAdding( IChainedInterface* obj );

   void           InputRemovalInProgress( IChainedInterface * chainedInput );
   void           FinalRemoveInputChain( U32 connectionId );

   void           PrintPacketTypes( bool printingOn = true );
   void           PrintFunctionNames( bool printingOn = true );

   void           SetupReroute( const string& address, U16 port );
   bool           IsRerouoting() { return ( m_reroutePort != 0 ) && ( m_rerouteAddress.size() > 0 ); }

   void           TrackCountStats( StatTracking stat, float value, int sub_category );
   bool           IsGatewayReady() const;

   //-----------------------------------------------------

   void           OutputConnected( IChainedInterface * );
   void           OutputRemovalInProgress( IChainedInterface * chainedOutput );

   void           GetListOfOutputs( list< FruitadensGateway* >& tempOutputContainer );
   void           BroadcastPacketToAllUsers( const string& errorText, int errorState, int param1, int param2, U8 matchingGameId, bool sendImmediate = false );
   void           CreateFilteredListOfClientConnections( U32 GameId, vector< U32 >& connectionIds );
   void           CreateListOfClientConnectionsForGame( vector< ClientConnectionForGame >& ccfg );
   bool           SendPacketToServer( BasePacket* packet, ServerType type );
   void           SendAllServerStateChangesToClients( const vector< QOS_ServiceChange >& listOfchanges );
   void           CloseConnection( U32 connectionId );
   void           InformUserOfMissingFeatures( const vector< ServerStatus >& servers, U32 connectionId, U8 gameId );
   void           LogUserOutIfKeyFeaturesAreUnavailable( ServerType type, U8 gameId, U32 connectionId );
   
   //-----------------------------------------------------
private:
   
   void           InputConnected( IChainedInterface * );
   bool           PreprocessServerBoundPackets( BasePacket* packet, U32 connectionId );
   bool           PushPacketToProperOutput( BasePacket* packet );
   void           SortOutgoingPackets();

   void           HandlePacketToKhaan( KhaanGateway* khaan, BasePacket* packet );
   BasePacket*    HandlePlayerLoginStatus(  KhaanGateway* khaan, BasePacket* packet );
   
   void           MoveClientBoundPacketsFromTempToKhaan();
   void           MarkConnectionForDeletion( U32 connectionId );

   void           HandleReroutRequest( U32 connectionId );

   void           UpdatedScheduledOutages();

   //void           CheckOnServerStatusChanges();
   //void           GetConnectedServerList( vector< ServerStatus >& serversNotConnected, bool onlyDisconnected );
   
   

   void           CheckOnConnectionIdBlocks();
   bool           RequestMoreConnectionIdsFromLoadBalancer();
   void           InformLoginServerAboutLostconnectedClients();

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
   time_t         m_timestampRequestConnectionIdBlocks;
   static const U32 timeoutCheckOnConnectionIdBlocks = 10*60;// five minutes

   //--------------------------------------------------------

   typedef map< int, KhaanGateway* >         ConnectionMap;
   typedef pair< int, KhaanGateway* >        ConnectionPair;
   typedef ConnectionMap::iterator           ConnectionMapIterator;

   list< ConnectionIdStorage >             m_usableConnectionIds;


   ListOfOutputLists          m_orderedOutputPacketHandlers;

   U32                        m_highestNumSimultaneousUsersWatermark;
   U32                        m_connectionIdTracker;
   U32                        m_connectionIdBeginningRange;
   U16                        m_connectionIdCountIds;
   std::deque< BasePacket* >  m_clientBoundTempStorage;
   std::deque< PacketServerConnectionInfo_ServerOutageSchedule* > m_scheduledOutages;

   ConnectionMap              m_connectionMap;

   PacketQueue                m_packetsToBeSentInternally;
   ServiceAvailabilityManager*m_serviceAvailabilityManager;

   bool                       m_printPacketTypes;
   bool                       m_printFunctionNames;
   bool                       m_connectionsRequireAuthentication;

   bool                       m_isServerDownForMaintenence;
   bool                       m_hasInformedConnectedClientsThatServerIsDownForMaintenence;
   bool                       m_serverIsAvaitingLB_Approval;
   time_t                     m_scheduledMaintnenceBegins;
   time_t                     m_scheduledMaintnenceEnd;

   string                     m_rerouteAddress;
   U16                        m_reroutePort;
};

////////////////////////////////////////////////////////////////////////////////////