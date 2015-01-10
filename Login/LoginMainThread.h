// LoginMainThread.h

#pragma once


#include "UserLogin.h"

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Stat/StatTrackingConnections.h"
#include "../NetworkCommon/Database/QueryHandler.h"

#include "KhaanLogin.h"
#include "FruitadensLogin.h"
#include "CreateAccountResultsAggregator.h"
#include "LoginCommon.h"

#include <ctime>

#include <set>
using namespace std;

class PacketDbQuery;
class PacketDbQueryResult;
class PacketCheat;
struct PurchaseEntry;

class PacketListOfUserPurchasesRequest;
class StringLookup;
class PacketLoginFromGateway;
class ScheduledOutageDBReader;

class ProductManager;

/////////////////////////////////////////////////////////////////////////////////

class LoginMainThread : public Queryer, public Diplodocus< KhaanLogin, FruitadensLogin >, public StatTrackingConnections
{
public:
   LoginMainThread( const string& serverName, U32 serverId );
   const char*    GetClassName() const { return "DiplodocusLoginOld"; }

   void           Init();

   void           PrintPacketTypes( bool printingOn );
   void           PrintFunctionNames( bool printingOn );
   bool           IsPrintingFunctionNames() const { return m_printFunctionNames; }
   bool           IsPrintingVerbose() const { return m_printPacketTypes; }
   void           AutoAddTheProductFromWhichYouLogin( bool addIt = false ) { m_autoAddProductFromWhichUsersLogin = addIt; }

   //---------------------------------------------
public: // supporting the base class functionality
   void           ServerWasIdentified( IChainedInterface* khaan );
   void           InputConnected( IChainedInterface* chainedInput );
   void           InputRemovalInProgress( IChainedInterface * chainedInput );

   bool           AddInputChainData( BasePacket* packet, U32 gatewayId );
   bool           AddOutputChainData(BasePacket *,U32);
   bool           AddQueryToOutput( PacketDbQuery* query );

   bool           SendPacketToOtherServer( BasePacket* packet, U32 connectionId );

   StringLookup*  GetStringLookup() { return m_stringLookup; }   
   ProductManager*GetProductManager() { return m_productManager; }
   

   U32            GetGatewayId( U32 userConnectionId );
   bool           FixupLookupInfo( U32 userConnectionId, const string& userName, const string& userUuid );
   //---------------------------------------------
   
   bool           IsUserConnectionValid( U32 connectionId );

   void           SendListOfUserProductsToAssetServer( U32 userConnectionId, U32 gatewayId );

public: 

   enum QueryType 
   {
      QueryType_UserQuery,
      QueryType_Products,
      QueryType_ProductStringLookup,
      QueryType_ScheduledOutage,
      QueryType_CreateAccountResults
   };

private:
   int      CallbackFunction();
   void     UpdateDbResults();
   void     RemoveOldConnections();
   bool     ProcessInboundPacket( PacketStorage& storage );
   bool     ProcessOutboundPacket( PacketStorage& storage );

   void     LoadInitializationData();

   void     LogUserIn( const PacketLoginFromGateway* loginPacket, U32 gatewayId, U32 userConnectionId, U8 gameProductId );
   bool     LoadUserWithoutLogin( const string& uuid, U32 userConnectionIdRequester );
   bool     LogoutListOfUsers( const PacketLogin_LogoutAllUsers* usersLogout, U32 gatewayId );
   //---------------------------------------------

   bool     HandleDbResult( const PacketDbQueryResult* dbResult );
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );

   void     TrackCountStats( StatTracking stat, float value, int sub_category );
   void     RunHourlyStats();
   void     RunDailyStats();
   void     ClearOutUniqueUsersNotLoggedIn();

