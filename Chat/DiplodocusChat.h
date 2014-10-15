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
#include "../NetworkCommon/Invitations/InvitationManager.h"

#include "KhaanChat.h"

class ChatUser;
class PacketDbQuery;
class PacketDbQueryResult;
class ChatRoomManager;

//////////////////////////////////////////////////////////////////////////////////

class DiplodocusChat : public Diplodocus< KhaanChat >, public StatTrackingConnections, public PacketSendingInterface
{
public: 
   typedef Diplodocus< KhaanChat > ChainedType;

public:
   DiplodocusChat( const string& serverName, U32 serverId );
   const char* GetClassName() const { return "DiplodocusChat"; }
   void     Init();

   void     ChatChannelManagerNeedsUpdate() { m_chatRoomManagerNeedsUpdate = true; }
   void     InvitationManagerNeedsUpdate() { m_invitationManagerNeedsUpdate = true; }
   void     ServerWasIdentified( IChainedInterface* khaan );
   void     InputRemovalInProgress( IChainedInterface * );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   

   // from both the ChatUser and the ChatChannelManager
   // PacketSendingInterface   
   bool     SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId );
   bool     AddQueryToOutput( PacketDbQuery* packet, U32 connectionId );
   bool     SendErrorToClient( U32 connectionId, U32 gatewayId, PacketErrorReport::ErrorType error );

   //-------------------------------------
public:
// utility functions used by the ChatChannelManager
   ChatUser*   UpdateExistingUsersConnectionId( const string& uuid, U32 connectionId, U32 gatewayId );
   ChatUser*   GetUser( U32 connectionId );
   ChatUser*   GetUserById( U32 userId );
   ChatUser*   GetUserByUuid( const string& userName );
   ChatUser*   GetUserByUsername( const string& userName );
   ChatUser*   GetUserByConnectionId( U32 ConnectionId );

   string      GetUserUuidByConnectionId( U32 connectionId );
   void        GetUserConnectionId( const string& uuid, U32& connectionId, U32& gatewayId );
   string      GetUserName( const string& uuid );

   //-------------------------------------
private:
   ChatUser*    CreateNewUser( U32 connectionId, U32 gatewayId );

   bool     HandleChatPacket( BasePacket* packet, U32 gatewayId );
   bool     HandleInvitationPacket( BasePacket* packet, U32 connectionId, U32 gatewayId );
   bool     HandleLoginPacket( BasePacket* packet, U32 gatewayId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 gatewayId );
   bool     HandlePacketFromClient( BasePacket* packet );
   void     PeriodicWriteToDB();
   void     RemoveLoggedOutUsers();

   void     UpdateChatChannelManager();
   void     UpdateInvitationManager();
   void     UpdateAllChatUsers();
   void     UpdateDbResults();
   void     TrackCountStats( StatTracking stat, float value, int sub_category );
   void     RunHourlyStats();
   void     RunDailyStats();
   bool     ProcessPacket( PacketStorage& storage );
   int      CallbackFunction();

   typedef map< U32, ChatUser* >  UserMap;
   typedef UserMap::iterator      UserMapIterator;
   typedef UserMap::const_iterator UserMapConstIterator;
   typedef pair< U32, ChatUser* > UserMapPair;

   map< U32, ChatUser* >         m_users;

   static const int              logoutTimeout = 2 * 60; // two minutes 

   ChatRoomManager*              m_chatRoomManager;
   bool                          m_chatRoomManagerNeedsUpdate;

   InvitationManager*            m_invitationManager;
   bool                          m_invitationManagerNeedsUpdate;

   time_t                        m_timestampHourlyStatServerStatisics;
   static const U32              timeoutHourlyStatisics = 60*60;
   time_t                        m_timestampDailyStatServerStatisics;
   static const U32              timeoutDailyStatisics = timeoutHourlyStatisics*24;
};

//////////////////////////////////////////////////////////////////////////////////