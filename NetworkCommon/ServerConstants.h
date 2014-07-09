#pragma once

#include "ServerType.h"
#include "DataTypes.h"

const static Range<U32> ConnectionIdExclusion = { 0xFFFFFF00, 0xFFFFFFFF };

const U32   ServerToServerConnectionId = ConnectionIdExclusion.high;
const static U32 MaxBufferSize = 32768;


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
   GameProductId_ASCENSION = 1,
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
   GameProductId_TANTO_CUORE,
   GameProductId_LORDS_OF_WATERDEEP,
   GameProductId_MONKEYS_FROM_MARS,
   GameProductId_NUM_GAMES 
};

/*const char* productNames [];
const char* platformStrings[];*/

int   FindProductId( const char* value );
int   FindPlatformId( const char* value );
const char*   FindProductName( int productId );
const char*   FindPlatformName( int platformId );


enum LanguageList // corresponds to the db-language table
{
   LanguageList_english = 1,
   LanguageList_spanish,
   LanguageList_french,
   LanguageList_german,
   LanguageList_italian,
   LanguageList_portuguese,
   LanguageList_russian,
   LanguageList_japanese,
   LanguageList_chinese,
   LanguageList_count
};

enum GameProductType 
{
   GameProductType_Game = 1,
   GameProductType_Dlc,  
   GameProductType_Consumable,
   GameProductType_Entry, // not necessarily tournaments
   GameProductType_currency,
   GameProductType_TournamentEntry,  
   GameProductType_NUM_TYPES 
};
