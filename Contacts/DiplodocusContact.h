#pragma once
#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "../NetworkCommon/Database/QueryHandler.h"
#include "../NetworkCommon/Stat/StatTrackingConnections.h"
#include "KhaanContact.h"
#include "UserContact.h"

#include <map>
#include <deque>
using namespace std;
class PacketPrepareForUserLogin;
class PacketPrepareForUserLogout;

///////////////////////////////////////////////////////////////////

class DiplodocusContact :  public Diplodocus< KhaanContact >, StatTrackingConnections
{
public:
   typedef Diplodocus< KhaanContact > Parent;
   enum { SecondsBeforeRemovingLoggedOutUser = 10
   };

public:
   DiplodocusContact( const string& serverName, U32 serverId );
   void     ServerWasIdentified( IChainedInterface* khaan );
   bool     AddInputChainData( BasePacket* packet, U32 connectionId );

   UserContact* GetUser( U32 userId );
   UserContact* GetUserByUuid( const string& uuid );
   UserContact* GetUserByUsername( const string& username );

   // tables:
   // friends
   // user_profile
   // friend_pending

   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool     AddQueryToOutput( PacketDbQuery* query );
   //bool     SendErrorToClient( U32 connectionId, PacketErrorReport::ErrorType error );

   void     TrackNumSearches() { m_numSearches++; }
   void     TrackInviteSent() { m_numInvitesSent++; }
   void     TrackInviteAccepted() { m_numInvitesAccepted++; }
   void     TrackInviteRejected() { m_numInvitesRejected++; }
   void     ClearStats();


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

   void     TrackCountStats( StatTracking stat, float value, int sub_category );
   void     RunHourlyStats();
   void     RunDailyStats();

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

   time_t                           m_timestampHourlyStatServerStatisics;
   static const U32                 timeoutHourlyStatisics = 60*60;
   time_t                           m_timestampDailyStatServerStatisics;
   static const U32                 timeoutDailyStatisics = timeoutHourlyStatisics*24;
};

///////////////////////////////////////////////////////////////////