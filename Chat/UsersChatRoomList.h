#pragma once

#include <list>
using namespace std;

#include "../NetworkCommon/DataTypes.h"

struct UsersChatRoomList
{
public:

   UsersChatRoomList( const string& uuid ) : userUuid( uuid ) {}
   // default copy operator and copy c'tor

   
   string               userUuid;
   U32                  userId;
   list< stringhash >   channels;
   bool                 isOnline;
   bool                 blockContactInvites;
   bool                 blockGroupInvites;

   string               userName;
   //UserConnection*   connection;
};