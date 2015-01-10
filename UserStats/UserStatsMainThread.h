// UserStatsMainThread.h
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
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

class PacketDbQuery;
class PacketDbQueryResult;
class PacketPrepareForUserLogin;
class PacketPrepareForUserLogout;
class PacketLoginExpireUser;

///////////////////////////////////////////////////////////////////

struct ConnectionPair
{
   ConnectionPair( U32 connId, U32 gateId ) : connectionId( connId ), gatewayId( gateId ){}
   U32 connectionId, gatewayId;
};

class UserStatsMainThread : public Diplodocus< KhaanServerToServer >
{
public:
   UserStatsMainThread( const string& serverName, U32 serverId );
   ~UserStatsMainThread();

   void     Init();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     SendMessageToClient( BasePacket* packet, U32 connectionId );
   bool     SendGameData( U32 connectionId, U32 gatewayId, int packetSize, const U8* packet );
   bool     AddQueryToOutput( PacketDbQuery* dbQuery );

   void     ServerWasIdentified( IChainedInterface* khaan );
   void     InputRemovalInProgress( IChainedInterface * chainedInput );

   DbHandle*   GetDbConnectionByType( Database::Deltadromeus::DbConnectionType type );

private:
   int      CallbackFunction();
   void     UpdateDbResults();
   bool     ProcessPacket( PacketStorage& storage );

   bool     ConnectUser( const PacketPrepareForUserLogin* login );
   bool     DisconnectUser( const PacketPrepareForUserLogout* login );
   bool     ExpireUser( const PacketLoginExpireUser* actualPacket );
   bool     DeleteAllUsers();

   bool     HandlePacketFromClient( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );// not thread safe
   bool     HandleLoginPacket( BasePacket* packet, U32 connectionId );
   bool     HandleUserStatPacket( BasePacket* packet, U32 connectionId );

   deque< ConnectionPair > m_userConnectionList;
};

///////////////////////////////////////////////////////////////////