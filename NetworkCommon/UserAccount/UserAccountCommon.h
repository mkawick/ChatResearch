// UserAccountCommon.h
#pragma once

#include <time.h>
#include <set>
#include <vector>
#include <string>
using namespace std;

#include "../DataTypes.h"
#include "../ServerConstants.h"

////////////////////////////////////////////////////////////////////////////

struct SimpleConnectionDetails
{
   SimpleConnectionDetails( U32 connId, U32 gateId ) : connectionId( connId ), gatewayId( gateId ) {}
   U32         connectionId;
   U32         gatewayId;
};

typedef vector< SimpleConnectionDetails > UserConnectionList;

class ConnectionDetails
{
public:
   ConnectionDetails();
   void  ResetConnection();
   U32         connectionId;
   U32         gatewayId;
   time_t      lastLogoutTime;
   U8          gameProductId;// unnecessary, but possibly helpful
   U8          pad [7];
};


class ConnectionDetailList
{
public:
   ConnectionDetailList();
   void  Login( U32 connectionId, U32 gatewayId, U8 gameProductId );
   void  Logout( U32 connectionId );
   void  Logout( U8 gameProductId );

   bool  IsLoggedIntoAnyProduct() const;
   bool  IsConnected( U32 connectionId ) const;
   U32   GetFirstConnectedId() const;
   U32   GetGatewayId( U32 connectionId ) const;
   U8    GetGameProductId( U32 connectionId ) const;

   bool  IsLoggedInWithThisProduct( U8 gameProductId );

   void  AssembleAllConnections( vector< SimpleConnectionDetails >& connectionList );

   ConnectionDetails connectionDetails[ GameProductId_NUM_GAMES ];

   time_t         GetLastLogoutTime() const;
};

////////////////////////////////////////////////////////////////////////////

// the following class is used for each server to track user login status.

class UserLoginBase
{
public:
   UserLoginBase();

   void           Login( U32 connectionId, U32 gatewayId, U8 gameProductId );
   void           Logout( U32 connectionId );

   virtual void   Update() {}

   time_t         GetLastLogoutTime() const  { return m_connectionDetails.GetLastLogoutTime(); }

   bool           IsConnected( U32 connectionId = 0 ) const;
   U32            GetFirstConnectedId() const;
   U32            GetGatewayId( U32 connectionId ) const;
   U8             GetGameProductId( U32 connectionId ) const;

   void           AssembleAllConnections( vector< SimpleConnectionDetails >& connectionList );

   //------------------------------------------

   void           SetHashLookup( stringhash h )                   { m_hashLookup = h; }
   stringhash     GetHashLookup()      const { return m_hashLookup; }// user name has usually
   int            GetConnectionId( int gatewayId ) const; // lookup

   const U32      GetId()              const { return m_userId; }
   const string&  GetUsername()        const { return m_userName; }
   
   const string&  GetUuid()            const { return m_userUuid; }
   const string&  GetPassword()        const { return m_passwordHash; }
   const string&  GetEmail()           const { return m_email; }
   const string&  GetAssetServerKey()  const { return m_assetServerKey; }
   const time_t   GetLastLoginTime()   const { return m_lastLoginTime; }
   //const string&  GetLastLogoutTime()  const { return m_lastLogoutTime; }
   const string&  GetUserMotto()       const { return m_userMotto; }
   int            GetUserAvatar()      const { return m_avatarIcon; }

   U32            GetLanguageId()      const { return m_languageId; }
   bool           GetIsValid()         const { return m_isValid; }
   bool           GetIsActive()        const { return m_isActive; }

   void           SetId( U32 id )                     { m_userId = id; }
   void           SetUserName( const string& name )   { m_userName = name; }
                      
   void           SetUuid( const string& uuid )       { m_userUuid = uuid; }
   void           SetPassword( const string& pass )   { m_passwordHash = pass; }
   void           SetEmail( const string& email )     { m_email = email; }
   void           SetAssetKey( const string& key )    { m_assetServerKey = key; }
   //void           SetUuid( const string& uuid )       { m_lastLoginTime = uuid; }
   //void           SetUuid( const string& uuid )       { m_lastLogoutTime = uuid; }
   void           SetUserMotto( const string& motto ) { m_userMotto = motto; }
   void           SetUserAvatar( int iconId )         { m_avatarIcon = iconId; }
   void           SetLanguageId( U32 languageId )     { m_languageId = languageId; }
   void           SetIsValid( bool isValid )          { m_isValid = isValid; }
   void           SetIsActive( bool isActive )        { m_isActive = isActive; }

protected:

   virtual void            PostLogin() {}
   virtual void            PostLogout() {}

   stringhash              m_hashLookup;
   U32                     m_userId;
   string                  m_userName;
   string                  m_userUuid;
   string                  m_passwordHash;
   string                  m_email;
   string                  m_assetServerKey;
   time_t                  m_lastLoginTime;
   //string                  m_lastLogoutTime;
   string                  m_userMotto;
   int                     m_avatarIcon;
   int                     m_languageId;
   bool                    m_isValid;
   bool                    m_isActive;

   ConnectionDetailList    m_connectionDetails;
};

////////////////////////////////////////////////////////////////////////////
