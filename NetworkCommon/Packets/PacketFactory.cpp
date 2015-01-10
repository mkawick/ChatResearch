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

template< typename PacketType >
PacketType* CreatePacket()
{
   return new PacketType();
}

//-----------------------------------------------------------------------------------------

bool	PacketFactory::Parse( const U8* bufferIn, int& bufferOffset, BasePacket** packetOut, int networkMinorVersion ) const
{
   *packetOut = NULL;// just to be sure that no one misuses this

   BasePacket firstPassParse;
   int offset = bufferOffset;
   firstPassParse.SerializeIn( bufferIn, offset, networkMinorVersion );
   bool success = Create( firstPassParse.packetType, firstPassParse.packetSubType, packetOut );

 /*  switch( firstPassParse.packetType )
   {
   case PacketType_Base:
      {
         success = ParseBasePacket( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Login:
      {
         success = ParseLogin( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Chat:
      {
         success = ParseChat( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_UserInfo:
      {
         success = ParseUserInfo( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Contact:
      {
         success = ParseContact( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Asset:
      {
         success = ParseAsset( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_DbQuery:
      {   
         success = ParseDbQuery( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_ServerToServerWrapper:
      {   
         success = ParseServerToServerWrapper( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_ServerInformation:
      {
         success = ParseServerInfo( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_GatewayWrapper:
      {   
         success = ParseGatewayWrapper( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Gameplay:
      {
         success = ParseGame( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_ErrorReport:
      {   
         // put here just to avoid a function call for only one type.
         PacketErrorReport* error = new PacketErrorReport();
         //error->SerializeIn( bufferIn, bufferOffset, networkMinorVersion );
         *packetOut = error;
         success = true;
      }
   case PacketType_Cheat:
      {
         success = ParseCheat( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Purchase:
      {
         success = ParsePurchase( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Analytics:
      {
         success = ParseAnalytics( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Tournament:
      {
         success = ParseTournament( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_UserStats:
      {
         success = ParseUserStats( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Notification:
      {
         success = ParseNotification( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   case PacketType_Invitation:
      {
         success = ParseInvitation( bufferIn, bufferOffset, subType, packetOut, networkMinorVersion );
      }
   }*/

   if( success && *packetOut )
   {
      (*packetOut)->SerializeIn( bufferIn, bufferOffset, networkMinorVersion );
   } 

   return success;
}// be sure to check the return value

bool     PacketFactory::Create( int packetType, int packetSubType, BasePacket** packetOut ) const
{
   bool success = false;
   switch( packetType )
   {
   case PacketType_Base:
      {
         success = CreateBasePacket( packetSubType, packetOut );
      }
      break;
   case PacketType_Login:
      {
         success = CreateLogin( packetSubType, packetOut );
      }
      break;
   case PacketType_Chat:
      {
         success = CreateChat( packetSubType, packetOut );
      }
      break;
   case PacketType_UserInfo:
      {
         success = CreateUserInfo( packetSubType, packetOut );
      }
      break;
   case PacketType_Contact:
      {
         success = CreateContact( packetSubType, packetOut );
      }
      break;
   case PacketType_Asset:
      {
         success = CreateAsset( packetSubType, packetOut );
      }
      break;
   case PacketType_DbQuery:
      {   
         success = CreateDbQuery( packetSubType, packetOut );
      }
      break;
   case PacketType_ServerToServerWrapper:
      {   
         success = CreateServerToServerWrapper( packetSubType, packetOut );
      }
      break;
   case PacketType_ServerInformation:
      {
         success = CreateServerInfo( packetSubType, packetOut );
      }
      break;
   case PacketType_GatewayWrapper:
      {   
         success = CreateGatewayWrapper( packetSubType, packetOut );
      }
      break;
   case PacketType_Gameplay:
      {
         success = CreateGame( packetSubType, packetOut );
      }
      break;
   case PacketType_ErrorReport:
      {   
         // put here just to avoid a function call for only one type.
         PacketErrorReport* error = new PacketErrorReport();
         //error->SerializeIn( bufferIn, bufferOffset, networkMinorVersion );
         *packetOut = error;
         success = true;
      }
      break;
   case PacketType_Cheat:
      {
         success = CreateCheat( packetSubType, packetOut );
      }
      break;
   case PacketType_Purchase:
      {
         success = CreatePurchase( packetSubType, packetOut );
      }
      break;
   case PacketType_Analytics:
      {
         success = CreateAnalytics( packetSubType, packetOut );
      }
      break;
   case PacketType_Tournament:
      {
         success = CreateTournament( packetSubType, packetOut );
      }
      break;
   case PacketType_UserStats:
      {
         success = CreateUserStats( packetSubType, packetOut );
      }
      break;
   case PacketType_Notification:
      {
         success = CreateNotification( packetSubType, packetOut );
      }
      break;
   case PacketType_Invitation:
      {
         success = CreateInvitation( packetSubType, packetOut );
      }
      break;
   }
   return success;
}

//---------------------------------------------------------------------------------------------------------------------------------------

