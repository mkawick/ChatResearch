// PacketFactory.cpp

#include <algorithm>
#include <iostream>
#include <map>
#include <assert.h>

#include "../DataTypes.h"
#include "PacketFactory.h"

#include "BasePacket.h"
#include "AssetPacket.h"
#include "ChatPacket.h"
#include "CheatPacket.h"
#include "ContactPacket.h"
#include "DbPacket.h"
#include "GamePacket.h"
#include "ServerToServerPacket.h"

using namespace std;

PacketFactory::PacketFactory(){}

template< typename PacketType >
PacketType* SerializeIn( const U8* bufferIn, int& bufferOffset )
{
   PacketType* newPacket = new PacketType();
   newPacket->SerializeIn( bufferIn, bufferOffset );

   return newPacket;
}

//-----------------------------------------------------------------------------------------

bool	PacketFactory::Parse( const U8* bufferIn, int& bufferOffset, BasePacket** packetOut ) const
{
   *packetOut = NULL;// just to be sure that no one misuses this

   BasePacket firstPassParse;
   int offset = bufferOffset;
   firstPassParse.SerializeIn( bufferIn, offset );

   switch( firstPassParse.packetType )
   {
   case PacketType_Base:
      {
         return ParseBasePacket( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
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
   case PacketType_Contact:
      {
         return ParseContact( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_Asset:
      {
         return ParseAsset( bufferIn, bufferOffset, &firstPassParse, packetOut );
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
   case PacketType_Cheat:
      {
         return ParseCheat( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   }

   return false;
}// be sure to check the return value

//---------------------------------------------------------------------------------------------------------------------------------------

void     PacketFactory::CleanupPacket( BasePacket*& packet )
{
   if( packet )
   {
      if( packet->packetType == PacketType_GatewayWrapper )
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
         delete wrapper->pPacket;
      }
      else if( packet->packetType == PacketType_ServerToServerWrapper )
      {
         PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );
         delete wrapper->pPacket;
      }

      delete packet;

      packet = NULL;
   }
}

//---------------------------------------------------------------------------------------------------------------------------------------

bool     PacketFactory::SafeParse( const U8* bufferIn, int& bufferOffset, BasePacket& packetOut ) const
{
   int offset = bufferOffset;
   packetOut.SerializeIn( bufferIn, offset );
   return true;
}

//---------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------

bool  PacketFactory::ParseBasePacket( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType )
   {
      case BasePacket::BasePacket_Type:
         {
            *packetOut = SerializeIn< BasePacket >( bufferIn, bufferOffset );
         }
         return true;
      case BasePacket::BasePacket_Hello:
         {
            *packetOut = SerializeIn< PacketHello >( bufferIn, bufferOffset );
         }
         return true;
      case BasePacket::BasePacket_CommsHandshake:
         {
            *packetOut = SerializeIn< PacketCommsHandshake >( bufferIn, bufferOffset );
         }
         return true;
   }
   return false;
}

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
      case PacketLogin::LoginType_CreateAccount:
         {
            *packetOut = SerializeIn< PacketCreateAccount >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_CreateAccountResponse:
         {
            *packetOut = SerializeIn< PacketCreateAccountResponse >( bufferIn, bufferOffset );
         }
         return true;

      case PacketLogin::LoginType_RequestListOfPurchases:
         {
            *packetOut = SerializeIn< PacketRequestListOfUserPurchases >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_ListOfPurchases:
         {
            *packetOut = SerializeIn< PacketListOfUserPurchases >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_ListOfProductsS2S:
         {
            *packetOut = SerializeIn< PacketListOfUserProductsS2S >( bufferIn, bufferOffset );
         }
         return true;
     /* case PacketLogin::LoginType_ListOfPurchases_Cheat:
         {
            *packetOut = SerializeIn< PacketSubmitListOfUserPurchases_Cheat >( bufferIn, bufferOffset );
         }
         return true;*/
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
   case PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer:
      {
         *packetOut = SerializeIn< PacketChatUserAddedToChatChannelFromGameServer >( bufferIn, bufferOffset );
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
   case PacketChatToServer::ChatType_CreateChatChannelFromGameServer:
      {
         *packetOut = SerializeIn< PacketChatCreateChatChannelFromGameServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannelFromGameServerResponse:
      {
         *packetOut = SerializeIn< PacketChatCreateChatChannelFromGameServerResponse >( bufferIn, bufferOffset );
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
   case PacketChatToServer::ChatType_AddUserToChatChannelGameServer:
      {
         *packetOut = SerializeIn< PacketChatAddUserToChatChannelGameServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_AddUserToChatChannelGameServerResponse:
      {
         *packetOut = SerializeIn< PacketChatAddUserToChatChannelGameServerResponse >( bufferIn, bufferOffset );
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
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServer:
      {
         *packetOut = SerializeIn< PacketChatRemoveUserFromChatChannelGameServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServerResponse:
      {
         *packetOut = SerializeIn< PacketChatRemoveUserFromChatChannelGameServerResponse >( bufferIn, bufferOffset );
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

   case PacketUserInfo::InfoType_ChatChannelListRequest:
      {
         *packetOut = SerializeIn< PacketChatChannelListRequest >( bufferIn, bufferOffset );
      }
      return true;

   case PacketUserInfo::InfoType_ChatChannelList:
      {
         *packetOut = SerializeIn< PacketChatChannelList >( bufferIn, bufferOffset );
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

bool     PacketFactory::ParseContact( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketContact::ContactType_Base:
      {
         *packetOut = SerializeIn< PacketContact >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_TestNotification:
      {
         *packetOut = SerializeIn< PacketContact_TestNotification >( bufferIn, bufferOffset );
      }
      return true;

   case PacketContact::ContactType_GetListOfContacts:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfContacts >( bufferIn, bufferOffset );
      }
      return true;

   case PacketContact::ContactType_GetListOfContactsResponse:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfContactsResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitations:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfInvitations >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_GetListOfInvitationsResponse:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfInvitationsResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitationsSent:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfInvitationsSent >( bufferIn, bufferOffset );
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitationsSentResponse:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfInvitationsSentResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_InviteContact:
      {
         *packetOut = SerializeIn< PacketContact_InviteContact >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_InviteSentNotification:
      {
         *packetOut = SerializeIn< PacketContact_InviteSentNotification >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_InviteReceived:
      {
         *packetOut = SerializeIn< PacketContact_InviteReceivedNotification >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_RemoveContact:
      {
         *packetOut = SerializeIn< PacketContact_ContactRemove >( bufferIn, bufferOffset );
      }
      return true;
    case PacketContact::ContactType_RemoveInvitation:
      {
         *packetOut = SerializeIn< PacketContact_RemoveInvitation >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_AcceptInvite:
      {
         *packetOut = SerializeIn< PacketContact_AcceptInvite >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_InvitationAccepted:
      {
         *packetOut = SerializeIn< PacketContact_InvitationAccepted >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_DeclineInvitation:
      {
         *packetOut = SerializeIn< PacketContact_DeclineInvitation >( bufferIn, bufferOffset );
      }
      return true;

   case PacketContact::ContactType_BlockUser:
      {
         *packetOut = SerializeIn< PacketContact_InviteBlockUser >( bufferIn, bufferOffset );
      }
      return true;


   case PacketContact::ContactType_UserOnlineStatusChange:
      {
         *packetOut = SerializeIn< PacketContact_FriendOnlineStatusChange >( bufferIn, bufferOffset );
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseAsset( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketAsset::AssetType_Base:
      {
         *packetOut = SerializeIn< PacketAsset >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_TestNotification:
      {
         *packetOut = SerializeIn< PacketAsset_TestNotification >( bufferIn, bufferOffset );
      }
      return true;

   case PacketAsset::AssetType_GetListOfStaticAssets:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfStaticAssets >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_GetListOfStaticAssetsResponse:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfStaticAssetsResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_GetListOfDynamicAssets:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfDynamicAssets >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_GetListOfDynamicAssetsResponse:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfDynamicAssetsResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_RequestAsset:
      {
         *packetOut = SerializeIn< PacketAsset_RequestAsset >( bufferIn, bufferOffset );
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
   case PacketGameToServer::GamePacketType_RequestUserWinLoss:
      {
         *packetOut = SerializeIn< PacketRequestUserWinLoss >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
      {
         *packetOut = SerializeIn< PacketRequestUserWinLossResponse >( bufferIn, bufferOffset );
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

bool     PacketFactory::ParseCheat( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Cheat
   {
   case PacketCheat::Cheat_Basic:
      {
         *packetOut = SerializeIn< PacketCheat >( bufferIn, bufferOffset );
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
