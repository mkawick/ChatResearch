// UserAccountCommon.cpp

#include <assert.h>

#include "UserAccountCommon.h"

#include "../Logging/server_log.h"

////////////////////////////////////////////////////////////////////////////

ConnectionDetails::ConnectionDetails() :
                  connectionId( 0 ),
                  gatewayId( 0 ),
                  lastLogoutTime( 0 ),
                  gameProductId( 0 )
{
}

////////////////////////////////////////////////////////////////////////////

void  ConnectionDetails::ResetConnection()
{
   time( &lastLogoutTime );
   connectionId = 0;
   gatewayId = 0;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

ConnectionDetailList:: ConnectionDetailList()
{
   for( int i=0; i<GameProductId_NUM_GAMES; i++ )
   {
      connectionDetails[ i ].gameProductId = i;
   }
}

////////////////////////////////////////////////////////////////////////////

void  ConnectionDetailList:: Login( U32 connectionId, U32 gatewayId, U8 gameProductId )
{
   if( gameProductId < GameProductId_NUM_GAMES )
   {
      if( connectionDetails[ gameProductId ].connectionId != 0 )
      {
         LogMessage( LOG_PRIO_INFO, "Error: ConnectionDetailList:: Logout( U32 gameProductId ) connectionId != 0" );
      }
      if( connectionDetails[ gameProductId ].gatewayId != 0 )
      {
         LogMessage( LOG_PRIO_INFO, "Error: ConnectionDetailList:: Logout( U32 gameProductId ) gatewayId != 0" );
      }

      //assert ( connectionDetails[ gameProductId ].connectionId == 0 );// already logged in
      //assert ( connectionDetails[ gameProductId ].gatewayId == 0 );// already logged in

      connectionDetails[ gameProductId ].connectionId = connectionId;
      connectionDetails[ gameProductId ].gatewayId = gatewayId;
      connectionDetails[ gameProductId ].lastLogoutTime = 0;
   }
}

////////////////////////////////////////////////////////////////////////////

void  ConnectionDetailList:: Logout( U32 connectionId )
{
   bool found = false;
   for( int i=0; i<GameProductId_NUM_GAMES; i++ )
   {
      if( connectionDetails[ i ].connectionId == connectionId )
      {
         connectionDetails[ i ].ResetConnection();
         found = true;
         break;
      }
   }

   if( found == false )
   {
      LogMessage( LOG_PRIO_INFO, "Error: ConnectionDetailList:: Logout( U32 connectionId ) connection not found" );
   }
}

////////////////////////////////////////////////////////////////////////////

void  ConnectionDetailList:: Logout( U8 gameProductId )
{
   assert( gameProductId < GameProductId_NUM_GAMES );

   if( connectionDetails[ gameProductId ].connectionId == 0 )
   {
      LogMessage( LOG_PRIO_INFO, "Error: ConnectionDetailList:: Logout( U32 gameProductId ) connectionId == 0" );
   }
   if( connectionDetails[ gameProductId ].gatewayId == 0 )
   {
      LogMessage( LOG_PRIO_INFO, "Error: ConnectionDetailList:: Logout( U32 gameProductId ) gatewayId == 0" );
   }
   
   //assert( connectionDetails[ gameProductId ].connectionId );// non-zero
  // assert( connectionDetails[ gameProductId ].gatewayId );

   connectionDetails[ gameProductId ].ResetConnection();
}

////////////////////////////////////////////////////////////////////////////

time_t   ConnectionDetailList:: GetLastLogoutTime() const
{
   time_t lastLogoutTime = connectionDetails[ 0 ].lastLogoutTime;
   for( int i=1; i<GameProductId_NUM_GAMES; i++ )
   {
      if( connectionDetails[ i ].lastLogoutTime > lastLogoutTime )
      {
         lastLogoutTime = connectionDetails[ i ].lastLogoutTime;
      }
   }
   return lastLogoutTime;
}

bool  ConnectionDetailList:: IsLoggedIntoAnyProduct() const
{
   for( int i=1; i<GameProductId_NUM_GAMES; i++ )
   {
      if( connectionDetails[ i ].connectionId != 0 )
      {
         return true;
      }
   }

   return false;
}


////////////////////////////////////////////////////////////////////////////

bool  ConnectionDetailList:: IsConnected( U32 connectionId ) const 
{
   if( connectionId == 0 )
   {
      for( int i=1; i<GameProductId_NUM_GAMES; i++ )
      {
         U32 connTest = connectionDetails[ i ].connectionId;
         if( connTest != 0 )
            return true;
      }
      return false;
   }

   for( int i=1; i<GameProductId_NUM_GAMES; i++ )
   {
      U32 connTest = connectionDetails[ i ].connectionId;
      if( connTest != 0 && 
         connTest == connectionId )
         return true;
   }
   return false;
}

U32   ConnectionDetailList:: GetFirstConnectedId() const
{
   for( int i=1; i<GameProductId_NUM_GAMES; i++ )
   {
      U32 connTest = connectionDetails[ i ].connectionId;
      if( connTest != 0 )
         return connTest;
   }
   return 0;
}

U32   ConnectionDetailList:: GetGatewayId( U32 connectionId ) const
{
   for( int i=1; i<GameProductId_NUM_GAMES; i++ )
   {
      U32 connTest = connectionDetails[ i ].connectionId;
      if( connTest != 0 && 
         connTest == connectionId )
         return connectionDetails[ i ].gatewayId;
   }
   return 0;
}

U8    ConnectionDetailList:: GetGameProductId( U32 connectionId ) const
{
   for( int i=1; i<GameProductId_NUM_GAMES; i++ )
   {
      U32 connTest = connectionDetails[ i ].connectionId;
      if( connTest != 0 && 
         connTest == connectionId )
         return i;
   }
   return 0;
}

void  ConnectionDetailList:: AssembleAllConnections( vector< SimpleConnectionDetails >& connectionList )
{
   connectionList.clear();

   for( int i=1; i<GameProductId_NUM_GAMES; i++ )
   {
      U32 connId = connectionDetails[ i ].connectionId;
      U32 gateId = connectionDetails[ i ].gatewayId;
      if( connId != 0 && gateId != 0 )
      {
         connectionList.push_back( SimpleConnectionDetails( connId, gateId ) );
      }
   }
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

UserLoginBase:: UserLoginBase(): m_connectionDetails(),
                                 m_hashLookup( 0 ),
                                 m_userId( 0 ),
                                 m_avatarIcon( 0 ),
                                 m_languageId( LanguageList_english ),
                                 m_isValid( false ),
                                 m_isActive( false )

{
}

////////////////////////////////////////////////////////////////////////////

void  UserLoginBase:: Login( U32 connectionId, U32 gatewayId, U8 gameProductId )
{
   m_isValid = true;
   m_isActive = true;
   m_connectionDetails.Login( connectionId, gatewayId, gameProductId );
   time( &m_lastLoginTime );

   PostLogin();
}

////////////////////////////////////////////////////////////////////////////

void  UserLoginBase:: Logout( U32 connectionId )
{
   m_connectionDetails.Logout( connectionId );
   if( m_connectionDetails.IsLoggedIntoAnyProduct() == false )
   {
      m_isValid = false;
   }

   PostLogout();
}

bool  UserLoginBase:: IsConnected( U32 connectionId ) const
{
   return m_connectionDetails.IsConnected( connectionId );
}

U32   UserLoginBase:: GetFirstConnectedId() const
{
   return m_connectionDetails.GetFirstConnectedId();
}

U32   UserLoginBase:: GetGatewayId( U32 connectionId ) const
{
   return m_connectionDetails.GetGatewayId( connectionId );
}

U8    UserLoginBase:: GetGameProductId( U32 connectionId ) const
{
   return m_connectionDetails.GetGameProductId( connectionId );
}

void  UserLoginBase:: AssembleAllConnections( vector< SimpleConnectionDetails >& connectionList )
{
   m_connectionDetails.AssembleAllConnections( connectionList );
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
