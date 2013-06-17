// DiplodocusLogin.h

#pragma once

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "KhaanLogin.h"


class PacketDbQuery;
class PacketDbQueryResult;

//-----------------------------------------------------------------------------------------

struct ConnectionToUser
{
   enum LoginStatus
   {
      LoginStatus_Pending,
      LoginStatus_Invalid,
      LoginStatus_LoggedIn,
      LoginStatus_Hacker
   };

   ConnectionToUser( const string& name, const string& pword, const string& key ) : username( name ), password( pword ), loginKey( key ), status( LoginStatus_Pending ) {}
   string   username;
   string   password;
   string   email;
   string   userUuid;
   string   loginKey;
   string   lastLoginTime;

   LoginStatus status;
};


//-----------------------------------------------------------------------------------------

enum LanguageList // corresponds to the db-language table
{
   LanguageList_english = 1,
   LanguageList_spanish,
   LanguageList_french,
   LanguageList_german,
   LanguageList_italian,
   LanguageList_portuguese,
   LanguageList_russian,
   LanguageList_japanese,
   LanguageList_chinese
};

//-----------------------------------------------------------------------------------------

class DiplodocusLogin : public Diplodocus< KhaanLogin >
{
   enum QueryType 
   {
      QueryType_UserLoginInfo = 1,
      QueryType_UserListOfGame,
      QueryType_UserCreateTempAccount,
      QueryType_UserCheckForNewAccount,
      QueryType_UserFindBlankUUID,
      QueryType_LoadStrings,
      QueryType_LoadWeblinks
   };

public:
   DiplodocusLogin( const string& serverName, U32 serverId );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );

private:

   int      CallbackFunction();

   bool     AddQueryToOutput( PacketDbQuery* query );
   bool     LogUserIn( const string& username, const string& password, const string& loginKey, U32 connectionId );
   bool     LogUserOut( U32 connectionId );

   bool     SuccessfulLogin( U32 connectionId );
   bool     ForceUserLogoutAndBlock( U32 connectionId );
   bool     CreateAccount( const char* username, const char* emailAddress, const char* password, int userId, int gameId );
   bool     RequestListOfGames( U32 connectionId, const string& userUuid );
   bool     SendListOfGamesToGameServers( U32 connectionId, const KeyValueVector& kv_array );

   bool     UpdateLastLoggedOutTime( U32 connectionId );
   bool     UpdateLastLoggedInTime( U32 connectionId );

   bool     SendLoginStatusToOtherServers( const string& username, const string& userUuid, U32 connectionId, const string& lastLoginTime, bool isLoggedIn );

   bool     SendPacketToGateway( BasePacket*, U32 connectionId );

   //---------------------------------------------------------------
   typedef map< U32, ConnectionToUser >      UserConnectionMap;
   typedef pair< U32, ConnectionToUser >     UserConnectionPair;
   typedef UserConnectionMap::iterator       UserConnectionMapIterator;

   typedef map< stringhash, vector< string > >    StringTableLookup;
   typedef pair< stringhash, vector< string > >   StringTableLookupPair;

   UserConnectionMap    m_userConnectionMap;
   bool                 m_updateGatewayConnections;

   /// the following is all related to email services
   void     PreloadLanguageStrings();
   void     SaveStrings( const PacketDbQueryResult* dbResult );
   string   GetString( const string& stringName, int languageId );
   bool                 m_hasLoadedStringTable;
   StringTableLookup    m_stringsTable;

   void     PreloadWeblinks();
   void     HandleWeblinks( const PacketDbQueryResult* dbResult );
   string   m_linkToAccountCreated;
   bool     m_hasLoadedWeblinks;

   // TO BE REMOVED (once the login server does not rely on other services updating the new account creation)

   void     FillInUserAccountUUIDs();
   void     HandleBlankUUIDs( PacketDbQueryResult* dbResult );
   void     UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId );
   void     CheckForNewAccounts();
   void     HandleNewAccounts( const PacketDbQueryResult* dbResult );

   time_t   m_checkOnBlankUuidTimer;
   int      m_checkOnBlankUuidTimeoutSeconds;
   time_t   m_newAccountCreationTimer;
   int      m_newAccountTimeoutSeconds;

   // TO BE REMOVED 
};

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------