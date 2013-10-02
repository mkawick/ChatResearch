// GamePacket.h
#pragma once 

#include "BasePacket.h"

class PacketGameToServer : public BasePacket 
{
public:
   enum GamePacketType
   {
      GamePacketType_LoginToServer,
      GamePacketType_LoginResponse,

      GamePacketType_CreateGame,
      GamePacketType_CreateGameResponse,
      GamePacketType_DeleteGame,
      GamePacketType_DeleteGameResponse,
      GamePacketType_ForfeitGame,
      GamePacketType_ForfeitGameResponse,
      GamePacketType_QuitGame,
      GamePacketType_QuitGameResponse,

      GamePacketType_InviteUser,
      GamePacketType_InviteUserResponse,
      GamePacketType_YouHaveBeenInvited,
      GamePacketType_AddUser,
      GamePacketType_AddUserResponse,
      GamePacketType_RemoveUser,
      GamePacketType_RemoveUserResponse,

      GamePacketType_AdvanceTurn,
      GamePacketType_TurnWasAdvanced,
      GamePacketType_LoginToGame,
      GamePacketType_LoginToGameResponse,
      GamePacketType_LogoutOfGame,
      GamePacketType_LogoutOfGameResponse,

      GamePacketType_RequestListOfGames,
      GamePacketType_RequestListOfGamesResponse,
      GamePacketType_RequestListOfUsersInGame,
      GamePacketType_RequestListOfUsersInGameResponse,

      GamePacketType_RequestUserWinLoss,
      GamePacketType_RequestUserWinLossResponse,

