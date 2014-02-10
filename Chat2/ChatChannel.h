#pragma once

#include <vector>
#include <list>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/ServerType.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

struct UserBasics
{
   UserBasics( const string& name, const string& uuid ) : userName( name ), userUuid( uuid ) {}
   string            userName;
   string            userUuid;
};

//---------------------------------------------------------------

struct ChatChannel
{
   ChatChannel() ;

   int         recordId;
   string      name;
   string      uuid;
   bool        isActive;
   int         maxPlayers;
   string      channelDetails;
   U8          gameType;
   U32         gameId;
   U16         gameTurn;
   string      createDate;

   // TODO.. add expry date for channels.

   //vector< ChatUser >   admins;
   list< stringhash >   userUuidList;
   list< UserBasics >   userBasics;
   

   static const int DefaultMaxNumPlayersInChatchannel = 32;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
