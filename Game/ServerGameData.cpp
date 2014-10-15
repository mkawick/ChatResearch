// ServerGameData.cpp

#include "ServerGameData.h"
#include "DiplodocusGame.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/StringUtils.h"


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------

U32           ServerGameData::m_gameCount = 0;

//-----------------------------------------------------------------------------

ServerGameData::ServerGameData( U32 instanceId, const char* gameServerName ) : m_instanceId( instanceId ), m_gameTurn( 0 ), m_isChatEnabled( true ), m_hasGameStarted( false )
{
   m_gameCount ++;
   GenerateUniqueName( gameServerName );
}

//-----------------------------------------------------------------------------

ServerGameData::~ServerGameData()
{
}

//-----------------------------------------------------------------------------

void        ServerGameData::AdvanceGameTurn()
{
   UserUuidMapIterator itUserUuid = m_users.begin();
   while( itUserUuid != m_users.end() )
   {
   }
}

//-----------------------------------------------------------------------------

bool        ServerGameData::AddUser( const string& username, const string& userUuid, bool isConnected )
{
   UserUuidMapIterator itUserUuid = FindUser( userUuid );
   if( itUserUuid != m_users.end() )
   {
      return false;
   }

   stringhash  userHashLookup = GenerateUniqueHash( userUuid );

   m_users.insert( UserUuidPair( userHashLookup, UserInfo( username, userUuid, isConnected ) ) );

   //m_serverConnection->UserAdded( userUuid, m_instanceId, m_gameInstanceName );
   return true;
}

//-----------------------------------------------------------------------------

bool        ServerGameData::RemoveUser( const string& userUuid )
{
   UserUuidMapIterator itUserUuid = FindUser( userUuid );
   if( itUserUuid != m_users.end() )
   {
      m_users.erase( itUserUuid );
      //m_serverConnection->UserRemoved( userUuid, m_instanceId, m_gameInstanceName );
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------

void        ServerGameData::UserDisconnects( const string& userUuid )
{
   UserUuidMapIterator itUserUuid = FindUser( userUuid );
   if( itUserUuid != m_users.end() )
   {
      itUserUuid->second.isConnected = false;
   }
}

//-----------------------------------------------------------------------------

void        ServerGameData::UserReconnects( const string& userUuid )
{
   UserUuidMapIterator itUserUuid = FindUser( userUuid );
   if( itUserUuid != m_users.end() )
   {
      itUserUuid->second.isConnected = true;
   }
}

//-----------------------------------------------------------------------------

void        ServerGameData::GetListOfUsers() const// packetize this
{
}


//-----------------------------------------------------------------------------

string      ServerGameData::GenerateUniqueName( const char* name )
{
   m_gameInstanceName = name;
   m_gameInstanceName  += "_";
   m_gameInstanceName += GenerateUUID( m_gameCount );

   return m_gameInstanceName ;
}

//-----------------------------------------------------------------------------

ServerGameData::UserUuidMapIterator       ServerGameData::FindUser( const string& userUuid )
{
   stringhash  userHashLookup = GenerateUniqueHash( userUuid );

   return m_users.find( userHashLookup );
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
