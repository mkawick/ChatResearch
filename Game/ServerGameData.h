#pragma once
// ServerGameData.h

#include "../NetworkCommon/DataTypes.h"
#include <string>
#include <vector>
#include <map>
using namespace std;

class DiplodocusGame;
/////////////////////////////////////////////////////////////////////////////

class UserInfo
{
public:
   UserInfo() : isConnected( true ) {}
   UserInfo( const string& name, const string& userUuid, bool bConnected ) : username( name ), uuid( userUuid ), isConnected( bConnected ) {}

   string   username;
   string   uuid;
   bool     isConnected;
};

/////////////////////////////////////////////////////////////////////////////

class ServerGameData
{
public:
   ServerGameData( U32 instanceId, const char* gameServerName = "agricola" );
   ~ServerGameData();

   U32         GetInstanceId() const { return m_instanceId; }
   int         GetGameTurn() const { return m_gameTurn; }
   void        SetDiplodocusGame( DiplodocusGame* serverConnection ) { m_serverConnection = serverConnection; }

   // cannot advance if a user is disconnected. Send notification to 
   void        AdvanceGameTurn();

   bool        AddUser( const string& username, const string& uuid, bool isConnected = true );
   bool        RemoveUser( const string& uuid );
   void        UserDisconnects( const string& uuid );
   void        UserReconnects( const string& uuid );
   void        GetListOfUsers() const;// packetize this

   //--------------------------------
protected:
   
   typedef map< stringhash, UserInfo >       UserUuidMap;
   typedef UserUuidMap::iterator             UserUuidMapIterator;
   typedef pair< stringhash, UserInfo >      UserUuidPair;

   string      GenerateUniqueName( const char* name );

   UserUuidMapIterator FindUser( const string& uuid );

   string                        m_gameInstanceName;
   int                           m_gameTurn;
   U32                           m_instanceId;
   bool                          m_isChatEnabled;
   bool                          m_hasGameStarted;

   


   UserUuidMap                   m_users;

   static DiplodocusGame*        m_serverConnection;

   //-----------------------------
   static U32           m_gameCount;
};

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