private:/// account creation
   bool     UserAccountCreationHandler( U32 userConnectionId, const PacketDbQueryResult* );


   typedef map< stringhash, UserLogin* >           UserLoginMap;// uuid
   typedef pair< stringhash, UserLogin* >          UserLoginPair;
   typedef UserLoginMap::iterator                  UserLoginMapIterator;
   typedef UserLoginMap::const_iterator            UserLoginMapConstIterator;

   typedef map< U32, stringhash >                  UserConnectionLookup;
   typedef pair< U32, stringhash >                 UserConnectionLookupPair;
   typedef UserConnectionLookup::iterator          UserConnectionLookupIterator;

   typedef map< stringhash, vector< string > >     StringTableLookup;
   typedef pair< stringhash, vector< string > >    StringTableLookupPair;

   
  
private:
   bool                       m_isInitialized;
   bool                       m_isInitializing;
   bool                       m_autoAddProductFromWhichUsersLogin;
   bool                       m_printPacketTypes;
   bool                       m_printFunctionNames;

   
   ProductManager*            m_productManager;
   StringLookup*              m_stringLookup;
   ScheduledOutageDBReader*   m_outageReader;

   time_t                     m_initializingProductListTimeStamp;

private:
    // utils:
   UserLoginMapIterator       AddUserConnection( const string& userName, const string& password, U32 userConnectionId );
   UserLoginMapIterator       FindUserConnection( U32 userConnectionId );
   UserLoginMapIterator       FindUserByUuid( const string& uuid );
   UserLoginMapIterator       FindUser( const string& userName );
   bool                       RemoveUserConnection( U32 userConnectionId );
   bool                       SendExpirationToAllOtherServers( const string& userName, const string& uuid );

private: // stat tracking
   int                        m_numRelogins;
   int                        m_numFailedLogins;
   int                        m_numSuccessfulLogins;
   int                        m_totalUserLoginSeconds;
   int                        m_totalNumLogouts;
   int                        m_temporaryConnectionId;

   UserConnectionLookup       m_userConnectionLookupMap;
   UserLoginMap               m_userList;
   set< string >              m_uniqueUsers;

   CreateAccountResultsAggregator*  m_accountCreator;


private:
   // stat tracking
   time_t                     m_timestampHourlyStatServerStatisics;
   static const U32           timeoutHourlyStatisics = 60*60;
   time_t                     m_timestampDailyStatServerStatisics;
   static const U32           timeoutDailyStatisics = timeoutHourlyStatisics*24;
   static const U32           timeoutLogoutExpireTime = timeoutHourlyStatisics * 3;// three hours

  /* m_stringLookup( NULL ),
   m_numRelogins( 0 ),
   m_numFailedLogins( 0 ),
   m_numSuccessfulLogins( 0 ),
   m_totalUserLoginSeconds( 0 ),
   m_totalNumLogouts( 0 ),
   
   m_outageReader( NULL )*/

public:
   bool           ForceUserLogoutAndBlock( U32 userConnectionId, U32 gatewayId );
   bool           BroadcastLoginStatus( U32 connectionId, 
                                      U8 gameProductId, 
                                      bool isLoggedIn, 
                                      bool wasDisconnectedByError,
                                      U32 gatewayId );
   bool           BroadcastLoginStatusSimple( U32 userConnectionId, 
                                               U8 gameProductId, 
                                               bool isLoggedIn, 
                                               bool wasDisconnectedByError,
                                               U32 gatewayId );
   bool           SendLoginStatus(  ChainType*  destinationServerPtr,
                              const string& userName, 
                              const string& userUuid, 
                              U32 connectionId, 
                              U8 gameProductId, 
                              const string& lastLoginTime, 
                              bool isActive, 
                              const string& email, 
                              const string& passwordHash, 
                              const string& userId, 
                              const string& loginKey,
                              int languageId, 
                              bool isLoggedIn, 
                              bool wasDisconnectedByError,
                              U32 gatewayId );
   bool           SendLoginStatusToOtherServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 connectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError,
                                                     U32 gatewayId );
   bool           SendLoginStatusTo_Non_GameServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 connectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError,
                                                     U32 gatewayId );
   bool           SendLoginStatusTo_GameServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 connectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError,
                                                     U32 gatewayId );
};


/////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////

