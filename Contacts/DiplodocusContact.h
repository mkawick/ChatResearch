#pragma once
#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "../NetworkCommon/Database/QueryHandler.h"
#include "KhaanContact.h"
#include "UserContact.h"

#include <map>
#include <deque>
using namespace std;

///////////////////////////////////////////////////////////////////

class DiplodocusContact :  public Diplodocus< KhaanContact >
{
public:
   typedef Diplodocus< KhaanContact > Parent;
   enum { SecondsBeforeRemovingLoggedOutUser = 10
   };

public:
   DiplodocusContact( const string& serverName, U32 serverId );
   void     ServerWasIdentified( ChainedInterface* khaan );
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

private:

   int      CallbackFunction();
   void     UpdateAllConnections();

   
   bool     HandleCommandFromGateway( BasePacket* packet, U32 connectionId );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     HandlePacketRequests( PacketContact* packet, U32 connectionId );
   bool     HandleDbQueryResult( PacketDbQueryResult* result );

   bool     ConnectUser( PacketPrepareForUserLogin* login );
   bool     DisconnectUser( PacketPrepareForUserLogout* login );

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
   UserContactMap                   m_users;
   UserIdToContactMap               m_userLookupById;

   
 /*  InitializationStage              m_initializationStage;

   time_t                           m_lastTimeStamp;
   int                              m_queryPeriodicity;
   bool                             m_isExecutingQuery;*/
};

///////////////////////////////////////////////////////////////////