// PacketFactory.cpp

#include <algorithm>
#include <iostream>
#include <map>
using namespace std;

#include <assert.h>

#include "PacketFactory.h"
#include "BasePacket.h"
#include "ChatPacket.h"
#include "GamePacket.h"
#include "ServerToServerPacket.h"

PacketFactory::PacketFactory(){}

template< typename PacketType >
PacketType* SerializeIn( const U8* bufferIn, int& bufferOffset )
{
   PacketType* logout = new PacketType();
   logout->SerializeIn( bufferIn, bufferOffset );

   return logout;
}

//-----------------------------------------------------------------------------------------

bool	PacketFactory::Parse( const U8* bufferIn, int& bufferOffset, BasePacket** packetOut ) const
{
   *packetOut = NULL;// just to be sure that no one misuses this

   //const BasePacket* testPtr = reinterpret_cast< const BasePacket* > ( bufferIn );
   BasePacket firstPassParse;
   int offset = bufferOffset;
   firstPassParse.SerializeIn( bufferIn, offset );

   switch( firstPassParse.packetType )
   {
   case PacketType_Base:
      //assert(0);// we should never receive one of these
      return false;

   case PacketType_Login:
      {
         return ParseLogin( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_Chat:
      {
         return ParseChat( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_UserInfo:
      {
         return ParseUserInfo( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_DbQuery:
      {   
         return ParseDbQuery( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_ServerToServerWrapper:
      {   
         return ParseServerToServerWrapper( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_ServerInformation:
      {
         return ParseServerInfo( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_GatewayWrapper:
      {   
         return ParseGatewayWrapper( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_Gameplay:
      {
         return ParseGame( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_ErrorReport:
      {   
         PacketErrorReport* error = new PacketErrorReport();
         error->SerializeIn( bufferIn, bufferOffset );
         *packetOut = error;
         return true;
      }
   }

   return false;
}// be sure to check the return value

//---------------------------------------------------------------------------------------------------------------------------------------

bool     PacketFactory::SafeParse( const U8* bufferIn, int& bufferOffset, BasePacket& packetOut ) const
{
   int offset = bufferOffset;
   packetOut.SerializeIn( bufferIn, offset );
   return true;
}

//---------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------

bool  PacketFactory::ParseLogin( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType )
   {
      case PacketLogin::LoginType_Login:
         {
            *packetOut = SerializeIn< PacketLogin >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_Logout:
         {
            *packetOut = SerializeIn< PacketLogout >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_PacketLogoutToClient:
         {
            *packetOut = SerializeIn< PacketLogoutToClient >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_InformClientOfLoginStatus:
         {
            *packetOut = SerializeIn< PacketLoginToClient >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_InformGatewayOfLoginStatus:
         {
            *packetOut = SerializeIn< PacketLoginToGateway >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogin:
         {
            *packetOut = SerializeIn< PacketPrepareForUserLogin >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogout:
         {
            *packetOut = SerializeIn< PacketPrepareForUserLogout >( bufferIn, bufferOffset );
         }
         return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool  PacketFactory::ParseChat( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketChatToServer::ChatType_ChatToServer:
      {
         *packetOut = SerializeIn< PacketChatToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_ChatToClient:
      {
         *packetOut = SerializeIn< PacketChatToClient >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_UserChatStatusChange:
      {
         *packetOut = SerializeIn< PacketChatUserStatusChangeBase >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_ChangeChatChannel:
      {
         *packetOut = SerializeIn< PacketChangeChatChannel >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_RequestHistory:
      {
         *packetOut = SerializeIn< PacketChatHistoryRequest >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_ChangeChatChannelToClient:
      {
         *packetOut = SerializeIn< PacketChangeChatChannelToClient >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_SendListOfChannelsToClient:
      {
         *packetOut = SerializeIn< PacketChatChannelListToClient >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_RequestHistoryResult:
      {
         *packetOut = SerializeIn< PacketChatHistoryResult >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_RequestHistorySinceLastLogin:
      {
         *packetOut = SerializeIn< PacketChatMissedHistoryRequest >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_RequestHistorySinceLastLoginResponse:
      {
         *packetOut = SerializeIn< PacketChatMissedHistoryResult >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannel:
      {
         *packetOut = SerializeIn< PacketChatCreateChatChannel >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatCreateChatChannelResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_DeleteChatChannel:
      {
         *packetOut = SerializeIn< PacketChatDeleteChatChannel >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_DeleteChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatDeleteChatChannelResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_InviteUserToChatChannel:
      {
         *packetOut = SerializeIn< PacketChatInviteUserToChatChannel >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_InviteUserToChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatInviteUserToChatChannelResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_AddUserToChatChannel:
      {
         *packetOut = SerializeIn< PacketChatAddUserToChatChannel >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_AddUserToChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatAddUserToChatChannelResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_RemoveUserFromChatChannel:
      {
         *packetOut = SerializeIn< PacketChatRemoveUserFromChatChannel >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatRemoveUserFromChatChannelResponse >( bufferIn, bufferOffset );
      }
      return true;
   
   case PacketChatToServer::ChatType_RequestChatters:
      {
         *packetOut = SerializeIn< PacketChatRequestChatters >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_RequestChattersResponse:
      {
         *packetOut = SerializeIn< PacketChatRequestChattersResponse >( bufferIn, bufferOffset );
      }
      return true;
   
   case PacketChatToServer::ChatType_EnableDisableFiltering:
      {
         *packetOut = SerializeIn< PacketChatEnableFiltering >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_EnableDisableFilteringResponse:
      {
         *packetOut = SerializeIn< PacketChatEnableFilteringResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketChatToServer::ChatType_ListAllMembersInChatChannel:
      {
         *packetOut = SerializeIn< PacketChatListAllMembersInChatChannel >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_ListAllMembersInChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatListAllMembersInChatChannelResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_AdminLoadAllChannels:
      {
         *packetOut = SerializeIn< PacketChatAdminLoadAllChannels >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_AdminLoadAllChannelsResponse:
      {
         *packetOut = SerializeIn< PacketChatAdminLoadAllChannelsResponse >( bufferIn, bufferOffset );
      }
      return true;
   
   case PacketChatToServer::ChatType_AdminRequestChatChannelList:
      {
         *packetOut = SerializeIn< PacketChatAdminRequestChatChannelList >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_AdminRequestChatChannelListResponse:
      {
         *packetOut = SerializeIn< PacketChatAdminRequestChatChannelListResponse >( bufferIn, bufferOffset );
      }
      return true;
   
   case PacketChatToServer::ChatType_AdminRequestUsersList:
      {
         *packetOut = SerializeIn< PacketChatAdminRequestUsersList >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_AdminRequestUsersListResponse:
      {
         *packetOut = SerializeIn< PacketChatAdminRequestUsersListResponse >( bufferIn, bufferOffset );
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------------------------------------------------

bool     PacketFactory::ParseUserInfo( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketUserInfo::InfoType_FriendsListRequest:
      {
         *packetOut = SerializeIn< PacketFriendsListRequest >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserInfo::InfoType_FriendsList:
      {
         *packetOut = SerializeIn< PacketFriendsList >( bufferIn, bufferOffset );
      }
      return true;

   case PacketUserInfo::InfoType_GroupsListRequest:
      {
         *packetOut = SerializeIn< PacketGroupsListRequest >( bufferIn, bufferOffset );
      }
      return true;

   case PacketUserInfo::InfoType_GroupsList:
      {
         *packetOut = SerializeIn< PacketGroupsList >( bufferIn, bufferOffset );
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseDbQuery( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketDbQuery::QueryType_Query:
      {
         *packetOut = SerializeIn< PacketDbQuery >( bufferIn, bufferOffset );
      }
      return true;
   case PacketDbQuery::QueryType_Result:
      {
         *packetOut = SerializeIn< PacketDbQueryResult >( bufferIn, bufferOffset );
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseGame( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketGameToServer::GamePacketType_LoginToServer:
      {
         *packetOut = SerializeIn< PacketGameToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_CreateGame:
      {
         *packetOut = SerializeIn< PacketCreateGame >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_CreateGameResponse:
      {
         *packetOut = SerializeIn< PacketCreateGameResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_DeleteGame:
      {
         *packetOut = SerializeIn< PacketDeleteGame >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_DeleteGameResponse:
      {
         *packetOut = SerializeIn< PacketDeleteGameResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_ForfeitGame:
      {
         *packetOut = SerializeIn< PacketForfeitGame >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_ForfeitGameResponse:
      {
         *packetOut = SerializeIn< PacketForfeitGameResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_QuitGame:
      {
         *packetOut = SerializeIn< PacketQuitGame >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_QuitGameResponse:
      {
         *packetOut = SerializeIn< PacketQuitGameResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_AddUser:
      {
         *packetOut = SerializeIn< PacketAddUserToGame >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_AddUserResponse:
      {
         *packetOut = SerializeIn< PacketAddUserToGameResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketGameToServer::GamePacketType_RemoveUser:
      {
         *packetOut = SerializeIn< PacketRemoveUserFromGame >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_RemoveUserResponse:
      {
         *packetOut = SerializeIn< PacketRemoveUserFromGameResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_AdvanceTurn:
      {
         *packetOut = SerializeIn< PacketGameAdvanceTurn >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_TurnWasAdvanced:
      {
         *packetOut = SerializeIn< PacketGameAdvanceTurnResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfGames:
      {
         *packetOut = SerializeIn< PacketRequestListOfGames >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfGamesResponse:
      {
         *packetOut = SerializeIn< PacketRequestListOfGamesResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfUsersInGame:
      {
         *packetOut = SerializeIn< PacketRequestListOfUsersInGame >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfUsersInGameResponse:
      {
         *packetOut = SerializeIn< PacketRequestListOfUsersInGameResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_ListOfGames:
      {
         *packetOut = SerializeIn< PacketListOfGames >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_GameIdentification:
      {
         *packetOut = SerializeIn< PacketGameIdentification >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_RawGameData:
      {
         *packetOut = SerializeIn< PacketGameplayRawData >( bufferIn, bufferOffset );
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseServerToServerWrapper( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper();
   *packetOut = wrapper;
   return wrapper->SerializeIn( bufferIn, bufferOffset );
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseServerInfo( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   PacketServerIdentifier* id = new PacketServerIdentifier();
   id->SerializeIn( bufferIn, bufferOffset );
   *packetOut = id;
   return true;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseGatewayWrapper( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   // I am left wondering whether or not to return the unwrapped packet or not -mkawick

   // we only have one type so no switch is needed.
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   *packetOut = wrapper;
   return wrapper->SerializeIn( bufferIn, bufferOffset );
}

//-----------------------------------------------------------------------------------------
