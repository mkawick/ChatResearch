// ClientSideNetworkCallback.h

#include "NetworkWrapper_SupportClasses.h"

namespace Mber
{

///////////////////////////////////////////////////////

class ClientNetworkWrapper;
class NetworkLayer2;
class ClientSideNetworkCallback
{
public:
   virtual void  UserLogin( bool success ) {}
   virtual void  UserLogout() {}
   virtual void  AreWeUsingCorrectNetworkVersion( bool isCorrect ){}
   virtual void  ServerRequestsListOfUserPurchases() {}
   virtual void  UserProfileResponse() {}
   virtual void  OtherUsersProfile( const map< string, string >& profileKeyValues ){} // items are also available
   virtual void  SelfProfileUpdate( bool success ) {}

   virtual void  HasBeenConnectedToGateway() {}
   virtual void  HasBeenDisconnectedFromGateway() {}
   virtual void  HasBeenConnectedToAssetGateway() {}
   virtual void  HasBeenDisconnectedFromAssetGateway() {}

   virtual void  ListOfAvailableProducts() {}
   // this list of purchases will not have localized names for products, especially on other players' products. 
   // make sure to request the list of available products first.
   virtual void  ListOfAggregateUserPurchases() {}

   virtual void  UserDemographics( const string& username, const Demographics& userDemographics ) {}
   virtual void  UserWinLoss( const string& username, const WinLoss& userWinLoss ) {}

   virtual void  GameData( U16 length, const U8* rawdata ) {}
   virtual void  AssetDataAvailable( const string& category, const string& assetHash ) {}

   virtual void  FriendsUpdate() {}
   virtual void  FriendOnlineStatusChanged( const string& uuid ) {}
   virtual void  ChatChannelUpdate( const string& channelUuid ) {}
   virtual void  ChatListUpdate() {}
   virtual void  ListOfFriendUpdate() {}

   virtual void  SearchResults( vector< string >& values ) {}

   virtual void  ReadyToStartSendingRequestsToGame() {}

   virtual void  ChatReceived( const string& message, const string& channelUUID, const string& userUUID, const string& timeStamp, U32 tempId ){}

   virtual void  InvitationReceived( const InvitationInfo& newInvitation ){}
   virtual void  InvitationsReceivedUpdate() {}
   virtual void  InvitationsSentUpdate() {}
   virtual void  InvitationAccepted( const string& sender, const string& receiver, bool wasAccepted ){}
   virtual void  SearchForUserResultsAvailable() {}

   virtual void  ChatChannelHistory( const string& channelUuid, const list< ChatEntry >& listOfChats ) {  }
   virtual void  ChatP2PHistory( const string& userUuid, const list< ChatEntry >& listOfChats ) { }
   virtual void  ChatHistoryMissedSinceLastLoginComposite( const list< MissedChatChannelEntry >& listOfChats ) { }
   
   virtual void  NewChatChannelAdded( const string& channelName, const string& channelUuid, bool success ) { }
   virtual void  ChatChannelDeleted( const string& channelUuid, bool success ) { }

   virtual void  ChatChannel_UserAdded( const string& channelName, const string& channelUuid, const string userName, const string userUuid ) { }
   virtual void  ChatChannel_UserRemoved( const string& channelUuid, const string& userUuid, bool success ) { }
   //virtual void  NewChatChannelAdded( const string& channelName, const string& channelUuid, bool success ) { }

   virtual void  AssetCategoriesLoaded() {}
   virtual void  AssetManifestAvailable( const string& category ) {}
   virtual bool  AssetLoaded( const string& name, const U8* buffer, int size ) { return true; }
   virtual void  TournamentListAvalable() const {}
   virtual void  TournamentPurchaseResult( const string& tournamentUuid, int result ) const {}

   virtual void  OnError( int code, int subCode, const char* text = NULL ){}

   virtual void  PurchaseSuccess( const string& purchaseUuid, bool success ){}
   virtual void  ProductsForSale( const SerializedKeyValueVector< PurchaseInfo >& thingsToBuy ) {}

   virtual void  ListOfDevicesUpdated() const {}

   ClientSideNetworkCallback(): network( NULL ){}

   enum NotificationType
   {
      NotificationType_UserLogin,
      NotificationType_UserLogout,
      NotificationType_AreWeUsingCorrectNetworkVersion,
      NotificationType_ServerRequestsListOfUserPurchases,
      NotificationType_UserProfileResponse,
      NotificationType_OtherUsersProfile,
      NotificationType_SelfProfileUpdate,

      NotificationType_HasBeenConnectedToGateway,
      NotificationType_HasBeenDisconnectedFromGateway,
      NotificationType_AssetHasBeenConnectedToGateway,
      NotificationType_AssetHasBeenDisconnectedToGateway,

      NotificationType_ListOfAvailableProducts,
      NotificationType_ListOfAggregateUserPurchases,

      //NotificationType_UserDemographics,
      //NotificationType_UserWinLoss,

      NotificationType_GameData,
      NotificationType_AssetDataAvailable,

      NotificationType_FriendsUpdate,
      NotificationType_FriendOnlineStatusChanged,
      NotificationType_ChatChannelUpdate,
      NotificationType_ChatListUpdate,

      NotificationType_SearchResults,

      NotificationType_ReadyToStartSendingRequestsToGame,

      NotificationType_ChatReceived,

      NotificationType_InvitationReceived,
      NotificationType_InvitationsReceivedUpdate,
      NotificationType_InvitationsSentUpdate,
      NotificationType_InvitationAccepted,
      NotificationType_SearchForUserResultsAvailable,

      NotificationType_ChatChannelHistory,
      NotificationType_ChatP2PHistory,
      NotificationType_ChatHistoryMissedSinceLastLoginComposite,
      
      NotificationType_NewChatChannelAdded,
      NotificationType_ChatChannelDeleted,

      NotificationType_ChatChannel_UserAdded,
      NotificationType_ChatChannel_UserRemoved,

      NotificationType_AssetCategoriesLoaded,
      NotificationType_AssetManifestAvailable,
      NotificationType_AssetLoaded,
      NotificationType_TournamentListAvalable,
      NotificationType_TournamentPurchaseResult,

      NotificationType_OnError,

      NotificationType_PurchaseSuccess,
      NotificationType_ProductsForSale,
      NotificationType_ListOfDevicesUpdated,
      NotificationType_Num
   };
   ClientNetworkWrapper* network;
};

///////////////////////////////////////////////////////

class ClientSideNetworkCallback2 : public ClientSideNetworkCallback
{
public:
   ClientSideNetworkCallback2() : ClientSideNetworkCallback(), network2( NULL ) {}
   NetworkLayer2* network2;
};

///////////////////////////////////////////////////////

typedef list< ClientSideNetworkCallback* > MBerNotifierList;
typedef list< ClientSideNetworkCallback2* > MBerNotifierList2;

///////////////////////////////////////////////////////

}