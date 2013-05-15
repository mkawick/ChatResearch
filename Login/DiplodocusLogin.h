#pragma once
#include "../NetworkCommon/NetworkIn/diplodocus.h"
#include "KhaanLogin.h"

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
   string   userUuid;
   string   loginKey;
   string   lastLoginTime;

   LoginStatus status;
   
};

//-----------------------------------------------------------------------------------------

class DiplodocusLogin : public Diplodocus< KhaanLogin >
{
   enum QueryType 
   {
      QueryType_UserLoginInfo = 1
   };

public:
   DiplodocusLogin();

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );
   bool     AddOutputChainData( BasePacket* packet, U32 connectionId );

private:

   int      CallbackFunction();

   bool     AddQueryToOutput( PacketDbQuery* query );
   bool     LogUserIn( const string& username, const string& password, const string& loginKey, U32 connectionId );
   bool     LogUserOut( U32 connectionId );

   bool     SuccessfulLogin( U32 connectionId );
   bool     ForceUserLogoutAndBlock( U32 connectionId );

   bool     UpdateLastLoggedOutTime( U32 connectionId );
   bool     UpdateLastLoggedInTime( U32 connectionId );

   bool     SendLoginStatusToOtherServers( const string& username, const string& userUuid, U32 connectionId, const string& lastLoginTime, bool isLoggedIn );

   bool     SendPacketToGateway( BasePacket*, U32 connectionId );

   //---------------------------------------------------------------
   typedef map< U32, ConnectionToUser >      UserConnectionMap;
   typedef pair< U32, ConnectionToUser >     UserConnectionPair;
   typedef UserConnectionMap::iterator       UserConnectionMapIterator;

   UserConnectionMap    m_userConnectionMap;
   bool                 m_updateGatewayConnections;
};

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------