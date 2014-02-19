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
class PacketDbQueryResult;
class ChatChannelManager;

//////////////////////////////////////////////////////////////////////////////////

class DiplodocusChat : public Diplodocus< KhaanChat >, StatTrackingConnections
{
public: 
   typedef Diplodocus< KhaanChat > ChainedType;

public:
   DiplodocusChat( const string& serverName, U32 serverId );
   void     Init();

   void     ChatChannelManagerNeedsUpdate() { m_chatChannelManagerNeedsUpdate = true; }
   void     ServerWasIdentified( IChainedInterface* khaan );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     SendMessageToClient( BasePacket* packet, U32 connectionId );

   // from both the ChatUser and the ChatChannelManager
   bool     AddQueryToOutput( PacketDbQuery* packet, U32 connectionId, bool isChatChannelManager );

   //-------------------------------------
public:
// utility functions used by the ChatChannelManager
   ChatUser*    UpdateExistingUsersConnectionId( const string& uuid, U32 connectionId );
   ChatUser*    GetUser( U32 connectionId );
   ChatUser*    GetUserById( U32 userId );
   ChatUser*    GetUserByUuid( const string& userName );
   ChatUser*    GetUserByUsername( const string& userName );

   //-------------------------------------
private:
   ChatUser*    CreateNewUser( U32 connectionId );

   bool     HandleChatPacket( BasePacket* packet, U32 connectionId );
   bool     HandleLoginPacket( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromClient( BasePacket* packet );
   void     PeriodicWriteToDB();
   void     RemoveLoggedOutUsers();

   void     UpdateChatChannelManager();
   void     UpdateAllChatUsers();
   void     UpdateDbResults();
   int      CallbackFunction();

   typedef map< U32, ChatUser* >  UserMap;
   typedef UserMap::iterator      UserMapIterator;
   typedef pair< U32, ChatUser* > UserMapPair;

   map< U32, ChatUser* >         m_users;
   deque< PacketDbQueryResult* > m_dbQueries;

   static const int              logoutTimeout = 2 * 60; // two minutes 

   ChatChannelManager*           m_chatChannelManager;
   bool                          m_chatChannelManagerNeedsUpdate;
};

//////////////////////////////////////////////////////////////////////////////////