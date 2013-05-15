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

