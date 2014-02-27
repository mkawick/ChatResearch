#pragma once

#include <vector>
#include <list>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/ServerType.h"
#include "../NetworkCommon/Packets/BasePacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class ChatChannelDbJob
{
public:
   enum JobType
   {
      JobType_Create,
      JobType_Delete,
      JobType_LoadSingleChannel,
      JobType_LoadAllChannels,
      JobType_LoadAllUsers,
      JobType_SelectAllChannelsForAUser,
      JobType_Exists,
      JobType_AddUser,
      JobType_RemoveUser,
      JobType_SelectAllChannelsToSendToAuth,
      JobType_SelectAllUsersToSendToAuth,
      JobType_FindChatters,
      JobType_AllUsersInChannel,
      JobType_AllUsersInAllChannels,

      JobType_CreateFromGameServer,
      JobType_MakeInactiveFromGameServer,
      JobType_AddUserFromGameServer,
      JobType_RemoveUserFromGameServer
   };

   string         debugString;

   string         lookupString;
   stringhash     chatUserLookup;
   stringhash     authUserLookup;
   int            jobId;
   U32            serverId;
   JobType        jobType;
   StringBucket   stringBucket;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////