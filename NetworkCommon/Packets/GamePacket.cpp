// GamePacket.cpp

#include "../ServerConstants.h"
#include "GamePacket.h"
#include "Serialize.h"
#include "PacketFactory.h"
#include <assert.h>

///////////////////////////////////////////////////////////////

bool  PacketGameToServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketGameToServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCreateGame::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );

   return true;
}

bool  PacketCreateGame::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketCreateGameResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );

   return true;
}

bool  PacketCreateGameResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketDeleteGame::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );

   return true;
}

bool  PacketDeleteGame::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketDeleteGameResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );

   return true;
}

bool  PacketDeleteGameResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketForfeitGame::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

bool  PacketForfeitGame::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketForfeitGameResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );

   return true;
}

bool  PacketForfeitGameResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketQuitGame::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

bool  PacketQuitGame::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketQuitGameResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

bool  PacketQuitGameResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketAddUserToGame::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

bool  PacketAddUserToGame::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketAddUserToGameResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );
   Serialize::In( data, bufferOffset, wasSuccessful, minorVersion );

   return true;
}

bool  PacketAddUserToGameResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );
   Serialize::Out( data, bufferOffset, wasSuccessful, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRemoveUserFromGame::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

bool  PacketRemoveUserFromGame::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRemoveUserFromGameResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );
   Serialize::In( data, bufferOffset, wasSuccessful, minorVersion );

   return true;
}

bool  PacketRemoveUserFromGameResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );
   Serialize::Out( data, bufferOffset, wasSuccessful, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGameAdvanceTurn::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

bool  PacketGameAdvanceTurn::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGameAdvanceTurnResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );
   Serialize::In( data, bufferOffset, currentTurn, minorVersion );

   return true;
}

bool  PacketGameAdvanceTurnResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );
   Serialize::Out( data, bufferOffset, currentTurn, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestListOfGames::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketRequestListOfGames::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestListOfGamesResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, games, minorVersion );

   return true;
}

bool  PacketRequestListOfGamesResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, games, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestListOfUsersInGame::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketRequestListOfUsersInGame::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketRequestListOfUsersInGameResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, gameName, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );
   Serialize::In( data, bufferOffset, users, minorVersion );

   return true;
}

bool  PacketRequestListOfUsersInGameResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, gameName, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );
   Serialize::Out( data, bufferOffset, users, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  WinLoss::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );
   Serialize::In( data, bufferOffset, isPrivate, minorVersion );
   Serialize::In( data, bufferOffset, wins, minorVersion );
   Serialize::In( data, bufferOffset, losses, minorVersion );

   Serialize::In( data, bufferOffset, rating, minorVersion );
   Serialize::In( data, bufferOffset, totalGamesPlayed, minorVersion );
   Serialize::In( data, bufferOffset, hoursLogged, minorVersion );
   Serialize::In( data, bufferOffset, minutesLogged, minorVersion );

   return true;
}

bool  WinLoss::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );
   Serialize::Out( data, bufferOffset, isPrivate, minorVersion );
   Serialize::Out( data, bufferOffset, wins, minorVersion );
   Serialize::Out( data, bufferOffset, losses, minorVersion );

   Serialize::Out( data, bufferOffset, rating, minorVersion );
   Serialize::Out( data, bufferOffset, totalGamesPlayed, minorVersion );
   Serialize::Out( data, bufferOffset, hoursLogged, minorVersion );
   Serialize::Out( data, bufferOffset, minutesLogged, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketRequestUserWinLoss::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameUuid, minorVersion );

   return true;
}

bool  PacketRequestUserWinLoss::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameUuid, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketRequestUserWinLossResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, winLoss, minorVersion );

   return true;
}

bool  PacketRequestUserWinLossResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, winLoss, minorVersion );

   return true;
}



///////////////////////////////////////////////////////////////

bool  PacketListOfGames::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, connectionId, minorVersion );
   Serialize::In( data, bufferOffset, games, minorVersion );

   return true;
}

bool  PacketListOfGames::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, connectionId, minorVersion );
   Serialize::Out( data, bufferOffset, games, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketGameIdentification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );
   Serialize::In( data, bufferOffset, shortName, minorVersion );

   return true;
}

bool  PacketGameIdentification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );
   Serialize::Out( data, bufferOffset, shortName, minorVersion );

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

bool  PacketGameplayRawData::SerializeIn( const U8* buffer, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( buffer, bufferOffset, minorVersion );
   Serialize::In( buffer, bufferOffset, identifier, minorVersion );
   Serialize::In( buffer, bufferOffset, size, minorVersion );
   Serialize::In( buffer, bufferOffset, dataType, minorVersion );
   Serialize::In( buffer, bufferOffset, index, minorVersion );
   assert( size > 0 && size <= MaxBufferSize );

   //data = new U8[size + 1];
   memcpy( data, buffer + bufferOffset, size );
   bufferOffset += size;
   data[ size ] = 0;// null terminate

   return true;
}

bool  PacketGameplayRawData::SerializeOut( U8* buffer, int& bufferOffset, int minorVersion ) const
{ 
   //assert( size > 0 && size <= MaxBufferSize );// can sometime be used for testing

   PacketGameToServer::SerializeOut( buffer, bufferOffset, minorVersion );
   Serialize::Out( buffer, bufferOffset, identifier, minorVersion );
   Serialize::Out( buffer, bufferOffset, size, minorVersion );
   Serialize::Out( buffer, bufferOffset, dataType, minorVersion );
   Serialize::Out( buffer, bufferOffset, index, minorVersion );

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


bool  PacketGame_Notification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketGameToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, notificationType, minorVersion );
   Serialize::In( data, bufferOffset, additionalText, minorVersion );

   return true;
}

bool  PacketGame_Notification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketGameToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, notificationType, minorVersion );
   Serialize::Out( data, bufferOffset, additionalText, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////