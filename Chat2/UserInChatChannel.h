#pragma once

#include <list>
using namespace std;

#include "../NetworkCommon/DataTypes.h"

struct UserInChatChannel 
{
public:
   string            userName;
   string            userUuid;
   //UserConnection*   connection;
   list< stringhash > channels;
   bool              isOnline;
};