#pragma once

#include <deque>
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

#include "UserConnection.h"

namespace Database
{
   class Deltadromeus;
}

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
   bool     HandlePacketFromGateway( BasePacket* packet, U32 connectionId );
   bool     ConnectUser( const PacketPrepareForUserLogin* loginPacket );
   bool     DisconnectUser( const PacketPrepareForUserLogout* unwrappedPacket );
   bool     HandleNotification( const PacketNotification_SendNotification* unwrappedPacket );
   
   void     PeriodicCheckForNewNotifications();

   void     RemoveExpiredConnections();

   int      MainLoop_InputProcessing();
   int      MainLoop_OutputProcessing();


   typedef  map< U32, UserConnection >    UserConnectionMap;
   typedef  pair< U32, UserConnection >   UserConnectionPair;
   typedef  UserConnectionMap::iterator   UserConnectionIterator;

   typedef  deque< PacketDbQueryResult* > QueryResultDeque;
   typedef  QueryResultDeque::iterator    QueryResultDequeIterator;

   UserConnectionMap       m_userConnectionMap;
   QueryResultDeque        m_dbQueries;

   Database::Deltadromeus* m_database; // only one for now

   time_t                  m_lastNotificationCheck_TimeStamp;
   static const int        timeoutNotificationSend = 60 * 60;// one hour
   static const int        SecondsBeforeRemovingLoggedOutUser = 3;

   
};

///////////////////////////////////////////////////////////////////