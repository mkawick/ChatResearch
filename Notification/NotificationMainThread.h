#pragma once

#include <map>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"


///////////////////////////////////////////////////////////////////

class NotificationMainThread : public Queryer, public Diplodocus< KhaanServerToServer >
{
public: 
   typedef Diplodocus< KhaanServerToServer > ChainedType;

public:
   NotificationMainThread( const string& serverName, U32 serverId );
   ~NotificationMainThread();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     SendMessageToClient( BasePacket* packet, U32 connectionId );

   bool     AddQueryToOutput( PacketDbQuery* packet );
   void     ServerWasIdentified( IChainedInterface* khaan );

private:

   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   void     PeriodicCheckForNewNotifications();

   int      MainLoop_InputProcessing();
   int      MainLoop_OutputProcessing();

   time_t                  m_lastNotificationCheck_TimeStamp;
   static const int        timeoutDBWriteStatisics = 60 * 60;// one hour
};

///////////////////////////////////////////////////////////////////