void     PacketFactory::CleanupPacket( BasePacket*& packet )
{
   if( packet )
   {
      U8 packetType = packet->packetType;
      switch( packetType )
      {
      case PacketType_GatewayWrapper:
         {
            PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
            delete wrapper->pPacket;
         }
         break;
      case PacketType_ServerToServerWrapper:
         {
            PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );
            delete wrapper->pPacket;
         }
         break;
      case PacketType_ServerJobWrapper:
         {
            PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
            delete wrapper->pPacket;
         }
         break;
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

bool  PacketFactory::CreateBasePacket( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType )
   {
      case BasePacket::BasePacket_Type:
         {
            *packetOut = CreatePacket< BasePacket >();
         }
         return true;
      case BasePacket::BasePacket_Hello:
         {
            *packetOut = CreatePacket< PacketHello >();
         }
         return true;
      case BasePacket::BasePacket_CommsHandshake:
         {
            *packetOut = CreatePacket< PacketCommsHandshake >();
         }
         return true;
      case BasePacket::BasePacket_RerouteRequest:
         {
            *packetOut = CreatePacket< PacketRerouteRequest >();
         }
         return true;
      case BasePacket::BasePacket_RerouteRequestResponse:
         {
            *packetOut = CreatePacket< PacketRerouteRequestResponse >();
         }
         return true;
      case BasePacket::BasePacket_QOS:
         {
            *packetOut = CreatePacket< Packet_QOS_ReportToClient >();
         }
         return true;
      case BasePacket::BasePacket_TestOnly:
         {
            *packetOut = CreatePacket< PacketBase_TestOnly >();
         }
         return true;
   }
   return false;
}

//---------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------------------------

bool  PacketFactory::CreateLogin( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType )
   {
      case PacketLogin::LoginType_Login:
         {
            *packetOut = CreatePacket< PacketLogin >();
         }
         return true;
      case PacketLogin::LoginType_LoginFromGateway:
         {
            *packetOut = CreatePacket< PacketLoginFromGateway >();
         }
         return true;
      case PacketLogin::LoginType_Logout:
         {
            *packetOut = CreatePacket< PacketLogout >();
         }
         return true;
      case PacketLogin::LoginType_PacketLogoutToClient:
         {
            *packetOut = CreatePacket< PacketLogoutToClient >();
         }
         return true;
      case PacketLogin::LoginType_EchoToServer:
         {
            *packetOut = CreatePacket< PacketLogin_EchoToServer >();
         }
         return true;
      case PacketLogin::LoginType_EchoToClient:
         {
            *packetOut = CreatePacket< PacketLogin_EchoToClient >();
         }
         return true;
      case PacketLogin::LoginType_InformClientOfLoginStatus:
         {
            *packetOut = CreatePacket< PacketLoginToClient >();
         }
         return true;
      case PacketLogin::LoginType_InformGatewayOfLoginStatus:
         {
            *packetOut = CreatePacket< PacketLoginToGateway >();
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogin:
         {
            *packetOut = CreatePacket< PacketPrepareForUserLogin >();
         }
         return true;
      case PacketLogin::LoginType_PrepareForUserLogout:
         {
            *packetOut = CreatePacket< PacketPrepareForUserLogout >();
         }
         return true;
      case PacketLogin::LoginType_ThrottleUsersConnection:
         {
            *packetOut = CreatePacket< PacketLoginThrottlePackets >();
         }
         return true;
      case PacketLogin::LoginType_DebugThrottleUsersConnection:
         {
            *packetOut = CreatePacket< PacketLoginDebugThrottleUserPackets >();
         }
         return true;
      case PacketLogin::LoginType_CreateAccount:
         {
            *packetOut = CreatePacket< PacketCreateAccount >();
         }
         return true;
      case PacketLogin::LoginType_CreateAccountResponse:
         {
            *packetOut = CreatePacket< PacketCreateAccountResponse >();
         }
         return true;
      case PacketLogin::LoginType_RequestListOfPurchases:
         {
            *packetOut = CreatePacket< PacketListOfUserPurchasesRequest >();
         }
         return true;
      case PacketLogin::LoginType_AddPurchaseEntry:
         {
            *packetOut = CreatePacket< PacketAddPurchaseEntry >();
         }
         return true;
      case PacketLogin::LoginType_ListOfAggregatePurchases:
         {
            *packetOut = CreatePacket< PacketListOfUserAggregatePurchases >();
         }
         return true;
      case PacketLogin::LoginType_ListOfProductsS2S:
         {
            *packetOut = CreatePacket< PacketListOfUserProductsS2S >();
         }
         return true;
      case PacketLogin::LoginType_RequestUserProfile:
         {
            *packetOut = CreatePacket< PacketRequestUserProfile >();
         }
         return true;
      case PacketLogin::LoginType_RequestUserProfileResponse:
         {
            *packetOut = CreatePacket< PacketRequestUserProfileResponse >();
         }
         return true;
      case PacketLogin::LoginType_UpdateUserProfile:
         {
            *packetOut = CreatePacket< PacketUpdateUserProfile >();
         }
         return true;
      case PacketLogin::LoginType_UpdateUserProfileResponse:
         {
            *packetOut = CreatePacket< PacketUpdateUserProfileResponse >();
         }
         return true;
      case PacketLogin::LoginType_RequestListOfProducts:
         {
            *packetOut = CreatePacket< PacketRequestListOfProducts >();
         }
         return true;
      case PacketLogin::LoginType_RequestListOfProductsResponse:
         {
            *packetOut = CreatePacket< PacketRequestListOfProductsResponse >();
         }
         return true;
      case PacketLogin::LoginType_RequestOtherUserProfile:
         {
            *packetOut = CreatePacket< PacketRequestOtherUserProfile >();
         }
         return true;
      case PacketLogin::LoginType_RequestOtherUserProfileResponse:
         {
            *packetOut = CreatePacket< PacketRequestOtherUserProfileResponse >();
         }
         return true;

      case PacketLogin::LoginType_UpdateSelfProfile:
         {
            *packetOut = CreatePacket< PacketUpdateSelfProfile >();
         }
         return true;
      case PacketLogin::LoginType_UpdateSelfProfileResponse:
         {
            *packetOut = CreatePacket< PacketUpdateSelfProfileResponse >();
         }
         return true;

      case PacketLogin::LoginType_UserUpdateProfile:
         {
            *packetOut = CreatePacket< PacketUserUpdateProfile >();
         }
         return true;

      case PacketLogin::LoginType_UserListOfPurchasesWasUpdated:
         {
            *packetOut = CreatePacket< PacketListOfUserPurchasesUpdated >();
         }
         return true;
      case PacketLogin::LoginType_LogoutAllUsers:
         {
            *packetOut = CreatePacket< PacketLogin_LogoutAllUsers >();
         }
         return true;
      case PacketLogin::LoginType_ListOfMissingFeatures:
         {
            *packetOut = CreatePacket< PacketLogin_ListOfMissingFeatures >();
         }
         return true;

      case PacketLogin::LoginType_ExpireUserLogin:
         {
            *packetOut = CreatePacket< PacketLoginExpireUser >();
         }
         return true;
      case PacketLogin::LoginType_RequestServiceToFlushAllUserLogins:
         {
            *packetOut = CreatePacket< PacketLogin_FlushAllConnectedUsers >();
         }
         return true;
      case PacketLogin::LoginType_RequestLoginStatusOfAllUsers:
         {
            *packetOut = CreatePacket< PacketLogin_RequestAllConnectedUsers >();
         }
         return true;
      case PacketLogin::LoginType_RequestToLogoutAllUsersForGame:
         {
            *packetOut = CreatePacket< PacketLogin_RequestLogoutAllUsersForGame >();
         }
         return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool  PacketFactory::CreateChat( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType )
   {
   case PacketChatToServer::ChatType_ChatToServer:
      {
         *packetOut = CreatePacket< PacketChatToServer >();
      }
      return true;
   case PacketChatToServer::ChatType_ChatToClient:
      {
         *packetOut = CreatePacket< PacketChatToClient >();
      }
      return true;
   case PacketChatToServer::ChatType_EchoToServer:
      {
         *packetOut = CreatePacket< PacketChat_EchoToServer >();
      }
      return true;
   case PacketChatToServer::ChatType_EchoToClient:
      {
         *packetOut = CreatePacket< PacketChat_EchoToClient >();
      }
      return true;
   case PacketChatToServer::ChatType_UserChatStatusChange:
      {
         *packetOut = CreatePacket< PacketChatUserStatusChangeBase >();
      }
      return true;

  /* case PacketChatToServer::ChatType_ChangeChatChannel:
      {
         *packetOut = CreatePacket< PacketChangeChatChannel >();
      }
      return true;*/

   case PacketChatToServer::ChatType_RequestHistory:
      {
         *packetOut = CreatePacket< PacketChatHistoryRequest >();
      }
      return true;

   /*case PacketChatToServer::ChatType_ChangeChatChannelToClient:
      {
         *packetOut = CreatePacket< PacketChangeChatChannelToClient >();
      }
      return true;*/

   case PacketChatToServer::ChatType_SendListOfChannelsToClient:
      {
         *packetOut = CreatePacket< PacketChatChannelListToClient >();
      }
      return true;
   case PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer:
      {
         *packetOut = CreatePacket< PacketChatUserAddedToChatChannelFromGameServer >();
      }
      return true;
   case PacketChatToServer::ChatType_RequestHistoryResult:
      {
         *packetOut = CreatePacket< PacketChatHistoryResult >();
      }
      return true;
   case PacketChatToServer::ChatType_RequestHistorySinceLastLogin:
      {
         *packetOut = CreatePacket< PacketChatMissedHistoryRequest >();
      }
      return true;
   case PacketChatToServer::ChatType_RequestHistorySinceLastLoginResponse:
      {
         *packetOut = CreatePacket< PacketChatMissedHistoryResult >();
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannel:
      {
         *packetOut = CreatePacket< PacketChatCreateChatChannel >();
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannelResponse:
      {
         *packetOut = CreatePacket< PacketChatCreateChatChannelResponse >();
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannelFromGameServer:
      {
         *packetOut = CreatePacket< PacketChatCreateChatChannelFromGameServer >();
      }
      return true;
   case PacketChatToServer::ChatType_CreateChatChannelFromGameServerResponse:
      {
         *packetOut = CreatePacket< PacketChatCreateChatChannelFromGameServerResponse >();
      }
      return true;

   case PacketChatToServer::ChatType_DeleteChatChannel:
      {
         *packetOut = CreatePacket< PacketChatDeleteChatChannel >();
      }
      return true;
   case PacketChatToServer::ChatType_DeleteChatChannelResponse:
      {
         *packetOut = CreatePacket< PacketChatDeleteChatChannelResponse >();
      }
      return true;

   case PacketChatToServer::ChatType_DeleteChatChannelFromGameServer:
      {
         *packetOut = CreatePacket< PacketChatDeleteChatChannelFromGameServer >();
      }
      return true;
   case PacketChatToServer::ChatType_DeleteChatChannelFromGameServerResponse:
      {
         *packetOut = CreatePacket< PacketChatDeleteChatChannelFromGameServerResponse >();
      }
      return true;

   case PacketChatToServer::ChatType_InviteUserToChatChannel:
      {
         *packetOut = CreatePacket< PacketChatInviteUserToChatChannel >();
      }
      return true;
   case PacketChatToServer::ChatType_InviteUserToChatChannelResponse:
      {
         *packetOut = CreatePacket< PacketChatInviteUserToChatChannelResponse >();
      }
      return true;

   case PacketChatToServer::ChatType_AddUserToChatChannel:
      {
         *packetOut = CreatePacket< PacketChatAddUserToChatChannel >();
      }
      return true;
   case PacketChatToServer::ChatType_AddUserToChatChannelResponse:
      {
         *packetOut = CreatePacket< PacketChatAddUserToChatChannelResponse >();
      }
      return true;
   case PacketChatToServer::ChatType_AddUserToChatChannelGameServer:
      {
         *packetOut = CreatePacket< PacketChatAddUserToChatChannelGameServer >();
      }
      return true;
   case PacketChatToServer::ChatType_AddUserToChatChannelGameServerResponse:
      {
         *packetOut = CreatePacket< PacketChatAddUserToChatChannelGameServerResponse >();
      }
      return true;


   case PacketChatToServer::ChatType_RemoveUserFromChatChannel:
      {
         *packetOut = CreatePacket< PacketChatRemoveUserFromChatChannel >();
      }
      return true;
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelResponse:
      {
         *packetOut = CreatePacket< PacketChatRemoveUserFromChatChannelResponse >();
      }
      return true;
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServer:
      {
         *packetOut = CreatePacket< PacketChatRemoveUserFromChatChannelGameServer >();
      }
      return true;
   case PacketChatToServer::ChatType_RemoveUserFromChatChannelGameServerResponse:
      {
         *packetOut = CreatePacket< PacketChatRemoveUserFromChatChannelGameServerResponse >();
      }
      return true;
   
   case PacketChatToServer::ChatType_RequestChatters:
      {
         *packetOut = CreatePacket< PacketChatRequestChatters >();
      }
      return true;
   case PacketChatToServer::ChatType_RequestChattersResponse:
      {
         *packetOut = CreatePacket< PacketChatRequestChattersResponse >();
      }
      return true;
   
   case PacketChatToServer::ChatType_EnableDisableFiltering:
      {
         *packetOut = CreatePacket< PacketChatEnableFiltering >();
      }
      return true;
   case PacketChatToServer::ChatType_EnableDisableFilteringResponse:
      {
         *packetOut = CreatePacket< PacketChatEnableFilteringResponse >();
      }
      return true;

   case PacketChatToServer::ChatType_ListAllMembersInChatChannel:
      {
         *packetOut = CreatePacket< PacketChatListAllMembersInChatChannel >();
      }
      return true;
   case PacketChatToServer::ChatType_ListAllMembersInChatChannelResponse:
      {
         *packetOut = CreatePacket< PacketChatListAllMembersInChatChannelResponse >();
      }
      return true;
   case PacketChatToServer::ChatType_AdminLoadAllChannels:
      {
         *packetOut = CreatePacket< PacketChatAdminLoadAllChannels >();
      }
      return true;
   case PacketChatToServer::ChatType_AdminLoadAllChannelsResponse:
      {
         *packetOut = CreatePacket< PacketChatAdminLoadAllChannelsResponse >();
      }
      return true;
   
   case PacketChatToServer::ChatType_AdminRequestChatChannelList:
      {
         *packetOut = CreatePacket< PacketChatAdminRequestChatChannelList >();
      }
      return true;
   case PacketChatToServer::ChatType_AdminRequestChatChannelListResponse:
      {
         *packetOut = CreatePacket< PacketChatAdminRequestChatChannelListResponse >();
      }
      return true;
   
   case PacketChatToServer::ChatType_AdminRequestUsersList:
      {
         *packetOut = CreatePacket< PacketChatAdminRequestUsersList >();
      }
      return true;
   case PacketChatToServer::ChatType_AdminRequestUsersListResponse:
      {
         *packetOut = CreatePacket< PacketChatAdminRequestUsersListResponse >();
      }
      return true;

   case PacketChatToServer::ChatType_RenameChatChannel:
      {
         *packetOut = CreatePacket< PacketChatRenameChannel >();
      }
      return true;
   case PacketChatToServer::ChatType_RenameChatChannelResponse:
      {
         *packetOut = CreatePacket< PacketChatRenameChannelResponse >();
      }
      return true;
   case PacketChatToServer::ChatType_UpdateProfile:
      {
         *packetOut = CreatePacket< PacketChat_UserProfileChange >();
      }
      return true;
   case PacketChatToServer::ChatType_MarkChannelHistoryAsRead:
      {
         *packetOut = CreatePacket< PacketChat_MarkChannelHistoryAsRead >();
      }
      return true;
   case PacketChatToServer::ChatType_MarkP2PHistoryAsRead:
      {
         *packetOut = CreatePacket< PacketChat_MarkP2PHistoryAsRead >();
      }
      return true;
   case PacketChatToServer::ChatType_UserChatHistory:
      {
         *packetOut = CreatePacket< PacketChat_UserChatHistory >();
      }
      return true;
   case PacketChatToServer::ChatType_ChannelChatHistory:
      {
         *packetOut = CreatePacket< PacketChat_ChannelChatHistory >();
      }
      return true;
   }

   return false;
}

//---------------------------------------------------------------------------------------------------------

bool     PacketFactory::CreateUserInfo( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType )
   {
   case PacketUserInfo::InfoType_FriendsListRequest:
      {
         *packetOut = CreatePacket< PacketFriendsListRequest >();
      }
      return true;
   case PacketUserInfo::InfoType_FriendsList:
      {
         *packetOut = CreatePacket< PacketFriendsList >();
      }
      return true;

   case PacketUserInfo::InfoType_ChatChannelListRequest:
      {
         *packetOut = CreatePacket< PacketChatChannelListRequest >();
      }
      return true;

   case PacketUserInfo::InfoType_ChatChannelList:
      {
         *packetOut = CreatePacket< PacketChatChannelList >();
      }
      return true;

   case PacketUserInfo::InfoType_GroupsListRequest:
      {
         *packetOut = CreatePacket< PacketGroupsListRequest >();
      }
      return true;

   case PacketUserInfo::InfoType_GroupsList:
      {
         *packetOut = CreatePacket< PacketGroupsList >();
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateContact( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType )
   {
   case PacketContact::ContactType_Base:
      {
         *packetOut = CreatePacket< PacketContact >();
      }
      return true;
   case PacketContact::ContactType_TestNotification:
      {
         *packetOut = CreatePacket< PacketContact_TestNotification >();
      }
      return true;

   case PacketContact::ContactType_EchoToServer:
      {
         *packetOut = CreatePacket< PacketContact_EchoToServer >();
      }
      return true;
   case PacketContact::ContactType_EchoToClient:
      {
         *packetOut = CreatePacket< PacketContact_EchoToClient >();
      }
      return true;

   case PacketContact::ContactType_GetListOfContacts:
      {
         *packetOut = CreatePacket< PacketContact_GetListOfContacts >();
      }
      return true;

   case PacketContact::ContactType_GetListOfContactsResponse:
      {
         *packetOut = CreatePacket< PacketContact_GetListOfContactsResponse >();
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitations:
      {
         *packetOut = CreatePacket< PacketContact_GetListOfInvitations >();
      }
      return true;
   case PacketContact::ContactType_GetListOfInvitationsResponse:
      {
         *packetOut = CreatePacket< PacketContact_GetListOfInvitationsResponse >();
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitationsSent:
      {
         *packetOut = CreatePacket< PacketContact_GetListOfInvitationsSent >();
      }
      return true;

   case PacketContact::ContactType_GetListOfInvitationsSentResponse:
      {
         *packetOut = CreatePacket< PacketContact_GetListOfInvitationsSentResponse >();
      }
      return true;
   case PacketContact::ContactType_InviteContact:
      {
         *packetOut = CreatePacket< PacketContact_InviteContact >();
      }
      return true;
   case PacketContact::ContactType_InviteSentNotification:
      {
         *packetOut = CreatePacket< PacketContact_InviteSentNotification >();
      }
      return true;
   case PacketContact::ContactType_InviteReceived:
      {
         *packetOut = CreatePacket< PacketContact_InviteReceivedNotification >();
      }
      return true;
   case PacketContact::ContactType_RemoveContact:
      {
         *packetOut = CreatePacket< PacketContact_ContactRemove >();
      }
      return true;
    case PacketContact::ContactType_RemoveInvitation:
      {
         *packetOut = CreatePacket< PacketContact_RemoveInvitation >();
      }
      return true;
   case PacketContact::ContactType_AcceptInvite:
      {
         *packetOut = CreatePacket< PacketContact_AcceptInvite >();
      }
      return true;
   case PacketContact::ContactType_InvitationAccepted:
      {
         *packetOut = CreatePacket< PacketContact_InvitationAccepted >();
      }
      return true;
   case PacketContact::ContactType_DeclineInvitation:
      {
         *packetOut = CreatePacket< PacketContact_DeclineInvitation >();
      }
      return true;

   case PacketContact::ContactType_Search:
      {
         *packetOut = CreatePacket< PacketContact_SearchForUser >();
      }
      return true;
   case PacketContact::ContactType_SearchResults:
      {
         *packetOut = CreatePacket< PacketContact_SearchForUserResult >();
      }
      return true;
   case PacketContact::ContactType_BlockUser:
      {
         *packetOut = CreatePacket< PacketContact_InviteBlockUser >();
      }
      return true;


   case PacketContact::ContactType_UserOnlineStatusChange:
      {
         *packetOut = CreatePacket< PacketContact_FriendOnlineStatusChange >();
      }
      return true;

   case PacketContact::ContactType_SetNotation:
      {
         *packetOut = CreatePacket< PacketContact_SetNotationOnUser >();
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateAsset( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType )
   {
   case PacketAsset::AssetType_Base:
      {
         *packetOut = CreatePacket< PacketAsset >();
      }
      return true;
   case PacketAsset::AssetType_TestNotification:
      {
         *packetOut = CreatePacket< PacketAsset_TestNotification >();
      }
      return true;
   case PacketAsset::AssetType_EchoToServer:
      {
         *packetOut = CreatePacket< PacketAsset_EchoToServer >();
      }
      return true;
   case PacketAsset::AssetType_EchoToClient:
      {
         *packetOut = CreatePacket< PacketAsset_EchoToClient >();
      }
      return true;
   case PacketAsset::AssetType_GetListOfStaticAssets:
      {
         *packetOut = CreatePacket< PacketAsset_GetListOfStaticAssets >();
      }
      return true;
   case PacketAsset::AssetType_GetListOfStaticAssetsResponse:
      {
         *packetOut = CreatePacket< PacketAsset_GetListOfStaticAssetsResponse >();
      }
      return true;
   case PacketAsset::AssetType_GetListOfDynamicAssets:
      {
         *packetOut = CreatePacket< PacketAsset_GetListOfDynamicAssets >();
      }
      return true;
   case PacketAsset::AssetType_GetListOfDynamicAssetsResponse:
      {
         *packetOut = CreatePacket< PacketAsset_GetListOfDynamicAssetsResponse >();
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssetCategories:
      {
         *packetOut = CreatePacket< PacketAsset_GetListOfAssetCategories >();
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssetCategoriesResponse:
      {
         *packetOut = CreatePacket< PacketAsset_GetListOfAssetCategoriesResponse >();
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssets:
      {
         *packetOut = CreatePacket< PacketAsset_GetListOfAssets >();
      }
      return true;
   case PacketAsset::AssetType_GetListOfAssetsResponse:
      {
         *packetOut = CreatePacket< PacketAsset_GetListOfAssetsResponse >();
      }
      return true;
   
   case PacketAsset::AssetType_RequestAsset:
      {
         *packetOut = CreatePacket< PacketAsset_RequestAsset >();
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateDbQuery( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType )
   {
   case PacketDbQuery::QueryType_Query:
      {
         *packetOut = CreatePacket< PacketDbQuery >();
      }
      return true;
   case PacketDbQuery::QueryType_Result:
      {
         *packetOut = CreatePacket< PacketDbQueryResult >();
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateGame( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType )
   {
   case PacketGameToServer::GamePacketType_LoginToServer:
      {
         *packetOut = CreatePacket< PacketGameToServer >();
      }
      return true;
   case PacketGameToServer::GamePacketType_CreateGame:
      {
         *packetOut = CreatePacket< PacketCreateGame >();
      }
      return true;
   case PacketGameToServer::GamePacketType_CreateGameResponse:
      {
         *packetOut = CreatePacket< PacketCreateGameResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_DeleteGame:
      {
         *packetOut = CreatePacket< PacketDeleteGame >();
      }
      return true;
   case PacketGameToServer::GamePacketType_DeleteGameResponse:
      {
         *packetOut = CreatePacket< PacketDeleteGameResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_ForfeitGame:
      {
         *packetOut = CreatePacket< PacketForfeitGame >();
      }
      return true;
   case PacketGameToServer::GamePacketType_ForfeitGameResponse:
      {
         *packetOut = CreatePacket< PacketForfeitGameResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_QuitGame:
      {
         *packetOut = CreatePacket< PacketQuitGame >();
      }
      return true;
   case PacketGameToServer::GamePacketType_QuitGameResponse:
      {
         *packetOut = CreatePacket< PacketQuitGameResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_AddUser:
      {
         *packetOut = CreatePacket< PacketAddUserToGame >();
      }
      return true;
   case PacketGameToServer::GamePacketType_AddUserResponse:
      {
         *packetOut = CreatePacket< PacketAddUserToGameResponse >();
      }
      return true;

   case PacketGameToServer::GamePacketType_RemoveUser:
      {
         *packetOut = CreatePacket< PacketRemoveUserFromGame >();
      }
      return true;
   case PacketGameToServer::GamePacketType_RemoveUserResponse:
      {
         *packetOut = CreatePacket< PacketRemoveUserFromGameResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_AdvanceTurn:
      {
         *packetOut = CreatePacket< PacketGameAdvanceTurn >();
      }
      return true;
   case PacketGameToServer::GamePacketType_TurnWasAdvanced:
      {
         *packetOut = CreatePacket< PacketGameAdvanceTurnResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfGames:
      {
         *packetOut = CreatePacket< PacketRequestListOfGames >();
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfGamesResponse:
      {
         *packetOut = CreatePacket< PacketRequestListOfGamesResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfUsersInGame:
      {
         *packetOut = CreatePacket< PacketRequestListOfUsersInGame >();
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestListOfUsersInGameResponse:
      {
         *packetOut = CreatePacket< PacketRequestListOfUsersInGameResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestUserWinLoss:
      {
         *packetOut = CreatePacket< PacketRequestUserWinLoss >();
      }
      return true;
   case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
      {
         *packetOut = CreatePacket< PacketRequestUserWinLossResponse >();
      }
      return true;
   case PacketGameToServer::GamePacketType_ListOfGames:
      {
         *packetOut = CreatePacket< PacketListOfGames >();
      }
      return true;
   case PacketGameToServer::GamePacketType_GameIdentification:
      {
         *packetOut = CreatePacket< PacketGameIdentification >();
      }
      return true;
   case PacketGameToServer::GamePacketType_RawGameData:
      {
         *packetOut = CreatePacket< PacketGameplayRawData >();
      }
      return true;
   case PacketGameToServer::GamePacketType_EchoToServer:
      {
         *packetOut = CreatePacket< PacketGame_EchoToServer >();
      }
      return true;
   case PacketGameToServer::GamePacketType_EchoToClient:
      {
         *packetOut = CreatePacket< PacketGame_EchoToClient >();
      }
      return true;
   case PacketGameToServer::GamePacketType_TestHook:
      {
         *packetOut = CreatePacket< PacketGame_TestHook >();
      }
      return true;

   case PacketGameToServer::GamePacketType_Notification:
      {
         *packetOut = CreatePacket< PacketGame_Notification >();
      }
      return true;
   case PacketGameToServer::GamePacketType_ServiceOutage:
      {
         *packetOut = CreatePacket< ClientSide_ServerOutageSchedule >();
      }
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateCheat( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType ) //PacketType_Cheat
   {
   case PacketCheat::Cheat_Basic:
      {
         *packetOut = CreatePacket< PacketCheat >();
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreatePurchase( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType ) //PacketType_Cheat
   {
   case PacketPurchase::PurchaseType_Base:
      {
         *packetOut = CreatePacket< PacketPurchase >();
      }
      return true;
   case PacketPurchase::PurchaseType_TestNotification:
      {
         *packetOut = CreatePacket< PacketPurchase_TestNotification >();
      }
      return true;
   case PacketPurchase::PurchaseType_EchoToServer:
      {
         *packetOut = CreatePacket< PacketPurchase_EchoToServer >();
      }
      return true;
   case PacketPurchase::PurchaseType_EchoToClient:
      {
         *packetOut = CreatePacket< PacketPurchase_EchoToClient >();
      }
      return true;
   case PacketPurchase::PurchaseType_Buy:
      {
         *packetOut = CreatePacket< PacketPurchase_Buy >();
      }
      return true;
   case PacketPurchase::PurchaseType_BuyResponse:
      {
         *packetOut = CreatePacket< PacketPurchase_BuyResponse >();
      }
      return true;
   case PacketPurchase::PurchaseType_RequestListOfSales:
      {
         *packetOut = CreatePacket< PacketPurchase_RequestListOfSales >();
      }
      return true;
   case PacketPurchase::PurchaseType_RequestListOfSalesResponse:
      {
         *packetOut = CreatePacket< PacketPurchase_RequestListOfSalesResponse >();
      }
      return true;
   case PacketPurchase::PurchaseType_ValidatePurchaseReceipt:
      {
         *packetOut = CreatePacket< PacketPurchase_ValidatePurchaseReceipt >();
      }
      return true;
   case PacketPurchase::PurchaseType_ValidatePurchaseReceiptResponse:
      {
         *packetOut = CreatePacket< PacketPurchase_ValidatePurchaseReceiptResponse >();
      }
      return true;
   }
   
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateAnalytics( int packetSubType, BasePacket** packetOut ) const
{
   *packetOut = CreatePacket< PacketAnalytics >();
   return true;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateTournament( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType ) //PacketType_Tournament
   {
    case PacketTournament::TournamentType_Base:
      {
         *packetOut = CreatePacket< PacketTournament >();
      }
      return true;
   case PacketTournament::TournamentType_TestNotification:
      {
         *packetOut = CreatePacket< PacketTournament_TestNotification >();
      }
      return true;
   case PacketTournament::TournamentType_RequestListOfTournaments:
      {
         *packetOut = CreatePacket< PacketTournament_RequestListOfTournaments >();
      }
      return true;
   case PacketTournament::TournamentType_RequestListOfTournamentsResponse:
      {
         *packetOut = CreatePacket< PacketTournament_RequestListOfTournamentsResponse >();
      }
      return true;
   case PacketTournament::TournamentType_UserRequestsEntryInTournament:
      {
         *packetOut = CreatePacket< PacketTournament_UserRequestsEntryInTournament >();
      }
      return true;
   case PacketTournament::TournamentType_UserRequestsEntryInTournamentResponse:
      {
         *packetOut = CreatePacket< PacketTournament_UserRequestsEntryInTournamentResponse >();
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntry:
      {
         *packetOut = CreatePacket< PacketTournament_PurchaseTournamentEntry >();
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryResponse:
      {
         *packetOut = CreatePacket< PacketTournament_PurchaseTournamentEntryResponse >();
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryRefund:
      {
         *packetOut = CreatePacket< PacketTournament_PurchaseTournamentEntryRefund >();
      }
      return true;
   case PacketTournament::TournamentType_PurchaseTournamentEntryRefundResponse:
      {
         *packetOut = CreatePacket< PacketTournament_PurchaseTournamentEntryRefundResponse >();
      }
      return true;
   default:
      assert( 0 );
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateUserStats( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType ) //PacketType_Tournament
   {
    case PacketUserStats::UserStatsType_Base:
      {
         *packetOut = CreatePacket< PacketUserStats >();
      }
      return true;
   case PacketUserStats::UserStatsType_TestUserStats:
      {
         *packetOut = CreatePacket< PacketUserStats_TestUserStats >();
      }
      return true;
   case PacketUserStats::UserStatsType_RequestListOfUserStats:
      {
         *packetOut = CreatePacket< PacketUserStats_RequestListOfUserStats >();
      }
      return true;
   case PacketUserStats::UserStatsType_RequestListOfUserStatsResponse:
      {
         *packetOut = CreatePacket< PacketUserStats_RequestListOfUserStatsResponse >();
      }
      return true;
   case PacketUserStats::UserStatsType_RecordUserStats:
      {
         *packetOut = CreatePacket< PacketUserStats_RecordUserStats >();
      }
      return true;
   case PacketUserStats::UserStatsType_RecordUserStatsResponse:
      {
         *packetOut = CreatePacket< PacketUserStats_RecordUserStatsResponse >();
      }
      return true;
   case PacketUserStats::UserStatsType_ReportGameResult:
      {
         *packetOut = CreatePacket< PacketUserStats_ReportGameResult >();
      }
      return true;
   case PacketUserStats::UserStatsType_ReportUserForfeit:
      {
         *packetOut = CreatePacket< PacketUserStats_ReportUserForfeit >();
      }
      return true;
   case PacketUserStats::UserStatsType_RequestGameFactionStats:
      {
         *packetOut = CreatePacket< PacketUserStats_RequestGameFactionStats >();
      }
      return true;
   case PacketUserStats::UserStatsType_RequestGameProfile:
      {
         *packetOut = CreatePacket< PacketUserStats_RequestGameProfile >();
      }
      return true;
   case PacketUserStats::UserStatsType_RequestUserProfileStats:
      {
         *packetOut = CreatePacket< PacketUserStats_RequestUserProfileStats >();
      }
      return true;
   case PacketUserStats::UserStatsType_RequestUserProfileStatsResponse:
      {
         *packetOut = CreatePacket< PacketUserStats_RequestUserProfileStatsResponse >();
      }
      return true;

      
   
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateNotification( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType ) //PacketType_Cheat
   {
   case PacketNotification::NotificationType_Base:
      {
         *packetOut = CreatePacket< PacketNotification >();
      }
      return true;
   case PacketNotification::NotificationType_TestNotification:
      {
         *packetOut = CreatePacket< PacketNotification_TestNotification >();
      }
      return true;
   case PacketNotification::NotificationType_EchoToServer:
      {
         *packetOut = CreatePacket< PacketNotification_EchoToServer >();
      }
      return true;
   case PacketNotification::NotificationType_EchoToClient:
      {
         *packetOut = CreatePacket< PacketNotification_EchoToClient >();
      }
      return true;
   case PacketNotification::NotificationType_RegisterDevice:
      {
         *packetOut = CreatePacket< PacketNotification_RegisterDevice >();
      }
      return true;
   case PacketNotification::NotificationType_RegisterDeviceResponse:
      {
         *packetOut = CreatePacket< PacketNotification_RegisterDeviceResponse >();
      }
      return true;
   case PacketNotification::NotificationType_UpdateDevice:
      {
         *packetOut = CreatePacket< PacketNotification_UpdateDevice >();
      }
      return true;
      
   case PacketNotification::NotificationType_SendNotification:
      {
         *packetOut = CreatePacket< PacketNotification_SendNotification >();
      }
      return true;

   case PacketNotification::NotificationType_UpdateNotificationCount:
      {
         *packetOut = CreatePacket< PacketNotification_UpdateNotificationCount >();
      }
      return true;
      
   case PacketNotification::NotificationType_RequestListOfDevices:
      {
         *packetOut = CreatePacket< PacketNotification_RequestListOfDevices >();
      }
      return true;
   case PacketNotification::NotificationType_RequestListOfDevicesResponse:
      {
         *packetOut = CreatePacket< PacketNotification_RequestListOfDevicesResponse >();
      }
      return true;

   case PacketNotification::NotificationType_RemoveDevice:
      {
         *packetOut = CreatePacket< PacketNotification_RemoveDevice >();
      }
      return true;
   case PacketNotification::NotificationType_RemoveDeviceResponse:
      {
         *packetOut = CreatePacket< PacketNotification_RemoveDeviceResponse >();
      }
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateInvitation( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType ) 
   {
   case PacketInvitation::InvitationType_Base:
      {
         *packetOut = CreatePacket< PacketInvitation >();
      }
      return true;
   case PacketInvitation::InvitationType_EchoToServer:
      {
         *packetOut = CreatePacket< PacketInvitation_EchoToServer >();
      }
      return true;
   case PacketInvitation::InvitationType_EchoToClient:
      {
         *packetOut = CreatePacket< PacketInvitation_EchoToClient >();
      }
      return true;
   case PacketInvitation::InvitationType_InviteUser:
      {
         *packetOut = CreatePacket< PacketInvitation_InviteUser >();
      }
      return true;
   case PacketInvitation::InvitationType_InviteUserResponse:
      {
         *packetOut = CreatePacket< PacketInvitation_InviteUserResponse >();
      }
      return true;
   case PacketInvitation::InvitationType_CancelInvitation:
      {
         *packetOut = CreatePacket< PacketInvitation_CancelInvitation >();
      }
      return true;
   case PacketInvitation::InvitationType_InvitationWasCancelled:
      {
         *packetOut = CreatePacket< PacketInvitation_InvitationWasCancelled >();
      }
      return true;
   case PacketInvitation::InvitationType_RejectInvitation:
      {
         *packetOut = CreatePacket< PacketInvitation_RejectInvitation >();
      }
      return true;

   case PacketInvitation::InvitationType_RejectInvitationResponse:
      {
         *packetOut = CreatePacket< PacketInvitation_RejectInvitationResponse >();
      }
      return true;
   case PacketInvitation::InvitationType_AcceptInvitation:
      {
         *packetOut = CreatePacket< PacketInvitation_AcceptInvitation >();
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitations:
      {
         *packetOut = CreatePacket< PacketInvitation_GetListOfInvitations >();
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsResponse:
      {
         *packetOut = CreatePacket< PacketInvitation_GetListOfInvitationsResponse >();
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsForGroup:
      {
         *packetOut = CreatePacket< PacketInvitation_GetListOfInvitationsForGroup >();
      }
      return true;
   case PacketInvitation::InvitationType_GetListOfInvitationsForGroupResponse:
      {
         *packetOut = CreatePacket< PacketInvitation_GetListOfInvitationsForGroupResponse >();
      }
      return true;
    }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateServerToServerWrapper( int packetSubType, BasePacket** packetOut ) const
{
   PacketServerToServerWrapper* wrapper = new PacketServerToServerWrapper();
   *packetOut = wrapper;
   return true;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateServerInfo( int packetSubType, BasePacket** packetOut ) const
{
   switch( packetSubType ) //PacketType_Tournament
   {
    case PacketServerConnectionInfo::PacketServerIdentifier_TypicalInfo:
      {
         *packetOut = CreatePacket< PacketServerIdentifier >();
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_ConnectionInfo:
      {
         *packetOut = CreatePacket< PacketServerConnectionInfo >();
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_Disconnect:
      {
         *packetOut = CreatePacket< PacketServerDisconnect >();
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_GatewayRequestLB_ConnectionIds:
      {
         *packetOut = CreatePacket< PacketServerToServer_GatewayRequestLB_ConnectionIds >();
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_GatewayRequestLB_ConnectionIdsResponse:
      {
         *packetOut = CreatePacket< PacketServerToServer_GatewayRequestLB_ConnectionIdsResponse >();
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_ServerOutageSchedule:
      {
         *packetOut = CreatePacket< PacketServerConnectionInfo_ServerOutageSchedule >();
      }
      return true;
   case PacketServerConnectionInfo::PacketServerIdentifier_KeepAlive:
      {
         *packetOut = CreatePacket< PacketServerConnectionInfo_KeepAlive >();
      }
      return true;
    }
   return false;
}

//-----------------------------------------------------------------------------------------

bool     PacketFactory::CreateGatewayWrapper( int packetSubType, BasePacket** packetOut ) const
{
   // I am left wondering whether or not to return the unwrapped packet or not -mkawick

   // we only have one type so no switch is needed.
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper();
   *packetOut = wrapper;
   //return wrapper->SerializeIn();
   return true;
}

//-----------------------------------------------------------------------------------------
