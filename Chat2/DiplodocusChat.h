#pragma once

#include <map>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/NetworkIn/Khaan.h"
#include "../NetworkCommon/Database/Deltadromeus.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Stat/StatTrackingConnections.h"
#include "KhaanChat.h"


class ChatUser;
class PacketDbQuery;

//////////////////////////////////////////////////////////////////////////////////

class DiplodocusChat : public Diplodocus< KhaanChat >, StatTrackingConnections
{
public: 
   typedef Diplodocus< KhaanChat > ChainedType;

public:
   DiplodocusChat( const string& serverName, U32 serverId );
   void     Init();

   void     ServerWasIdentified( IChainedInterface* khaan );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddQueryToOutput( PacketDbQuery* packet );

private:
   ChatUser* CreateNewUser( U32 connectionId );
   ChatUser* UpdateExistingUsersConnectionId( const string& uuid, U32 connectionId );
   ChatUser* GetUser( U32 connectionId );
   ChatUser* GetUserById( U32 userId );
   ChatUser* GetUserByUuid( const string& userName );
   ChatUser* GetUserByUsername( const string& userName );

   bool     HandleChatPacket( BasePacket* packet, U32 connectionId );
   bool     HandleLoginPacket( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   void     PeriodicWriteToDB();
   int      CallbackFunction();

   typedef map< U32, ChatUser* >  UserMap;
   typedef UserMap::iterator      UserMapIterator;
   typedef pair< U32, ChatUser* > UserMapPair;

   map< U32, ChatUser* >     m_users;

};

//////////////////////////////////////////////////////////////////////////////////