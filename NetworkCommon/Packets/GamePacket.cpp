// GamePacket.cpp

#include "../ServerConstants.h"
#include "GamePacket.h"
#include "../Serialize.h"
#include "PacketFactory.h"
#include <assert.h>

///////////////////////////////////////////////////////////////

bool  PacketGameToServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketGameToServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCreateGame::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );

   return true;
}

bool  PacketCreateGame::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCreateGameResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketCreateGameResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketDeleteGame::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketDeleteGame::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketDeleteGameResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketDeleteGameResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketForfeitGame::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameUuid );

   return true;
}

bool  PacketForfeitGame::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketForfeitGameResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketForfeitGameResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketQuitGame::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameUuid );

   return true;
}

bool  PacketQuitGame::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketQuitGameResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameUuid );

   return true;
}

bool  PacketQuitGameResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketAddUserToGame::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameUuid );

   return true;
}

bool  PacketAddUserToGame::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketAddUserToGameResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameUuid );
   Serialize::In( data, bufferOffset, wasSuccessful );

   return true;
}

bool  PacketAddUserToGameResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameUuid );
   Serialize::Out( data, bufferOffset, wasSuccessful );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRemoveUserFromGame::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameUuid );

   return true;
}

bool  PacketRemoveUserFromGame::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRemoveUserFromGameResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameUuid );
   Serialize::In( data, bufferOffset, wasSuccessful );

   return true;
}

bool  PacketRemoveUserFromGameResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameUuid );
   Serialize::Out( data, bufferOffset, wasSuccessful );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGameAdvanceTurn::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, gameUuid );

   return true;
}

bool  PacketGameAdvanceTurn::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, gameUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGameAdvanceTurnResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, gameUuid );
   Serialize::In( data, bufferOffset, currentTurn );

   return true;
}

bool  PacketGameAdvanceTurnResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, gameUuid );
   Serialize::Out( data, bufferOffset, currentTurn );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestListOfGames::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketRequestListOfGames::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestListOfGamesResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, games );

   return true;
}

bool  PacketRequestListOfGamesResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, games );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestListOfUsersInGame::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketRequestListOfUsersInGame::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestListOfUsersInGameResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, gameName );
   Serialize::In( data, bufferOffset, gameUuid );
   Serialize::In( data, bufferOffset, users );

   return true;
}

bool  PacketRequestListOfUsersInGameResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, gameName );
   Serialize::Out( data, bufferOffset, gameUuid );
   Serialize::Out( data, bufferOffset, users );

   return true;
}

///////////////////////////////////////////////////////////////

bool  WinLoss::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, gameUuid );
   Serialize::In( data, bufferOffset, isPrivate );
   Serialize::In( data, bufferOffset, wins );
   Serialize::In( data, bufferOffset, losses );

   Serialize::In( data, bufferOffset, rating );
   Serialize::In( data, bufferOffset, totalGamesPlayed );
   Serialize::In( data, bufferOffset, hoursLogged );
   Serialize::In( data, bufferOffset, minutesLogged );

   return true;
}

bool  WinLoss::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, gameUuid );
   Serialize::Out( data, bufferOffset, isPrivate );
   Serialize::Out( data, bufferOffset, wins );
   Serialize::Out( data, bufferOffset, losses );

   Serialize::Out( data, bufferOffset, rating );
   Serialize::Out( data, bufferOffset, totalGamesPlayed );
   Serialize::Out( data, bufferOffset, hoursLogged );
   Serialize::Out( data, bufferOffset, minutesLogged );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketRequestUserWinLoss::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameUuid );

   return true;
}

bool  PacketRequestUserWinLoss::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameUuid );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketRequestUserWinLossResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, winLoss );

   return true;
}

bool  PacketRequestUserWinLossResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, winLoss );

   return true;
}



///////////////////////////////////////////////////////////////

bool  PacketListOfGames::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, connectionId );
   Serialize::In( data, bufferOffset, games );

   return true;
}

bool  PacketListOfGames::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, connectionId );
   Serialize::Out( data, bufferOffset, games );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGameIdentification::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, shortName );

   return true;
}

bool  PacketGameIdentification::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, shortName );

   return true;
}

///////////////////////////////////////////////////////////////
/*
PacketGameplayRawData::PacketGameplayRawData( PacketGameplayRawData& packet ) 
{
   this->size = packet.size;
   this->data = packet.data;
   packet.data = NULL;
}
PacketGameplayRawData& PacketGameplayRawData::operator = ( PacketGameplayRawData& packet ) 
{
   if( this->data )
      delete this->data;
   this->size = packet.size;
   this->data = packet.data;
   packet.data = NULL;

   return *this;
}

PacketGameplayRawData::~PacketGameplayRawData() 
{ 
   if( data ) 
      delete data; 
   data = NULL;// pointless
}*/

bool  PacketGameplayRawData::SerializeIn( const U8* buffer, int& bufferOffset )
{
   PacketGameToServer::SerializeIn( buffer, bufferOffset );
   Serialize::In( buffer, bufferOffset, identifier );
   Serialize::In( buffer, bufferOffset, size );
   Serialize::In( buffer, bufferOffset, dataType );
   Serialize::In( buffer, bufferOffset, index );
   assert( size > 0 && size <= MaxBufferSize );

   //data = new U8[size + 1];
   memcpy( data, buffer + bufferOffset, size );
   bufferOffset += size;
   data[ size ] = 0;// null terminate

   return true;
}

bool  PacketGameplayRawData::SerializeOut( U8* buffer, int& bufferOffset ) const
{ 
   assert( size > 0 && size <= MaxBufferSize );

   PacketGameToServer::SerializeOut( buffer, bufferOffset );
   Serialize::Out( buffer, bufferOffset, identifier );
   Serialize::Out( buffer, bufferOffset, size );
   Serialize::Out( buffer, bufferOffset, dataType );
   Serialize::Out( buffer, bufferOffset, index );

   memcpy( buffer + bufferOffset, data, size );
   bufferOffset += size;

   return true;
}

void  PacketGameplayRawData::Prep( U16 numBytes, const U8* ptr, int packetIndex )
{
   /*if( data )
      delete data;*/

   size = numBytes;
   //data = new U8[size];
   memcpy( data, ptr, size );
   index = packetIndex;
}

///////////////////////////////////////////////////////////////


