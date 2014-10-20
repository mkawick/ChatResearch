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
class Fruitadens;
class FruitadensGateway;

////////////////////////////////////////////////////////////////////////////////////

typedef vector< FruitadensGateway* >  OutputConnectorList;
typedef vector< OutputConnectorList > ListOfOutputLists;

////////////////////////////////////////////////////////////////////////////////////

struct ConnectionIdStorage
{
   ConnectionIdStorage( U32 _id, U16 _count ) : id( _id ), countIds( _count ){}
   U32 id;
   U16 countIds;
};


////////////////////////////////////////////////////////

struct QOS_ServiceChange
{
   U8       serverType;
   U8       gameId;
   U8       errorTypeMessageToSend;
   bool     forceUsersToDc;
   bool     isConnected;
   
   char*    text;
};

////////////////////////////////////////////////////////////////////////////////////

class MainGatewayThread : public Diplodocus< KhaanGateway >, public StatTrackingConnections
{
public: 
   typedef Diplodocus< KhaanGateway > ChainedType;

   //-----------------------------------------

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

   //-----------------------------------------------------
private:
   
   void           InputConnected( IChainedInterface * );
   bool           PushPacketToProperOutput( BasePacket* packet );
   void           SortOutgoingPackets();

   void           HandlePacketToKhaan( KhaanGateway* khaan, BasePacket* packet );
   BasePacket*    HandlePlayerLoginStatus(  KhaanGateway* khaan, BasePacket* packet );
   void           BroadcastPacketToAllUsers( const string& errorText, int errorState, int param1, int param2, U8 matchingGameId );
   
   void           MoveClientBoundPacketsFromTempToKhaan();
   void           MarkConnectionForDeletion( U32 connectionId );

   void           HandleReroutRequest( U32 connectionId );
   void           UpdateAllClientConnections();

   void           CheckOnServerStatusChanges();
   bool           SendPacketToServer( BasePacket* packet, ServerType type );
   void           CreateFilteredListOfClientConnections( U32 GameId, vector< U32 >& connectionIds );
   void           SendAllServerStateChangesToClients( const vector< QOS_ServiceChange >& listOfchanges );

   void           CheckOnConnectionIdBlocks();
   bool           RequestMoreConnectionIdsFromLoadBalancer();

   //void           RequestNewConenctionIdsFromLoadBalancer();
   //void           UpdateRemovedConnections();

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

   ConnectionMap              m_connectionMap;

   PacketQueue                m_packetsToBeSentInternally;

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