      GamePacketType_ListOfGames,// usually s2s
      GamePacketType_GameIdentification, // to client
      GamePacketType_RawGameData
   };
public:
   PacketGameToServer( int packet_type = PacketType_Gameplay, int packet_sub_type = GamePacketType_LoginToServer ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////

class PacketCreateGame : public PacketGameToServer
{
public:
   PacketCreateGame(): PacketGameToServer( PacketType_Gameplay, GamePacketType_CreateGame ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   name;
};

///////////////////////////////////////////////////////////////

class PacketCreateGameResponse : public PacketGameToServer
{
public:
   PacketCreateGameResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_CreateGameResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   name;
   string   uuid;
};
      
///////////////////////////////////////////////////////////////

class PacketDeleteGame : public PacketGameToServer
{
public:
   PacketDeleteGame(): PacketGameToServer( PacketType_Gameplay, GamePacketType_DeleteGame ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   uuid;
};

///////////////////////////////////////////////////////////////

class PacketDeleteGameResponse : public PacketGameToServer
{
public:
   PacketDeleteGameResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_DeleteGameResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   name;
   string   uuid;
};

///////////////////////////////////////////////////////////////

class PacketForfeitGame : public PacketGameToServer
{
public:
   PacketForfeitGame(): PacketGameToServer( PacketType_Gameplay, GamePacketType_ForfeitGame ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   string   gameUuid;
};

///////////////////////////////////////////////////////////////

class PacketForfeitGameResponse : public PacketGameToServer
{
public:
   PacketForfeitGameResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_ForfeitGameResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   name;
   string   uuid;
};
      
///////////////////////////////////////////////////////////////

class PacketQuitGame : public PacketGameToServer
{
public:
   PacketQuitGame(): PacketGameToServer( PacketType_Gameplay, GamePacketType_QuitGame ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   string   gameUuid;
};

///////////////////////////////////////////////////////////////

class PacketQuitGameResponse : public PacketGameToServer
{
public:
   PacketQuitGameResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_QuitGameResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userName;
   string   userUuid;
   string   gameUuid;
};

///////////////////////////////////////////////////////////////

class PacketAddUserToGame : public PacketGameToServer
{
public:
   PacketAddUserToGame(): PacketGameToServer( PacketType_Gameplay, GamePacketType_AddUser ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   string   gameUuid;
};

///////////////////////////////////////////////////////////////

class PacketAddUserToGameResponse : public PacketGameToServer
{
public:
   PacketAddUserToGameResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_AddUserResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   string   gameUuid;
   bool     wasSuccessful;
};

///////////////////////////////////////////////////////////////

class PacketRemoveUserFromGame : public PacketGameToServer
{
public:
   PacketRemoveUserFromGame(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RemoveUser ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   string   gameUuid;
};

///////////////////////////////////////////////////////////////

class PacketRemoveUserFromGameResponse : public PacketGameToServer
{
public:
   PacketRemoveUserFromGameResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RemoveUserResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   string   gameUuid;
   bool     wasSuccessful;
};

///////////////////////////////////////////////////////////////

class PacketGameAdvanceTurn : public PacketGameToServer
{
public:
   PacketGameAdvanceTurn(): PacketGameToServer( PacketType_Gameplay, GamePacketType_AdvanceTurn ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   gameUuid;
};

///////////////////////////////////////////////////////////////

class PacketGameAdvanceTurnResponse : public PacketGameToServer
{
public:
   PacketGameAdvanceTurnResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_TurnWasAdvanced ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   gameUuid;
   int      currentTurn;
};

///////////////////////////////////////////////////////////////

class PacketRequestListOfGames : public PacketGameToServer
{
public:
   PacketRequestListOfGames(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RequestListOfGames ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

};

///////////////////////////////////////////////////////////////

class PacketRequestListOfGamesResponse : public PacketGameToServer
{
public:
   PacketRequestListOfGamesResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RequestListOfGamesResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< ChannelInfo >   games; // game details?
};

///////////////////////////////////////////////////////////////

class PacketRequestListOfUsersInGame : public PacketGameToServer
{
public:
   PacketRequestListOfUsersInGame(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RequestListOfUsersInGame ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

};

///////////////////////////////////////////////////////////////

class PacketRequestListOfUsersInGameResponse : public PacketGameToServer
{
public:
   PacketRequestListOfUsersInGameResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RequestListOfUsersInGameResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   gameName;
   string   gameUuid;
   SerializedKeyValueVector< string >   users; // game details?
};


///////////////////////////////////////////////////////////////


struct WinLoss
{
   WinLoss() : isPrivate( false ),
               wins( 0 ),
               losses( 0 ),
               rating( 0 ),
               totalGamesPlayed( 0 ),
               hoursLogged( 0 ),
               minutesLogged( 0 )
   {}
   string   gameUuid;
   bool     isPrivate;
   int      wins;
   int      losses;
   int      rating;
   int      totalGamesPlayed;

   int      hoursLogged;
   int      minutesLogged;

   bool     SerializeIn( const U8* data, int& bufferOffset );
   bool     SerializeOut( U8* data, int& bufferOffset ) const;
};


///////////////////////////////////////////////////////////////

class PacketRequestUserWinLoss : public PacketGameToServer
{
public:
   PacketRequestUserWinLoss(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RequestUserWinLoss ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;   
   string   gameUuid;
};

///////////////////////////////////////////////////////////////

class PacketRequestUserWinLossResponse : public PacketGameToServer
{
public:
   PacketRequestUserWinLossResponse(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RequestUserWinLossResponse ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   userUuid;
   WinLoss  winLoss;
};

///////////////////////////////////////////////////////////////

class PacketListOfGames : public PacketGameToServer
{
public:
   PacketListOfGames(): PacketGameToServer( PacketType_Gameplay, GamePacketType_ListOfGames ), connectionId ( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32      connectionId;
   SerializedKeyValueVector< string >   games;
};

///////////////////////////////////////////////////////////////

class PacketGameIdentification : public PacketGameToServer
{
public:
   PacketGameIdentification(): PacketGameToServer( PacketType_Gameplay, GamePacketType_GameIdentification ), gameId ( 0 ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   U32      gameId;
   string   name;
   string   shortName;
};

///////////////////////////////////////////////////////////////

class PacketGameplayRawData : public PacketGameToServer
{
public:
   PacketGameplayRawData(): PacketGameToServer( PacketType_Gameplay, GamePacketType_RawGameData ), size( 0 ), index( 1 ), dataType( Game ) { data[0] = 0; }
   //PacketGameplayRawData( PacketGameplayRawData& packet ) ;
   //~PacketGameplayRawData();
   //PacketGameplayRawData& operator = ( PacketGameplayRawData& packet ) ;

   bool  SerializeIn( const U8* data, int& bufferOffset ); // allocates memory
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   void  Prep( U16 numBytes, const U8* ptr, int packetIndex );

   enum { MaxBufferSize = 1008-sizeof( PacketGameToServer) }; // 1024 - the other variables
   enum DataType{ Game, Asset, NumDataTypes };
   string   identifier;
   U16      size;
   U16      index; // when subdividing these, these are reverse ordered. 6.5.4.3.2.1 so that we can send up to 254k.We still rely on high layers to serialize properly.
   U8       dataType;
   U8       data[ MaxBufferSize ];
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////
