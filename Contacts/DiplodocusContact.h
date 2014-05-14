#pragma once
#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "../NetworkCommon/Database/QueryHandler.h"
#include "../NetworkCommon/Stat/StatTrackingConnections.h"
#include "../NetworkCommon/Invitations/InvitationManager.h"
#include "KhaanContact.h"
#include "UserContact.h"

#include <map>
#include <deque>
using namespace std;
class PacketPrepareForUserLogin;
class PacketPrepareForUserLogout;
class PacketUserUpdateProfile;
class UserLookupManager;

///////////////////////////////////////////////////////////////////

class DiplodocusContact :  public Diplodocus< KhaanContact >, public StatTrackingConnections, public PacketSendingInterface
{
public:
   typedef Diplodocus< KhaanContact > Parent;
   enum { SecondsBeforeRemovingLoggedOutUser = 10
   };

public:
   DiplodocusContact( const string& serverName, U32 serverId );
   void     Init();

   void     ServerWasIdentified( IChainedInterface* khaan );
   bool     AddInputChainData( BasePacket* packet, U32 connectionId );

   UserContact* GetUser( U32 userId );
   UserContact* GetUserByUuid( const string& uuid );
   UserContact* GetUserByUsername( const string& username );

   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     AddQueryToOutput( PacketDbQuery* query );

   void     TrackNumSearches() { m_numSearches++; }
   void     TrackInviteSent() { m_numInvitesSent++; }
   void     TrackInviteAccepted() { m_numInvitesAccepted++; }
   void     TrackInviteRejected() { m_numInvitesRejected++; }
   void     ClearStats();

   U32      GetInvitationExpryTime() const { return m_secondsBetweenSendingInvitationAndExpiration; }

   void     InvitationManagerNeedsUpdate() { m_invitationManagerNeedsUpdate = true; }
   void     UserLookupNeedsUpdate() { m_userLookupNeedsUpdate = true; }

   bool     SendMessageToClient( BasePacket* packet, U32 connectionId );
   bool     AddQueryToOutput( PacketDbQuery* packet, U32 connectionId );
   bool     SendErrorToClient( U32 connectionId, PacketErrorReport::ErrorType error );
   string   GetUserUuidByConnectionId( U32 connectionId );
   void     GetUserConnectionId( const string& uuid, U32& connectionId );
   string   GetUserName( const string& uuid );

private:

   int      CallbackFunction();
   void     UpdateAllConnections();
   void     UpdateDbResults();

   
   bool     HandleCommandFromGateway( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandlePacketRequests( PacketContact* packet, U32 connectionId );
   //bool     HandleDbQueryResult( PacketDbQueryResult* result );

   bool     ConnectUser( PacketPrepareForUserLogin* login );
   bool     DisconnectUser( PacketPrepareForUserLogout* login );

   bool     UpdateUserProfile( const PacketUserUpdateProfile* profile );

   void     TrackCountStats( StatTracking stat, float value, int sub_category );
   void     RunHourlyStats();
   void     RunDailyStats();
   void     ExpireOldInvitations();

   void     RequestAdminSettings();
   void     HandleAdminSettings( const PacketDbQueryResult* dbResult );
   void     HandleExipiredInvitations( const PacketDbQueryResult* dbResult );

   //bool     DiplodocusContact::InviteUser( const string& inviterUuid, const string& inviteeUuid, const string& message );

   //bool     SendInitializationQuery( QueryType );

   //--------------------------------------

   typedef map< U32, UserContact >     UserContactMap;
   typedef pair< U32, UserContact >    UserContactPair;
   typedef UserContactMap::iterator    UserContactMapIterator;

   typedef map< U32, U32 >             UserIdToContactMap;
   typedef pair< U32, U32 >            UserIdToContactPair;
   typedef UserIdToContactMap::iterator  UserIdToContactMapIterator;

   deque< U32 >                     m_serversNeedingUpdate;
   deque< PacketDbQueryResult* >    m_dbQueries;
   UserContactMap                   m_users;
   UserIdToContactMap               m_userLookupById;

   int                              m_numSearches;
   int                              m_numInvitesSent;
   int                              m_numInvitesAccepted;
   int                              m_numInvitesRejected;

   bool                             m_hasRequestedAdminSettings;
   bool                             m_isWaitingForAdminSettings;

   time_t                           m_timestampHourlyStatServerStatisics;
   static const U32                 timeoutHourlyStatisics = 60*60;
   time_t                           m_timestampDailyStatServerStatisics;
   static const U32                 timeoutDailyStatisics = timeoutHourlyStatisics*24;

   time_t                           m_timestampExpireOldInvitations;
   U32                              m_secondsBetweenSendingInvitationAndExpiration;

   UserLookupManager*               m_userLookup;
   bool                             m_invitationManagerNeedsUpdate;

   InvitationManager*               m_invitationManager;
   bool                             m_userLookupNeedsUpdate;

   enum QueryType 
   {
      QueryType_AccountAdminSettings = 1,
      QueryType_SelectExpiredInvitations,
      QueryType_DeleteExpiredInvitations
   };
};

///////////////////////////////////////////////////////////////////