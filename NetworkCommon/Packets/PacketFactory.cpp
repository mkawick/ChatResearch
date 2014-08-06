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
         // put here just to avoid a function call for only one type.
         PacketErrorReport* error = new PacketErrorReport();
         error->SerializeIn( bufferIn, bufferOffset );
         *packetOut = error;
         return true;
      }
   case PacketType_Cheat:
      {
         return ParseCheat( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_Purchase:
      {
         return ParsePurchase( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_Analytics:
      {
         return ParseAnalytics( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_Tournament:
      {
         return ParseTournament( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_UserStats:
      {
         return ParseUserStats( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_Notification:
      {
         return ParseNotification( bufferIn, bufferOffset, &firstPassParse, packetOut );
      }
   case PacketType_Invitation:
      {
         return ParseInvitation( bufferIn, bufferOffset, &firstPassParse, packetOut );
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
      case BasePacket::BasePacket_RerouteRequest:
         {
            *packetOut = SerializeIn< PacketRerouteRequest >( bufferIn, bufferOffset );
         }
         return true;
      case BasePacket::BasePacket_RerouteRequestResponse:
         {
            *packetOut = SerializeIn< PacketRerouteRequestResponse >( bufferIn, bufferOffset );
         }
         return true;
      case BasePacket::BasePacket_QOS:
         {
            *packetOut = SerializeIn< Packet_QOS_ReportToClient >( bufferIn, bufferOffset );
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
      case PacketLogin::LoginType_LoginFromGateway:
         {
            *packetOut = SerializeIn< PacketLoginFromGateway >( bufferIn, bufferOffset );
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
      case PacketLogin::LoginType_EchoToServer:
         {
            *packetOut = SerializeIn< PacketLogin_EchoToServer >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_EchoToClient:
         {
            *packetOut = SerializeIn< PacketLogin_EchoToClient >( bufferIn, bufferOffset );
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
      case PacketLogin::LoginType_ThrottleUsersConnection:
         {
            *packetOut = SerializeIn< PacketLoginThrottlePackets >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_DebugThrottleUsersConnection:
         {
            *packetOut = SerializeIn< PacketLoginDebugThrottleUserPackets >( bufferIn, bufferOffset );
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
            *packetOut = SerializeIn< PacketListOfUserPurchasesRequest >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_AddPurchaseEntry:
         {
            *packetOut = SerializeIn< PacketAddPurchaseEntry >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_ListOfAggregatePurchases:
         {
            *packetOut = SerializeIn< PacketListOfUserAggregatePurchases >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_ListOfProductsS2S:
         {
            *packetOut = SerializeIn< PacketListOfUserProductsS2S >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_RequestUserProfile:
         {
            *packetOut = SerializeIn< PacketRequestUserProfile >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_RequestUserProfileResponse:
         {
            *packetOut = SerializeIn< PacketRequestUserProfileResponse >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_UpdateUserProfile:
         {
            *packetOut = SerializeIn< PacketUpdateUserProfile >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_UpdateUserProfileResponse:
         {
            *packetOut = SerializeIn< PacketUpdateUserProfileResponse >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_RequestListOfProducts:
         {
            *packetOut = SerializeIn< PacketRequestListOfProducts >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_RequestListOfProductsResponse:
         {
            *packetOut = SerializeIn< PacketRequestListOfProductsResponse >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_RequestOtherUserProfile:
         {
            *packetOut = SerializeIn< PacketRequestOtherUserProfile >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_RequestOtherUserProfileResponse:
         {
            *packetOut = SerializeIn< PacketRequestOtherUserProfileResponse >( bufferIn, bufferOffset );
         }
         return true;

      case PacketLogin::LoginType_UpdateSelfProfile:
         {
            *packetOut = SerializeIn< PacketUpdateSelfProfile >( bufferIn, bufferOffset );
         }
         return true;
      case PacketLogin::LoginType_UpdateSelfProfileResponse:
         {
            *packetOut = SerializeIn< PacketUpdateSelfProfileResponse >( bufferIn, bufferOffset );
         }
         return true;

      case PacketLogin::LoginType_UserUpdateProfile:
         {
            *packetOut = SerializeIn< PacketUserUpdateProfile >( bufferIn, bufferOffset );
         }
         return true;

      case PacketLogin::LoginType_UserListOfPurchasesWasUpdated:
         {
            *packetOut = SerializeIn< PacketListOfUserPurchasesUpdated >( bufferIn, bufferOffset );
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
   case PacketChatToServer::ChatType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketChat_EchoToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketChat_EchoToClient >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_UserChatStatusChange:
      {
         *packetOut = SerializeIn< PacketChatUserStatusChangeBase >( bufferIn, bufferOffset );
      }
      return true;

  /* case PacketChatToServer::ChatType_ChangeChatChannel:
      {
         *packetOut = SerializeIn< PacketChangeChatChannel >( bufferIn, bufferOffset );
      }
      return true;*/

   case PacketChatToServer::ChatType_RequestHistory:
      {
         *packetOut = SerializeIn< PacketChatHistoryRequest >( bufferIn, bufferOffset );
      }
      return true;

   /*case PacketChatToServer::ChatType_ChangeChatChannelToClient:
      {
         *packetOut = SerializeIn< PacketChangeChatChannelToClient >( bufferIn, bufferOffset );
      }
      return true;*/

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

   case PacketChatToServer::ChatType_DeleteChatChannelFromGameServer:
      {
         *packetOut = SerializeIn< PacketChatDeleteChatChannelFromGameServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_DeleteChatChannelFromGameServerResponse:
      {
         *packetOut = SerializeIn< PacketChatDeleteChatChannelFromGameServerResponse >( bufferIn, bufferOffset );
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

   case PacketChatToServer::ChatType_RenameChatChannel:
      {
         *packetOut = SerializeIn< PacketChatRenameChannel >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_RenameChatChannelResponse:
      {
         *packetOut = SerializeIn< PacketChatRenameChannelResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketChatToServer::ChatType_UpdateProfile:
      {
         *packetOut = SerializeIn< PacketChat_UserProfileChange >( bufferIn, bufferOffset );
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

   case PacketContact::ContactType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketContact_EchoToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketContact_EchoToClient >( bufferIn, bufferOffset );
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

   case PacketContact::ContactType_Search:
      {
         *packetOut = SerializeIn< PacketContact_SearchForUser >( bufferIn, bufferOffset );
      }
      return true;
   case PacketContact::ContactType_SearchResults:
      {
         *packetOut = SerializeIn< PacketContact_SearchForUserResult >( bufferIn, bufferOffset );
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

   case PacketContact::ContactType_SetNotation:
      {
         *packetOut = SerializeIn< PacketContact_SetNotationOnUser >( bufferIn, bufferOffset );
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
   case PacketAsset::AssetType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketAsset_EchoToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketAsset_EchoToClient >( bufferIn, bufferOffset );
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
   case PacketAsset::AssetType_GetListOfAssetCategories:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfAssetCategories >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssetCategoriesResponse:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfAssetCategoriesResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssets:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfAssets >( bufferIn, bufferOffset );
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssetsResponse:
      {
         *packetOut = SerializeIn< PacketAsset_GetListOfAssetsResponse >( bufferIn, bufferOffset );
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
   case PacketGameToServer::GamePacketType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketGame_EchoToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketGame_EchoToClient >( bufferIn, bufferOffset );
      }
      return true;
   case PacketGameToServer::GamePacketType_TestHook:
      {
         *packetOut = SerializeIn< PacketGame_TestHook >( bufferIn, bufferOffset );
      }
      return true;

   case PacketGameToServer::GamePacketType_Notification:
      {
         *packetOut = SerializeIn< PacketGame_Notification >( bufferIn, bufferOffset );
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

bool     PacketFactory::ParsePurchase( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Cheat
   {
   case PacketPurchase::PurchaseType_Base:
      {
         *packetOut = SerializeIn< PacketPurchase >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_TestNotification:
      {
         *packetOut = SerializeIn< PacketPurchase_TestNotification >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketPurchase_EchoToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketPurchase_EchoToClient >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_Buy:
      {
         *packetOut = SerializeIn< PacketPurchase_Buy >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_BuyResponse:
      {
         *packetOut = SerializeIn< PacketPurchase_BuyResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_RequestListOfSales:
      {
         *packetOut = SerializeIn< PacketPurchase_RequestListOfSales >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_RequestListOfSalesResponse:
      {
         *packetOut = SerializeIn< PacketPurchase_RequestListOfSalesResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_ValidatePurchaseReceipt:
      {
         *packetOut = SerializeIn< PacketPurchase_ValidatePurchaseReceipt >( bufferIn, bufferOffset );
      }
      return true;
   case PacketPurchase::PurchaseType_ValidatePurchaseReceiptResponse:
      {
         *packetOut = SerializeIn< PacketPurchase_ValidatePurchaseReceiptResponse >( bufferIn, bufferOffset );
      }
      return true;
   }
   
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseAnalytics( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   *packetOut = SerializeIn< PacketAnalytics >( bufferIn, bufferOffset );
   return true;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseTournament( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Tournament
   {
    case PacketTournament::TournamentType_Base:
      {
         *packetOut = SerializeIn< PacketTournament >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_TestNotification:
      {
         *packetOut = SerializeIn< PacketTournament_TestNotification >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_RequestListOfTournaments:
      {
         *packetOut = SerializeIn< PacketTournament_RequestListOfTournaments >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_RequestListOfTournamentsResponse:
      {
         *packetOut = SerializeIn< PacketTournament_RequestListOfTournamentsResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_UserRequestsEntryInTournament:
      {
         *packetOut = SerializeIn< PacketTournament_UserRequestsEntryInTournament >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_UserRequestsEntryInTournamentResponse:
      {
         *packetOut = SerializeIn< PacketTournament_UserRequestsEntryInTournamentResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntry:
      {
         *packetOut = SerializeIn< PacketTournament_PurchaseTournamentEntry >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryResponse:
      {
         *packetOut = SerializeIn< PacketTournament_PurchaseTournamentEntryResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryRefund:
      {
         *packetOut = SerializeIn< PacketTournament_PurchaseTournamentEntryRefund >( bufferIn, bufferOffset );
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryRefundResponse:
      {
         *packetOut = SerializeIn< PacketTournament_PurchaseTournamentEntryRefundResponse >( bufferIn, bufferOffset );
      }
      return true;
   default:
      assert( 0 );
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseUserStats( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Tournament
   {
    case PacketUserStats::UserStatsType_Base:
      {
         *packetOut = SerializeIn< PacketUserStats >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_TestUserStats:
      {
         *packetOut = SerializeIn< PacketUserStats_TestUserStats >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestListOfUserStats:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestListOfUserStats >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestListOfUserStatsResponse:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestListOfUserStatsResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_RecordUserStats:
      {
         *packetOut = SerializeIn< PacketUserStats_RecordUserStats >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_RecordUserStatsResponse:
      {
         *packetOut = SerializeIn< PacketUserStats_RecordUserStatsResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_ReportGameResult:
      {
         *packetOut = SerializeIn< PacketUserStats_ReportGameResult >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestGameFactionStats:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestGameFactionStats >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestGameProfile:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestGameProfile >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestUserProfileStats:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestUserProfileStats >( bufferIn, bufferOffset );
      }
      return true;
   case PacketUserStats::UserStatsType_RequestUserProfileStatsResponse:
      {
         *packetOut = SerializeIn< PacketUserStats_RequestUserProfileStatsResponse >( bufferIn, bufferOffset );
      }
      return true;

      
   
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseNotification( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType ) //PacketType_Cheat
   {
   case PacketNotification::NotificationType_Base:
      {
         *packetOut = SerializeIn< PacketNotification >( bufferIn, bufferOffset );
      }
      return true;
   case PacketNotification::NotificationType_TestNotification:
      {
         *packetOut = SerializeIn< PacketNotification_TestNotification >( bufferIn, bufferOffset );
      }
      return true;
   case PacketNotification::NotificationType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketNotification_EchoToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketNotification::NotificationType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketNotification_EchoToClient >( bufferIn, bufferOffset );
      }
      return true;
   case PacketNotification::NotificationType_RegisterDevice:
      {
         *packetOut = SerializeIn< PacketNotification_RegisterDevice >( bufferIn, bufferOffset );
      }
      return true;
   case PacketNotification::NotificationType_RegisterDeviceResponse:
      {
         *packetOut = SerializeIn< PacketNotification_RegisterDeviceResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketNotification::NotificationType_UpdateDevice:
      {
         *packetOut = SerializeIn< PacketNotification_UpdateDevice >( bufferIn, bufferOffset );
      }
      return true;
      
   case PacketNotification::NotificationType_SendNotification:
      {
         *packetOut = SerializeIn< PacketNotification_SendNotification >( bufferIn, bufferOffset );
      }
      return true;

   case PacketNotification::NotificationType_UpdateNotificationCount:
      {
         *packetOut = SerializeIn< PacketNotification_UpdateNotificationCount >( bufferIn, bufferOffset );
      }
      return true;
      
   case PacketNotification::NotificationType_RequestListOfDevices:
      {
         *packetOut = SerializeIn< PacketNotification_RequestListOfDevices >( bufferIn, bufferOffset );
      }
      return true;
   case PacketNotification::NotificationType_RequestListOfDevicesResponse:
      {
         *packetOut = SerializeIn< PacketNotification_RequestListOfDevicesResponse >( bufferIn, bufferOffset );
      }
      return true;

   case PacketNotification::NotificationType_RemoveDevice:
      {
         *packetOut = SerializeIn< PacketNotification_RemoveDevice >( bufferIn, bufferOffset );
      }
      return true;
   case PacketNotification::NotificationType_RemoveDeviceResponse:
      {
         *packetOut = SerializeIn< PacketNotification_RemoveDeviceResponse >( bufferIn, bufferOffset );
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::ParseInvitation( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut ) const
{
   switch( firstPassParse->packetSubType ) 
   {
   case PacketInvitation::InvitationType_Base:
      {
         *packetOut = SerializeIn< PacketInvitation >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_EchoToServer:
      {
         *packetOut = SerializeIn< PacketInvitation_EchoToServer >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_EchoToClient:
      {
         *packetOut = SerializeIn< PacketInvitation_EchoToClient >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_InviteUser:
      {
         *packetOut = SerializeIn< PacketInvitation_InviteUser >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_InviteUserResponse:
      {
         *packetOut = SerializeIn< PacketInvitation_InviteUserResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_CancelInvitation:
      {
         *packetOut = SerializeIn< PacketInvitation_CancelInvitation >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_InvitationWasCancelled:
      {
         *packetOut = SerializeIn< PacketInvitation_InvitationWasCancelled >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_RejectInvitation:
      {
         *packetOut = SerializeIn< PacketInvitation_RejectInvitation >( bufferIn, bufferOffset );
      }
      return true;

   case PacketInvitation::InvitationType_RejectInvitationResponse:
      {
         *packetOut = SerializeIn< PacketInvitation_RejectInvitationResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_AcceptInvitation:
      {
         *packetOut = SerializeIn< PacketInvitation_AcceptInvitation >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitations:
      {
         *packetOut = SerializeIn< PacketInvitation_GetListOfInvitations >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsResponse:
      {
         *packetOut = SerializeIn< PacketInvitation_GetListOfInvitationsResponse >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsForGroup:
      {
         *packetOut = SerializeIn< PacketInvitation_GetListOfInvitationsForGroup >( bufferIn, bufferOffset );
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsForGroupResponse:
      {
         *packetOut = SerializeIn< PacketInvitation_GetListOfInvitationsForGroupResponse >( bufferIn, bufferOffset );
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
   switch( firstPassParse->packetSubType ) //PacketType_Tournament
   {
    case PacketServerConnectionInfo::PacketServerIdentifier_TypicalInfo:
      {
         *packetOut = SerializeIn< PacketServerIdentifier >( bufferIn, bufferOffset );
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_ConnectionInfo:
      {
         *packetOut = SerializeIn< PacketServerConnectionInfo >( bufferIn, bufferOffset );
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_Disconnect:
      {
         *packetOut = SerializeIn< PacketServerDisconnect >( bufferIn, bufferOffset );
      }
      return true;
    }
   return false;
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
