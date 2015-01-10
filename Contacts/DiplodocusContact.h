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
class PacketLoginExpireUser;

///////////////////////////////////////////////////////////////////

class DiplodocusContact :  public Diplodocus< KhaanContact >, public StatTrackingConnections, public PacketSendingInterface
{
public:
   typedef Diplodocus< KhaanContact > Parent;
   enum { SecondsBeforeRemovingLoggedOutUser = 10
   };

public:
   DiplodocusContact( const string& serverName, U32 serverId );
   const char* GetClassName() const { return "DiplodocusContact"; }
   void     Init();

   void     ServerWasIdentified( IChainedInterface* khaan );
   void     InputRemovalInProgress( IChainedInterface * );
   bool     AddInputChainData( BasePacket* packet, U32 connectionId );

   

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

   bool     SendMessageToClient( BasePacket* packet, U32 connectionId, U32 gatewayId );
   bool     AddQueryToOutput( PacketDbQuery* packet, U32 connectionId );
   bool     SendErrorToClient( U32 connectionId, U32 gatewayId, PacketErrorReport::ErrorType error );
private:

   int      CallbackFunction();
   void     UpdateAllConnections();
   void     UpdateDbResults();
   bool     ProcessPacket( PacketStorage& storage );

   
   bool     HandleCommandFromGateway( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandlePacketRequests( PacketContact* packet, U32 connectionId, U32 gatewayId );
   //bool     HandleDbQueryResult( PacketDbQueryResult* result );

   bool     ConnectUser( const PacketPrepareForUserLogin* login );
   bool     DisconnectUser( const PacketPrepareForUserLogout* login );
   bool     ExpireUser( const PacketLoginExpireUser* actualPacket );
   bool     DeleteAllUsers();
   bool     UpdateUser( const string& userUuid, U32 connectionId, U32 gatewayId );

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

  /* typedef map< U32, UserContact >     UserContactMap;
   typedef pair< U32, UserContact >    UserContactPair;
   typedef UserContactMap::iterator    UserContactMapIterator;

   typedef map< U32, U32 >             UserIdToContactMap;
   typedef pair< U32, U32 >            UserIdToContactPair;
   typedef UserIdToContactMap::iterator  UserIdToContactMapIterator;

   UserContactMap                   m_users;
   UserIdToContactMap               m_userLookupById;*/

   typedef map< stringhash, UserContact >       UserContactMap;
   typedef pair< stringhash, UserContact >      UserContactPair;
   typedef UserContactMap::iterator             UserContactMapIterator;
   typedef UserContactMap::const_iterator       ConstUserContactMapIterator;

public:
   string                  GetUserUuidByConnectionId( U32 connectionId );
   void                    GetUserConnectionId( const string& uuid, vector< SimpleConnectionDetails >& listOfConnections );
   string                  GetUserName( const string& uuid );
   bool                    GetUser( const string& uuid, UserContact*& user );
   const string            GetUuid( U32 connectionId ) const;
   bool                    GetUser( U32 userId, UserContact*& user );
   bool                    GetUserByUsername( const string& name, UserContact*& user );
   UserContactMapIterator  GetUserByConnectionId( U32 connectionId );
   
private:

   UserContactMap                   m_userTickets;

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
   bool                             m_userLookupNeedsUpdate;

   InvitationManager*               m_invitationManager;
   bool                             m_invitationManagerNeedsUpdate;

   enum QueryType 
   {
      QueryType_AccountAdminSettings = 1,
      QueryType_SelectExpiredInvitations,
      QueryType_DeleteExpiredInvitations
   };
};

///////////////////////////////////////////////////////////////////