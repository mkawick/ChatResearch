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
#include "InvitationPacket.h"
#include "LoginPacket.h"
#include "NotificationPacket.h"
#include "PurchasePacket.h"
#include "ServerToServerPacket.h"
#include "AnalyticsPacket.h"
#include "TournamentPacket.h"
#include "UserStatsPacket.h"

using namespace std;

PacketFactory::PacketFactory(){}

template< typename PacketType >
PacketType* SerializeIn( const U8* bufferIn, int& bufferOffset, int networkMinorVersion )
{
   PacketType* newPacket = new PacketType();
   newPacket->SerializeIn( bufferIn, bufferOffset, networkMinorVersion );

   return newPacket;
}

//-----------------------------------------------------------------------------------------

bool	PacketFactory::Parse( const U8* bufferIn, int& bufferOffset, BasePacket** packetOut, int networkMinorVersion ) const
{
   *packetOut = NULL;// just to be sure that no one misuses this

   BasePacket firstPassParse;
   int offset = bufferOffset;
   firstPassParse.SerializeIn( bufferIn, offset, networkMinorVersion );

   switch( firstPassParse.packetType )
   {
   case PacketType_Base:
      {
         return ParseBasePacket( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Login:
      {
         return ParseLogin( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Chat:
      {
         return ParseChat( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_UserInfo:
      {
         return ParseUserInfo( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Contact:
      {
         return ParseContact( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Asset:
      {
         return ParseAsset( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_DbQuery:
      {   
         return ParseDbQuery( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_ServerToServerWrapper:
      {   
         return ParseServerToServerWrapper( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_ServerInformation:
      {
         return ParseServerInfo( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_GatewayWrapper:
      {   
         return ParseGatewayWrapper( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Gameplay:
      {
         return ParseGame( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_ErrorReport:
      {   
         // put here just to avoid a function call for only one type.
         PacketErrorReport* error = new PacketErrorReport();
         error->SerializeIn( bufferIn, bufferOffset, networkMinorVersion );
         *packetOut = error;
         return true;
      }
   case PacketType_Cheat:
      {
         return ParseCheat( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Purchase:
      {
         return ParsePurchase( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Analytics:
      {
         return ParseAnalytics( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Tournament:
      {
         return ParseTournament( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_UserStats:
      {
         return ParseUserStats( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Notification:
      {
         return ParseNotification( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
      }
   case PacketType_Invitation:
      {
         return ParseInvitation( bufferIn, bufferOffset, &firstPassParse, packetOut, networkMinorVersion );
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

bool     PacketFactory::SafeParse( const U8* bufferIn, int& bufferOffset, BasePacket& packetOut, int networkMinorVersion ) const
{
   int offset = bufferOffset;
   packetOut.SerializeIn( bufferIn, offset, networkMinorVersion );
   return true;
}

//---------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------

bool  PacketFactory::ParseBasePacket( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType )
   {
      case BasePacket::BasePacket_Type:
         {
            *packetOut = SerializeIn< BasePacket >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case BasePacket::BasePacket_Hello:
         {
            *packetOut = SerializeIn< PacketHello >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case BasePacket::BasePacket_CommsHandshake:
         {
            *packetOut = SerializeIn< PacketCommsHandshake >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case BasePacket::BasePacket_RerouteRequest:
         {
            *packetOut = SerializeIn< PacketRerouteRequest >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case BasePacket::BasePacket_RerouteRequestResponse:
         {
            *packetOut = SerializeIn< PacketRerouteRequestResponse >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case BasePacket::BasePacket_QOS:
         {
            *packetOut = SerializeIn< Packet_QOS_ReportToClient >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case BasePacket::BasePacket_TestOnly:
         {
            *packetOut = SerializeIn< PacketBase_TestOnly >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
   }
   return false;
}

//---------------------------------------------------------------------------------------------------------------------------------------
bool  PacketFactory::ParseLogin( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType )
   {
      case PacketLogin::LoginType_Login:
         {
            *packetOut = SerializeIn< PacketLogin >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_LoginFromGateway:
         {
            *packetOut = SerializeIn< PacketLoginFromGateway >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_Logout:
         {
            *packetOut = SerializeIn< PacketLogout >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_PacketLogoutToClient:
         {
            *packetOut = SerializeIn< PacketLogoutToClient >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_EchoToServer:
         {
            *packetOut = SerializeIn< PacketLogin_EchoToServer >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_EchoToClient:
         {
            *packetOut = SerializeIn< PacketLogin_EchoToClient >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_InformClientOfLoginStatus:
         {
            *packetOut = SerializeIn< PacketLoginToClient >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_InformGatewayOfLoginStatus:
         {
            *packetOut = SerializeIn< PacketLoginToGateway >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogin:
         {
            *packetOut = SerializeIn< PacketPrepareForUserLogin >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogout:
         {
            *packetOut = SerializeIn< PacketPrepareForUserLogout >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_ThrottleUsersConnection:
         {
            *packetOut = SerializeIn< PacketLoginThrottlePackets >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_DebugThrottleUsersConnection:
         {
            *packetOut = SerializeIn< PacketLoginDebugThrottleUserPackets >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_CreateAccount:
         {
            *packetOut = SerializeIn< PacketCreateAccount >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_CreateAccountResponse:
         {
            *packetOut = SerializeIn< PacketCreateAccountResponse >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_RequestListOfPurchases:
         {
            *packetOut = SerializeIn< PacketListOfUserPurchasesRequest >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_AddPurchaseEntry:
         {
            *packetOut = SerializeIn< PacketAddPurchaseEntry >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_ListOfAggregatePurchases:
         {
            *packetOut = SerializeIn< PacketListOfUserAggregatePurchases >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_ListOfProductsS2S:
         {
            *packetOut = SerializeIn< PacketListOfUserProductsS2S >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_RequestUserProfile:
         {
            *packetOut = SerializeIn< PacketRequestUserProfile >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_RequestUserProfileResponse:
         {
            *packetOut = SerializeIn< PacketRequestUserProfileResponse >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_UpdateUserProfile:
         {
            *packetOut = SerializeIn< PacketUpdateUserProfile >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_UpdateUserProfileResponse:
         {
            *packetOut = SerializeIn< PacketUpdateUserProfileResponse >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_RequestListOfProducts:
         {
            *packetOut = SerializeIn< PacketRequestListOfProducts >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_RequestListOfProductsResponse:
         {
            *packetOut = SerializeIn< PacketRequestListOfProductsResponse >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_RequestOtherUserProfile:
         {
            *packetOut = SerializeIn< PacketRequestOtherUserProfile >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_RequestOtherUserProfileResponse:
         {
            *packetOut = SerializeIn< PacketRequestOtherUserProfileResponse >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;

      case PacketLogin::LoginType_UpdateSelfProfile:
         {
            *packetOut = SerializeIn< PacketUpdateSelfProfile >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_UpdateSelfProfileResponse:
         {
            *packetOut = SerializeIn< PacketUpdateSelfProfileResponse >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;

      case PacketLogin::LoginType_UserUpdateProfile:
         {
            *packetOut = SerializeIn< PacketUserUpdateProfile >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;

      case PacketLogin::LoginType_UserListOfPurchasesWasUpdated:
         {
            *packetOut = SerializeIn< PacketListOfUserPurchasesUpdated >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;

      case PacketLogin::LoginType_LogoutAllUsers:
         {
            *packetOut = SerializeIn< PacketLogin_LogoutAllUsers >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
      case PacketLogin::LoginType_ListOfMissingFeatures:
         {
            *packetOut = SerializeIn< PacketLogin_ListOfMissingFeatures >( bufferIn, bufferOffset, networkMinorVersion );
         }
         return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool  PacketFactory::ParseChat( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketChatToServer::ChatType_ChatToServer:
      {
         *packetOut = SerializeIn< PacketChatToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_ChatToClient:
      {
         *packetOut = SerializeIn< PacketChatToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketChat_EchoToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketChat_EchoToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_UserChatStatusChange:
      {
         *packetOut = SerializeIn< PacketChatUserStatusChangeBase >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

  /* case PacketChatToServer::ChatType_ChangeChatChannel:
      {
         *packetOut = SerializeIn< PacketChangeChatChannel >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;*/

   case PacketChatToServer::ChatType_RequestHistory:
      {
         *packetOut = SerializeIn< PacketChatHistoryRequest >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   /*case PacketChatToServer::ChatType_ChangeChatChannelToClient:
      {
         *packetOut = SerializeIn< PacketChangeChatChannelToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;*/

   case PacketChatToServer::ChatType_SendListOfChannelsToClient:
      {
         *packetOut = SerializeIn< PacketChatChannelListToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer:
      {
         *packetOut = SerializeIn< PacketChatUserAddedToChatChannelFromGameServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_RequestHistoryResult:
      {
         *packetOut = SerializeIn< PacketChatHistoryResult >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_RequestHistorySinceLastLogin:
      {
         *packetOut = SerializeIn< PacketChatMissedHistoryRequest >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_RequestHistorySinceLastLoginResponse:
      {
         *packetOut = SerializeIn< PacketChatMissedHistoryResult >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannel:
      {
         *packetOut = SerializeIn< PacketChatCreateChatChannel >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatCreateChatChannelResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannelFromGameServer:
      {
         *packetOut = SerializeIn< PacketChatCreateChatChannelFromGameServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannelFromGameServerResponse:
      {
         *packetOut = SerializeIn< PacketChatCreateChatChannelFromGameServerResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketChatToServer::ChatType_DeleteChatChannel:
      {
         *packetOut = SerializeIn< PacketChatDeleteChatChannel >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_DeleteChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatDeleteChatChannelResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketChatToServer::ChatType_DeleteChatChannelFromGameServer:
      {
         *packetOut = SerializeIn< PacketChatDeleteChatChannelFromGameServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_DeleteChatChannelFromGameServerResponse:
      {
         *packetOut = SerializeIn< PacketChatDeleteChatChannelFromGameServerResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketChatToServer::ChatType_InviteUserToChatChannel:
      {
         *packetOut = SerializeIn< PacketChatInviteUserToChatChannel >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_InviteUserToChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatInviteUserToChatChannelResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketChatToServer::ChatType_AddUserToChatChannel:
      {
         *packetOut = SerializeIn< PacketChatAddUserToChatChannel >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_AddUserToChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatAddUserToChatChannelResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_AddUserToChatChannelGameServer:
      {
         *packetOut = SerializeIn< PacketChatAddUserToChatChannelGameServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_AddUserToChatChannelGameServerResponse:
      {
         *packetOut = SerializeIn< PacketChatAddUserToChatChannelGameServerResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;


   case PacketChatToServer::ChatType_RemoveUserFromChatChannel:
      {
         *packetOut = SerializeIn< PacketChatRemoveUserFromChatChannel >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatRemoveUserFromChatChannelResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServer:
      {
         *packetOut = SerializeIn< PacketChatRemoveUserFromChatChannelGameServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServerResponse:
      {
         *packetOut = SerializeIn< PacketChatRemoveUserFromChatChannelGameServerResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   
   case PacketChatToServer::ChatType_RequestChatters:
      {
         *packetOut = SerializeIn< PacketChatRequestChatters >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_RequestChattersResponse:
      {
         *packetOut = SerializeIn< PacketChatRequestChattersResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   
   case PacketChatToServer::ChatType_EnableDisableFiltering:
      {
         *packetOut = SerializeIn< PacketChatEnableFiltering >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_EnableDisableFilteringResponse:
      {
         *packetOut = SerializeIn< PacketChatEnableFilteringResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketChatToServer::ChatType_ListAllMembersInChatChannel:
      {
         *packetOut = SerializeIn< PacketChatListAllMembersInChatChannel >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_ListAllMembersInChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatListAllMembersInChatChannelResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_AdminLoadAllChannels:
      {
         *packetOut = SerializeIn< PacketChatAdminLoadAllChannels >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_AdminLoadAllChannelsResponse:
      {
         *packetOut = SerializeIn< PacketChatAdminLoadAllChannelsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   
   case PacketChatToServer::ChatType_AdminRequestChatChannelList:
      {
         *packetOut = SerializeIn< PacketChatAdminRequestChatChannelList >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_AdminRequestChatChannelListResponse:
      {
         *packetOut = SerializeIn< PacketChatAdminRequestChatChannelListResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   
   case PacketChatToServer::ChatType_AdminRequestUsersList:
      {
         *packetOut = SerializeIn< PacketChatAdminRequestUsersList >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_AdminRequestUsersListResponse:
      {
         *packetOut = SerializeIn< PacketChatAdminRequestUsersListResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketChatToServer::ChatType_RenameChatChannel:
      {
         *packetOut = SerializeIn< PacketChatRenameChannel >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_RenameChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatRenameChannelResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_UpdateProfile:
      {
         *packetOut = SerializeIn< PacketChat_UserProfileChange >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_MarkChannelHistoryAsRead:
      {
         *packetOut = SerializeIn< PacketChat_MarkChannelHistoryAsRead >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_MarkP2PHistoryAsRead:
      {
         *packetOut = SerializeIn< PacketChat_MarkP2PHistoryAsRead >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_UserChatHistory:
      {
         *packetOut = SerializeIn< PacketChat_UserChatHistory >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketChatToServer::ChatType_ChannelChatHistory:
      {
         *packetOut = SerializeIn< PacketChat_ChannelChatHistory >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------------------------------------------------

bool     PacketFactory::ParseUserInfo( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketUserInfo::InfoType_FriendsListRequest:
      {
         *packetOut = SerializeIn< PacketFriendsListRequest >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserInfo::InfoType_FriendsList:
      {
         *packetOut = SerializeIn< PacketFriendsList >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketUserInfo::InfoType_ChatChannelListRequest:
      {
         *packetOut = SerializeIn< PacketChatChannelListRequest >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketUserInfo::InfoType_ChatChannelList:
      {
         *packetOut = SerializeIn< PacketChatChannelList >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketUserInfo::InfoType_GroupsListRequest:
      {
         *packetOut = SerializeIn< PacketGroupsListRequest >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketUserInfo::InfoType_GroupsList:
      {
         *packetOut = SerializeIn< PacketGroupsList >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseContact( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketContact::ContactType_Base:
      {
         *packetOut = SerializeIn< PacketContact >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_TestNotification:
      {
         *packetOut = SerializeIn< PacketContact_TestNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketContact::ContactType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketContact_EchoToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketContact_EchoToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketContact::ContactType_GetListOfContacts:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfContacts >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketContact::ContactType_GetListOfContactsResponse:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfContactsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitations:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfInvitations >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_GetListOfInvitationsResponse:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfInvitationsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitationsSent:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfInvitationsSent >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitationsSentResponse:
      {
         *packetOut = SerializeIn< PacketContact_GetListOfInvitationsSentResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_InviteContact:
      {
         *packetOut = SerializeIn< PacketContact_InviteContact >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_InviteSentNotification:
      {
         *packetOut = SerializeIn< PacketContact_InviteSentNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_InviteReceived:
      {
         *packetOut = SerializeIn< PacketContact_InviteReceivedNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_RemoveContact:
      {
         *packetOut = SerializeIn< PacketContact_ContactRemove >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
    case PacketContact::ContactType_RemoveInvitation:
      {
         *packetOut = SerializeIn< PacketContact_RemoveInvitation >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_AcceptInvite:
      {
         *packetOut = SerializeIn< PacketContact_AcceptInvite >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_InvitationAccepted:
      {
         *packetOut = SerializeIn< PacketContact_InvitationAccepted >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_DeclineInvitation:
      {
         *packetOut = SerializeIn< PacketContact_DeclineInvitation >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketContact::ContactType_Search:
      {
         *packetOut = SerializeIn< PacketContact_SearchForUser >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_SearchResults:
      {
         *packetOut = SerializeIn< PacketContact_SearchForUserResult >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketContact::ContactType_BlockUser:
      {
         *packetOut = SerializeIn< PacketContact_InviteBlockUser >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;


   case PacketContact::ContactType_UserOnlineStatusChange:
      {
         *packetOut = SerializeIn< PacketContact_FriendOnlineStatusChange >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketContact::ContactType_SetNotation:
      {
         *packetOut = SerializeIn< PacketContact_SetNotationOnUser >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseAsset( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketAsset::AssetType_Base:
      {
         *packetOut = SerializeIn< PacketAsset >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_TestNotification:
      {
         *packetOut = SerializeIn< PacketAsset_TestNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketAsset_EchoToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketAsset_EchoToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_GetListOfStaticAssets:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfStaticAssets >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_GetListOfStaticAssetsResponse:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfStaticAssetsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_GetListOfDynamicAssets:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfDynamicAssets >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_GetListOfDynamicAssetsResponse:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfDynamicAssetsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssetCategories:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfAssetCategories >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssetCategoriesResponse:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfAssetCategoriesResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssets:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfAssets >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssetsResponse:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfAssetsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   
   case PacketAsset::AssetType_RequestAsset:
      {
         *packetOut = SerializeIn< PacketAsset_RequestAsset >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseDbQuery( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketDbQuery::QueryType_Query:
      {
         *packetOut = SerializeIn< PacketDbQuery >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketDbQuery::QueryType_Result:
      {
         *packetOut = SerializeIn< PacketDbQueryResult >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseGame( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType )
   {
   case PacketGameToServer::GamePacketType_LoginToServer:
      {
         *packetOut = SerializeIn< PacketGameToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_CreateGame:
      {
         *packetOut = SerializeIn< PacketCreateGame >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_CreateGameResponse:
      {
         *packetOut = SerializeIn< PacketCreateGameResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_DeleteGame:
      {
         *packetOut = SerializeIn< PacketDeleteGame >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_DeleteGameResponse:
      {
         *packetOut = SerializeIn< PacketDeleteGameResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_ForfeitGame:
      {
         *packetOut = SerializeIn< PacketForfeitGame >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_ForfeitGameResponse:
      {
         *packetOut = SerializeIn< PacketForfeitGameResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_QuitGame:
      {
         *packetOut = SerializeIn< PacketQuitGame >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_QuitGameResponse:
      {
         *packetOut = SerializeIn< PacketQuitGameResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_AddUser:
      {
         *packetOut = SerializeIn< PacketAddUserToGame >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_AddUserResponse:
      {
         *packetOut = SerializeIn< PacketAddUserToGameResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketGameToServer::GamePacketType_RemoveUser:
      {
         *packetOut = SerializeIn< PacketRemoveUserFromGame >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_RemoveUserResponse:
      {
         *packetOut = SerializeIn< PacketRemoveUserFromGameResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_AdvanceTurn:
      {
         *packetOut = SerializeIn< PacketGameAdvanceTurn >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_TurnWasAdvanced:
      {
         *packetOut = SerializeIn< PacketGameAdvanceTurnResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfGames:
      {
         *packetOut = SerializeIn< PacketRequestListOfGames >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfGamesResponse:
      {
         *packetOut = SerializeIn< PacketRequestListOfGamesResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfUsersInGame:
      {
         *packetOut = SerializeIn< PacketRequestListOfUsersInGame >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfUsersInGameResponse:
      {
         *packetOut = SerializeIn< PacketRequestListOfUsersInGameResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestUserWinLoss:
      {
         *packetOut = SerializeIn< PacketRequestUserWinLoss >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
      {
         *packetOut = SerializeIn< PacketRequestUserWinLossResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_ListOfGames:
      {
         *packetOut = SerializeIn< PacketListOfGames >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_GameIdentification:
      {
         *packetOut = SerializeIn< PacketGameIdentification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_RawGameData:
      {
         *packetOut = SerializeIn< PacketGameplayRawData >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketGame_EchoToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketGame_EchoToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_TestHook:
      {
         *packetOut = SerializeIn< PacketGame_TestHook >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketGameToServer::GamePacketType_Notification:
      {
         *packetOut = SerializeIn< PacketGame_Notification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketGameToServer::GamePacketType_ServiceOutage:
      {
         *packetOut = SerializeIn< ClientSide_ServerOutageSchedule >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseCheat( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Cheat
   {
   case PacketCheat::Cheat_Basic:
      {
         *packetOut = SerializeIn< PacketCheat >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParsePurchase( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Cheat
   {
   case PacketPurchase::PurchaseType_Base:
      {
         *packetOut = SerializeIn< PacketPurchase >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_TestNotification:
      {
         *packetOut = SerializeIn< PacketPurchase_TestNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketPurchase_EchoToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketPurchase_EchoToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_Buy:
      {
         *packetOut = SerializeIn< PacketPurchase_Buy >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_BuyResponse:
      {
         *packetOut = SerializeIn< PacketPurchase_BuyResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_RequestListOfSales:
      {
         *packetOut = SerializeIn< PacketPurchase_RequestListOfSales >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_RequestListOfSalesResponse:
      {
         *packetOut = SerializeIn< PacketPurchase_RequestListOfSalesResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_ValidatePurchaseReceipt:
      {
         *packetOut = SerializeIn< PacketPurchase_ValidatePurchaseReceipt >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketPurchase::PurchaseType_ValidatePurchaseReceiptResponse:
      {
         *packetOut = SerializeIn< PacketPurchase_ValidatePurchaseReceiptResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }
   
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseAnalytics( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   *packetOut = SerializeIn< PacketAnalytics >( bufferIn, bufferOffset, networkMinorVersion );
   return true;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseTournament( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Tournament
   {
    case PacketTournament::TournamentType_Base:
      {
         *packetOut = SerializeIn< PacketTournament >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_TestNotification:
      {
         *packetOut = SerializeIn< PacketTournament_TestNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_RequestListOfTournaments:
      {
         *packetOut = SerializeIn< PacketTournament_RequestListOfTournaments >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_RequestListOfTournamentsResponse:
      {
         *packetOut = SerializeIn< PacketTournament_RequestListOfTournamentsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_UserRequestsEntryInTournament:
      {
         *packetOut = SerializeIn< PacketTournament_UserRequestsEntryInTournament >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_UserRequestsEntryInTournamentResponse:
      {
         *packetOut = SerializeIn< PacketTournament_UserRequestsEntryInTournamentResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntry:
      {
         *packetOut = SerializeIn< PacketTournament_PurchaseTournamentEntry >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryResponse:
      {
         *packetOut = SerializeIn< PacketTournament_PurchaseTournamentEntryResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryRefund:
      {
         *packetOut = SerializeIn< PacketTournament_PurchaseTournamentEntryRefund >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryRefundResponse:
      {
         *packetOut = SerializeIn< PacketTournament_PurchaseTournamentEntryRefundResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   default:
      assert( 0 );
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseUserStats( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Tournament
   {
    case PacketUserStats::UserStatsType_Base:
      {
         *packetOut = SerializeIn< PacketUserStats >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_TestUserStats:
      {
         *packetOut = SerializeIn< PacketUserStats_TestUserStats >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestListOfUserStats:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestListOfUserStats >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestListOfUserStatsResponse:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestListOfUserStatsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_RecordUserStats:
      {
         *packetOut = SerializeIn< PacketUserStats_RecordUserStats >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_RecordUserStatsResponse:
      {
         *packetOut = SerializeIn< PacketUserStats_RecordUserStatsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_ReportGameResult:
      {
         *packetOut = SerializeIn< PacketUserStats_ReportGameResult >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_ReportUserForfeit:
      {
         *packetOut = SerializeIn< PacketUserStats_ReportUserForfeit >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestGameFactionStats:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestGameFactionStats >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestGameProfile:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestGameProfile >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestUserProfileStats:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestUserProfileStats >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestUserProfileStatsResponse:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestUserProfileStatsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

      
   
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseNotification( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Cheat
   {
   case PacketNotification::NotificationType_Base:
      {
         *packetOut = SerializeIn< PacketNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketNotification::NotificationType_TestNotification:
      {
         *packetOut = SerializeIn< PacketNotification_TestNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketNotification::NotificationType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketNotification_EchoToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketNotification::NotificationType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketNotification_EchoToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketNotification::NotificationType_RegisterDevice:
      {
         *packetOut = SerializeIn< PacketNotification_RegisterDevice >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketNotification::NotificationType_RegisterDeviceResponse:
      {
         *packetOut = SerializeIn< PacketNotification_RegisterDeviceResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketNotification::NotificationType_UpdateDevice:
      {
         *packetOut = SerializeIn< PacketNotification_UpdateDevice >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
      
   case PacketNotification::NotificationType_SendNotification:
      {
         *packetOut = SerializeIn< PacketNotification_SendNotification >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketNotification::NotificationType_UpdateNotificationCount:
      {
         *packetOut = SerializeIn< PacketNotification_UpdateNotificationCount >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
      
   case PacketNotification::NotificationType_RequestListOfDevices:
      {
         *packetOut = SerializeIn< PacketNotification_RequestListOfDevices >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketNotification::NotificationType_RequestListOfDevicesResponse:
      {
         *packetOut = SerializeIn< PacketNotification_RequestListOfDevicesResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketNotification::NotificationType_RemoveDevice:
      {
         *packetOut = SerializeIn< PacketNotification_RemoveDevice >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketNotification::NotificationType_RemoveDeviceResponse:
      {
         *packetOut = SerializeIn< PacketNotification_RemoveDeviceResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseInvitation( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType ) 
   {
   case PacketInvitation::InvitationType_Base:
      {
         *packetOut = SerializeIn< PacketInvitation >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketInvitation_EchoToServer >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketInvitation_EchoToClient >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_InviteUser:
      {
         *packetOut = SerializeIn< PacketInvitation_InviteUser >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_InviteUserResponse:
      {
         *packetOut = SerializeIn< PacketInvitation_InviteUserResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_CancelInvitation:
      {
         *packetOut = SerializeIn< PacketInvitation_CancelInvitation >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_InvitationWasCancelled:
      {
         *packetOut = SerializeIn< PacketInvitation_InvitationWasCancelled >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_RejectInvitation:
      {
         *packetOut = SerializeIn< PacketInvitation_RejectInvitation >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;

   case PacketInvitation::InvitationType_RejectInvitationResponse:
      {
         *packetOut = SerializeIn< PacketInvitation_RejectInvitationResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_AcceptInvitation:
      {
         *packetOut = SerializeIn< PacketInvitation_AcceptInvitation >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitations:
      {
         *packetOut = SerializeIn< PacketInvitation_GetListOfInvitations >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsResponse:
      {
         *packetOut = SerializeIn< PacketInvitation_GetListOfInvitationsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsForGroup:
      {
         *packetOut = SerializeIn< PacketInvitation_GetListOfInvitationsForGroup >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsForGroupResponse:
      {
         *packetOut = SerializeIn< PacketInvitation_GetListOfInvitationsForGroupResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
    }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseServerToServerWrapper( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper();
   *packetOut = wrapper;
   return wrapper->SerializeIn( bufferIn, bufferOffset, networkMinorVersion );
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseServerInfo( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Tournament
   {
    case PacketServerConnectionInfo::PacketServerIdentifier_TypicalInfo:
      {
         *packetOut = SerializeIn< PacketServerIdentifier >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_ConnectionInfo:
      {
         *packetOut = SerializeIn< PacketServerConnectionInfo >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_Disconnect:
      {
         *packetOut = SerializeIn< PacketServerDisconnect >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_GatewayRequestLB_ConnectionIds:
      {
         *packetOut = SerializeIn< PacketServerToServer_GatewayRequestLB_ConnectionIds >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_GatewayRequestLB_ConnectionIdsResponse:
      {
         *packetOut = SerializeIn< PacketServerToServer_GatewayRequestLB_ConnectionIdsResponse >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_ServerOutageSchedule:
      {
         *packetOut = SerializeIn< PacketServerConnectionInfo_ServerOutageSchedule >( bufferIn, bufferOffset, networkMinorVersion );
      }
      return true;
    }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseGatewayWrapper( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const
{
   // I am left wondering whether or not to return the unwrapped packet or not -mkawick

   // we only have one type so no switch is needed.
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   *packetOut = wrapper;
   return wrapper->SerializeIn( bufferIn, bufferOffset, networkMinorVersion );
}

//-----------------------------------------------------------------------------------------
