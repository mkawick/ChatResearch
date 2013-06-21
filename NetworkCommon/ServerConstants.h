#pragma once

#include "ServerType.h"
#include "DataTypes.h"

const static Range<U32> ConnectionIdExclusion = { 0xFFFFFF00, 0xFFFFFFFF };

const U32   ServerToServerConnectionId = ConnectionIdExclusion.high;
const static U32 MaxBufferSize = 8192;


/*
copied from server_game_select

#define GAME_SELECT_ASCENSION       1
#define GAME_SELECT_DOMINION        2
#define GAME_SELECT_THUNDERSTONE    3
#define GAME_SELECT_WOWCMG          4
#define GAME_SELECT_SUMMONWAR       5
#define GAME_SELECT_FOODFIGHT       6
#define GAME_SELECT_NIGHTFALL       7
#define GAME_SELECT_PENNYARCADE     8
#define GAME_SELECT_INFINITECITY    9
#define GAME_SELECT_AGRICOLA        10
#define GAME_SELECT_FLUXX           11
#define GAME_SELECT_SMASHUP         12
#define GAME_SELECT_NUM_GAMES       12
*/
enum GameProductId 
{
   GameProductId_ASCENSION,
   GameProductId_DOMINION,  
   GameProductId_THUNDERSTONE,
   GameProductId_WOWCMG,
   GameProductId_SUMMONWAR,
   GameProductId_FOODFIGHT,  
   GameProductId_NIGHTFALL,  
   GameProductId_PENNYARCADE,
   GameProductId_INFINITECITY,
   GameProductId_AGRICOLA,
   GameProductId_FLUXX,   
   GameProductId_SMASHUP,
   GameProductId_NUM_GAMES 
};