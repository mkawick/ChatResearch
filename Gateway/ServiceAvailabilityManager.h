// ServiceAvailabilityManager.h

#pragma once

#include <vector>
using namespace std;

#include "../NetworkCommon/ServerType.h"
#include "../NetworkCommon/DataTypes.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"

#include "GatewayCommon.h"
#include "MainGatewayThread.h"
//class MainGatewayThread;

////////////////////////////////////////////////////////////////////////////////////

class ServiceAvailabilityManager
{
public:
   ServiceAvailabilityManager();
   ~ServiceAvailabilityManager();

   void  Update();


   void  ServiceIsScheduledToGoDown();

   void  InformUserAboutAvailableFeatures( U8 gameId, U32 connectionId );
   void  InformUserAboutScheduledOutages( U8 gameId, U32 connectionId );

   void  SetServiceManager( MainGatewayThread* mgwt ) { m_mainGatewayThread = mgwt; }

   void  ScheduledOutages( const PacketServerConnectionInfo_ServerOutageSchedule* outagePacket );

private:

   void     CheckOnServerStatusChanges();
   void     SendAllServerStateChangesToClients( const vector< QOS_ServiceChange >& listOfchanges );
   void     GetConnectedServerList( vector< ServerStatus >& serversNotConnected, bool onlyDisconnected );
   
   bool     CopyScheduledOutagesToLocalOutages( const ScheduledOutage& newScheduledOutage );
   void     EnableAndDisableServicesBasedOnSchecules();
   void     RemoveCancelledAndExpiredScheduledOutages();
   void     InformAllConnectedUsersOfScheduledOutages();

   // part of update
   void     DisableServicesInScheduledMaintenenceMode();
   void     ReenableServicesWithExpiredScheduledMaintenence();

   vector< ServerStatus >     m_serviceStatus;
   MainGatewayThread*         m_mainGatewayThread;

   vector< ScheduledOutage >  m_scheduledOutages;
};

////////////////////////////////////////////////////////////////////////////////////
