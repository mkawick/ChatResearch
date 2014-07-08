#pragma once
// ClientNetworkWrapper.h

#include "../NetworkCommon/ServerConstants.h" // defines game product ids

#include "../NetworkCommon/NetworkOut/Fruitadens.h"
#include "../NetworkCommon/Packets/BasePacket.h"

#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/ContactPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/InvitationPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PurchasePacket.h"
#include "../NetworkCommon/Packets/NotificationPacket.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"

#if defined(ANDROID)
#define __stdcall 
#endif

#include <queue>
#include <map>
using namespace std;

#include "ClientSideNetworkCallback.h"
#include "NetworkWrapper_SupportClasses.h"

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////

namespace Mber
{

///////////////////////////////////////////////////////

class ClientNetworkWrapper  : public PacketHandlerInterface
{
public:
   ClientNetworkWrapper( U8 gameProductId, bool connectToAssetServer = true );
   ~ClientNetworkWrapper();

   void     EnableMultithreadedCallbackSystem();
   void     CheckForReroutes( bool checkForRerouts );
   void     OverrideSocketPort( int port ) { m_loadBalancerPort = port; }
   void     Init( const char* serverDNS = "mber.pub.playdekgames.com" /*"64.183.9.93"*/ );
   bool     RegisterCallbackInterface( ClientSideNetworkCallback* _callbacks );
   bool     NeedsProcessingTime() const;
   
   void     Exit();
   void     UpdateNotifications();

   //--------------------------------------------------------------
   // ********************   Login/Profile   *******************
   bool     RequestLogin( const string& username, const string& password, const string& languageCode );
   bool     RequestAccountCreate( const string& username, const string& useremail, const string& password, int languageId, const string& deviceId, const string& gkHash ); // deviceId could be NULL except in andriod world
   bool     RequestLogout() const;
   bool     ForceLogout();

   bool     IsReadyToLogin() const { return !m_isLoggingIn && !m_isLoggedIn && !m_requiresGatewayDiscovery; }
   bool     IsLoggingIn() const { return m_isLoggingIn; }
   bool     IsLoggedIn() const { return m_isLoggedIn; }   
   bool     IsConnected( bool isMainServer = true ) const;
   string   GetUsername() const { return m_userName; }
   string   GetEmail() const { return m_email; }
   int      GetAvatarId() const { return m_avatarId; }
   string   GetMotto() const { return m_motto; }
   string   GetDeviceUuid() const { return m_thisDeviceUuid; }
   int      GetLanguage() const { return m_languageId; }
   bool     GetShowWinLossRecord() const { return m_showWinLossRecord; }
   bool     GetMarketingOptOut() const { return m_marketingOptOut; }
   bool     GetShowGenderProfile() const { return m_showGenderProfile; }
   bool     GetDisplayOnlineStatusToOtherUsers() const { return m_displayOnlineStatusToOtherUsers; }
   bool     GetBlockContactInvitations() const { return m_blockContactInvitations; }
   bool     GetBlockGroupInvitations() const { return m_blockGroupInvitations; }      

   bool     RequestProfile( const string userName ); //if empty, profile for currently logged in user is used. For other users, you must have admin
   bool     ThrottleConnection( U8 level );
   bool     RequestOtherUserInGameProfile( const string& userName ); // friends, games list, etc
   bool     RequestChatChannelList();

   bool     RequestChangeAvatarId( int newId ) const;
   // for things that don't change, send blank strings and 0s.
   bool     UpdateOwnProfile( const string& userName, const string& email, const string& motto );
   bool     SetLanguage( int languageId );
   bool     SetMarketingOptOut ( bool marketingOptOut = true );
   bool     SetShowWinLossRecord( bool winLossShow = true );
   bool     SetShowGenderProfile( bool showGenderProfile = true );
   bool     SetDisplayOnlineStatusToOtherUsers( bool display = true );
   bool     SetBlockContactInvitations( bool block = true );
   bool     SetBlockGroupInvitations( bool block = true );

   // note that changing a username, email, or uuid is not possible. This is for lookup only.
   // this is meant for other users and only an admin may use it.
   bool     AdminUpdateUserProfile( const string userName, const string& email, const string& userUuid, int adminLevel, int languageId, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile );

   //--------------------------------------------------------------

   //--------------------------------------------------------------
   // ********************   Contacts/Chat     *******************
   bool     RequestListOfContacts() const;
   bool     RequestFriendDemographics( const string& username ) const;
   bool     RequestUserWinLossRecord( const string& username ) const;

   bool     RequestChatChannelHistory( const string& channelUuid, int numRecords = 20, int startingIndex = 0, const char* startingTimestamp = NULL ) const;
   bool     RequestChatP2PHistory( const string& userUuid, int numRecords = 20, int startingIndex = 0, const char* startingTimestamp = NULL  ) const;

   bool     RequestListOfUsersInChatChannel( const string& uuid ); // two packets sent for current members and invitees

   //--------------------------------------------------------------

   void     AddNotationToContact( const string& uuid, bool isFavorite, const string& message ); 
   bool     RemoveContact( const string& uuid, const string message = "" );

   //bool     ChangeGame( const string& gameName );
   bool	   SendP2PTextMessage( const string& message, const string& destinationUserUuid );
   bool	   SendChannelTextMessage( const string& message, const string& chatChannelUuid, U32 gameTurn = 0 );

   //---------------------- invitations ---------------------------

   // contacts
   bool     RequestListOfInvitationsSentForContacts() const;
   bool     RequestListOfInvitationsReceivedForContacts() const;
   bool     AcceptInvitationForContacts( const string& uuid ) const;
   bool     DeclineInvitationForContacts( const string& uuid, string message ) const;
   bool     RemoveSentInvitationForContacts( const string& uuid ) const;

   bool     GetListOfInvitationsReceivedForContacts( list< InvitationInfo >& keyValues );
   bool     GetListOfInvitationsSentForContacts( list< InvitationInfo >& keyValues );
   
   bool     SendSearchForUsers( const string& searchString, int numRequested, int offset ) const; // min 2 char
   bool     InviteUserToBeContact( const string& uuid, const string& username, const string& message );

   // all other types
   bool     RequestListOfInvitations( Invitation::InvitationType = Invitation::InvitationType_ChatRoom ) const;
   bool     AcceptInvitation( const string& uuid, Invitation::InvitationType = Invitation::InvitationType_ChatRoom ) const;
   bool     DeclineInvitation( const string& uuid, string message, Invitation::InvitationType = Invitation::InvitationType_ChatRoom ) const;
   bool     RemoveSentInvitation( const string& uuid, Invitation::InvitationType = Invitation::InvitationType_ChatRoom ) const;

   bool     GetListOfInvitationsReceived( list< Invitation >& invitations, Invitation::InvitationType = Invitation::InvitationType_ChatRoom );
   bool     GetListOfInvitationsSent( list< Invitation >& invitations, Invitation::InvitationType = Invitation::InvitationType_ChatRoom );
   bool     GetListOfGroupInvitations( list< Invitation >& invitations, Invitation::InvitationType = Invitation::InvitationType_ChatRoom );// generic... applies to different groups
   bool     InviteUserToChatChannel( const string& channelUuid, const string& userUuid, const string& message );
   
   bool     RequestListOfMembersInGroup( const string& groupUuid, Invitation::InvitationType = Invitation::InvitationType_ChatRoom ) const;
   bool     RequestListOfInvitationsForGroup( const string& groupUuid, Invitation::InvitationType = Invitation::InvitationType_ChatRoom ) const;

   //--------------------------------------------------------------

   bool     CreateNewChatChannel( const string& channelName );
   bool     RenameChannel( const string& channelUuid, const string& newName );
   bool     AddUserToChannel( const string& userUuid, const string& channelUuid ); // PacketChatAddUserToChatChannel
   //bool     DeleteChannel( const string& channelUuid ); // PacketChatDeleteChatChannel
   bool     LeaveChannel( const string& channelUuid );// remove self from channel .. PacketChatRemoveUserFromChatChannel
   bool     RequestChatChannelInfo( const string& channelUuid );

   //--------------------------------------------------------------
   // ********************   Purchases/Products   *******************
   bool     RequestListOfAppStoreProducts() const; // everything
   bool     RequestListOfMberProducts() const; // just things that you can buy in our store
   bool     RequestListOfPurchases( const string userUuid = "" ) const;
   bool     MakePurchase( const string& exchangeUuid ) const;
   bool     UserHasMadePurchase( const string& vendorProductUuid, const string& receipt, int platformId = Platform_ios  ) const;

   int      GetNumPurchases() const { return m_purchases.size(); }
   bool     GetPurchase( int index, PurchaseEntry& purchase ) const;
   bool     RequestListOfTournaments();
   bool     PurchaseEntryIntoTournament( const string& tournamentUuid, const vector<PurchaseServerDebitItem>& listOfDebitItems, const string& customDeck );

   int      GetNumAvailableProducts() const { return m_products.size(); }
   bool     GetAvailableProduct( int index, ProductBriefPacketed& product ) const; // not complete
   bool     FindProductByVendorUuid( const string& vendorUuid, ProductBriefPacketed& product ) const;
   bool     FindProduct( const string& uuid, ProductBriefPacketed& product ) const;

   //------------------------- asset -------------------------------
   bool     RequestListOfAssetCategories();
   
   bool     RequestListOfAssets( const string& category, int platformId = Platform_ios );
   bool     RequestAssetByHash( const string& assetHash );
   bool     RequestAssetByName( const string& assetName );
   bool     RequestAvatarById( U32 id );

   bool     RegisterDevice( const string& playdekUuid, const string& deviceName, PlatformType platformId, const string& vendorProvidedDeviceId );// platformId = Platform_ios ); 
   bool     RequestListOfDevicesForThisGame( int platformId = Platform_ios );
   bool     ChangeDevice( const string& deviceUuid, const string& deviceNewName, const string& audioFileToPlay, bool isEnabled, int iconId, int repeatFrequencyInHours );
   int      GetNumDevices() const { return m_devices.size(); }
   bool     GetDevice( int index, RegisteredDevice& device ) const;
   bool     RemoveDevice( const string& deviceUuid ) const;

   bool     SendPurchases( const vector< RegisteredProduct >& purchases, int platformId = Platform_ios ); 
   bool     GiveProduct( const string& userName, const string& productUuid, int quantity, const string& notes, int platformId = Platform_ios );
   bool     SendCheat( const string& cheat );

   //--------------------------------------------------------------

   bool     SendRawPacket( const char* buffer, int length ) const;

   //--------------------------------------------------------------
   // utility functions

   U32      FindGame( const string& name ) const;
   string   FindGameNameFromGameId( U32 id ) const;
   string   FindContact( const string& name ) const;
   string   FindContactFromUuid( const string& uuid ) const;

   int      GetNumContacts() const { return static_cast<int>( m_contacts.size() ); }
   bool     GetContact( int index, BasicUser& user );
   bool     GetContact( const string& uuid, BasicUser& user );
   bool     IsContact( const string& userUuid );
   bool     IsContactByName( const string& userName );

   int      GetNumUserSearchResults() const { return m_lastUserSearch.size(); }
   bool     GetUserSearchResult( int index, BasicUser& user );

   int      GetNumChannels() const { return static_cast<int>( m_channels.size() ); }
   bool     GetChannel( const string& uuid, ChatChannel& channel ) const;
   bool     GetChannel( int index, ChatChannel& channel ) const;
   bool     IsGameChannel( const string& channelUuid ) const;

   int      GetAssetCategories( vector< string >& categories ) const;
   int      GetNumAssets( const string& category );
   bool     GetAssetInfo( const string& category, int index, AssetInfoExtended& assetInfo );

   bool     GetAssetInfo( const string& category, const string& hash, AssetInfoExtended& asset ) { return GetAsset( category, hash, asset ); }
   bool     GetAvatarAsset( U32 index, AssetInfoExtended& assetInfo );
   bool     ClearAssetInfo( const string& category, const string& hash );

   int      GetNumAvailableTournaments() const { return static_cast<int>( m_availableTournaments.size() ); }
   bool     GetTournamentInfo( int index, TournamentInfo& tournamentInfo );

   string   GetLocalUUID() { return m_uuid; }

   string   GenerateHash( const string& stringThatIWantHashed );
   void     FillProfileChangeRequest( PacketUpdateSelfProfile& profile ) const;

   void     SendNotifications();
   
protected:

   void     InputConnected( IChainedInterface * ) {}
   void     OutputConnected( IChainedInterface * ) {}
   void     InputRemovalInProgress( IChainedInterface * ) {}
   void     OutputRemovalInProgress( IChainedInterface * ) {}

   // datatypes
   bool     Log( const std::string& text, int priority = 1 ) const { return true; }
   bool     Log( const char* text, int priority = 1 ) const { return true; }
   
   typedef SerializedKeyValueVector< BasicUser >   UserNameKeyValue;
   typedef vector< PacketGameIdentification >      GameList;


   //----------------------------------
   enum     ConnectionNames
   {
      ConnectionNames_Main,
      ConnectionNames_Asset,
      ConnectionNames_Num
   };
   Fruitadens* m_fruitadens[ ConnectionNames_Num ];
   int                                       m_normalSleepTime, m_boostedSleepTime;
   bool                                      m_isThreadPerformanceBoosted[ ConnectionNames_Num ];
   time_t                                    m_timeWhenThreadPerformanceBoosted[ ConnectionNames_Num ];
   //----------------------------------

   string                                    m_userName, m_attemptedUsername;
   string                                    m_email;
   string                                    m_uuid;
   string                                    m_serverIpAddress[ ConnectionNames_Num ];
   U16                                       m_serverConnectionPort[ ConnectionNames_Num ];
   string                                    m_loadBalancerDns;
   string                                    m_loginKey;
   string                                    m_motto;
   string                                    m_thisDeviceUuid;
   U32                                       m_selectedGame;
   int                                       m_avatarId;
   int                                       m_languageId;

   bool                                      m_showWinLossRecord;
   bool                                      m_marketingOptOut;
   bool                                      m_showGenderProfile;
   bool                                      m_displayOnlineStatusToOtherUsers;
   bool                                      m_blockContactInvitations;
   bool                                      m_blockGroupInvitations;

   SerializedKeyValueVector< InvitationInfo >   m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >   m_invitationsSent;
   list< Invitation >                           m_invitations[ Invitation::InvitationType_Num ];
   list< Invitation >                           m_invitationsToGroup[ Invitation::InvitationType_Num ];
   SerializedVector< ProductBriefPacketed >     m_products;
   SerializedVector< RegisteredDevice >         m_devices;
   UserNameKeyValue                             m_contacts;
   UserNameKeyValue                             m_lastUserSearch;
   ChatChannelVector                            m_channels;
   GameList                                     m_gameList;

   SerializedVector< PurchaseEntry >            m_purchases;

   U8                                        m_gameProductId;
   bool                                      m_connectToAssetServer;
   bool                                      m_isLoggingIn;
   bool                                      m_isLoggedIn;
   bool                                      m_isCreatingAccount;
   U32                                       m_connectionId;
   string                                    m_lastLoggedOutTime;
   int                                       m_lastRawDataIndex;
   int                                       m_loadBalancerPort;

   mutable U32                               m_beginTime, m_endTime;
   bool                                      m_wasCallbackForReadyToBeginSent;
   bool                                      m_requiresGatewayDiscovery;   

   MBerNotifierList                          m_callbacks;
   RawDataAccumulator                        m_rawDataBuffer;

   AssetMap                                  m_assets;
   vector< TournamentInfo >                  m_availableTournaments;

   //----------------------------------
   bool                                      m_enabledMultithreadedProtections;
   struct QueuedNotification
   {
      QueuedNotification() : eventId( 0 ),  intValue( 0 ), intValue2( 0 ), packet( NULL ), genericData( NULL )  {}
      QueuedNotification( int eid, BasePacket* data = NULL ) : eventId( eid ), intValue( 0 ), intValue2( 0 ), packet( data ), genericData( NULL ) {}
      int                                 eventId;
      int                                 intValue;
      int                                 intValue2;
      BasePacket*                         packet;
      U8*                                 genericData;
      SerializedKeyValueVector< string >  genericKeyValuePairs;
   };
   Threading::Mutex                          m_notificationMutex;
   std::queue< QueuedNotification >          m_notifications;

   void     Notification( int type );
   void     Notification( int type, U8* genericData, int size );
   void     Notification( int type, BasePacket* packet );
   void     Notification( int type, int data, int meta = 0 );
   void     Notification( int type, const string& data );
   void     Notification( int type, const string& data, const string& data2 );
   void     Notification( int type, SerializedKeyValueVector< string >& strings );

   //----------------------------------

   // only for dealing with the fact that the game will often login well before the 
   // server has connected to the client.
   PacketLogin*                              m_savedLoginInfo;

protected:
   void     ReconnectAfterTalkingToLoadBalancer();
   void     Disconnect();

   vector< ChatChannel >::iterator    GetChannel( const string& channelUuid );
   void     CleanupLingeringMemory();
   bool     SerializePacketOut( BasePacket* packet ) const;// helper
   
   //void     HasBeenConnectedCallback();
   //void     HasBeenDisconnectedCallback();

   bool     HandlePacketReceived( BasePacket* packetIn );
   void     InheritedUpdate();
   void     HandleChatChannelUpdate( BasePacket* packetIn );

   void     Update();
   void     HandleAssetData( PacketGameplayRawData* data );
   void     HandleData( PacketGameplayRawData* );

   void     LoadBalancedNewAddress              ( const PacketRerouteRequestResponse* response );
   void     HandleListOfContacts                ( const PacketContact_GetListOfContactsResponse* packet );
   void     HandleUserOnlineStatusChange        ( const PacketContact_FriendOnlineStatusChange* packet );
   void     HandleListOfReceivedInvitations     ( const PacketContact_GetListOfInvitationsResponse* packet );
   void     HandleListOfSentInvitations         ( const PacketContact_GetListOfInvitationsSentResponse* packet );
   void     HandleInvitationReceived            ( const PacketContact_InviteReceivedNotification* packet );
   void     HandleInvitationAccepted            ( const PacketContact_InvitationAccepted* packet );
   void     HandleSearchForUserResults          ( const PacketContact_SearchForUserResult* packet );
   void     HandleListOfAggregatePurchases      ( const PacketListOfUserAggregatePurchases* packet );
   void     HandleListOfProducts                ( const PacketRequestListOfProductsResponse* packet );
   void     HandleBeingAddedByServerToChatChannel( const PacketChatUserAddedToChatChannelFromGameServer* packet );
   void     HandleUserChatStatusChange          ( const PacketChatUserStatusChangeBase* packet );
   void     HandleAddUserToChatChannel          ( const PacketChatAddUserToChatChannelResponse* packet );
   void     HandleRemoveUserFromChatChannel     ( const PacketChatRemoveUserFromChatChannelResponse* packet );
   void     HandleListOfAssetCategoriesUpdate   ( const PacketAsset_GetListOfAssetCategoriesResponse* packet );
   void     HandleListOfAssets                  ( const PacketAsset_GetListOfAssetsResponse* packet );
   void     HandleListOfDevices                 ( const PacketNotification_RequestListOfDevicesResponse* packet );


   bool     AddChatChannel( const ChatChannel& channel );
   //bool     RemoveChatChannel( const string& channelUuid );
   bool     AddUserToChatChannel( const string& channelUuid, const string& userUuid, const string& userName );
   bool     RemoveUserfromChatChannel( const string& channelUuid, const string& userUuid );
   bool     RemoveChannel( const string& uuid );

   bool     RemoveInvitationFromSent( const string& uuid );
   bool     RemoveInvitationFromReceived( const string& uuid );

   void     BoostThreadPerformance( ConnectionNames whichConnection = ConnectionNames_Asset );
   void     RestoreNormalThreadPerformance( ConnectionNames whichConnection = ConnectionNames_Asset );
   void     ExpireThreadPerformanceBoost( ConnectionNames whichConnection = ConnectionNames_Asset );

   bool     GetAsset( const string& category, const string& hash, AssetInfoExtended& asset );
   bool     GetAsset( const string& hash, AssetInfoExtended& asset );
   AssetInfoExtended*   GetAsset( const string& hash );
   bool     UpdateAssetData( const string& hash, AssetInfoExtended& asset );

   void     NotifyClientToBeginSendingRequests();
   //void     InitalConnectionCallback();

   bool     InitialConnectionCallback( const Fruitadens* connectionObject );
   bool     InitialDisconnectionCallback( const Fruitadens* connectionObject );
};

///////////////////////////////////////////////////////

class ClientSideNetworkCallbackExtended: public ClientSideNetworkCallback
{
public:
   void  SetStartTime();

   virtual void AssetEcho() {}
   virtual void ChatEcho(){}
   virtual void ContactEcho() {}
   virtual void LoginEcho() {}
   virtual void GameEcho() {}
   virtual void PurchaseEcho() {}

   unsigned long   m_timeSent;
   unsigned long   m_timeTaken;
};

class NetworkLayerExtended: public ClientNetworkWrapper
{
public:
   NetworkLayerExtended( U8 gameProductId, bool processOnlyOneIncommingPacketPerLoop = false ) : ClientNetworkWrapper( gameProductId, processOnlyOneIncommingPacketPerLoop ){}
   
   bool  HandlePacketReceived( BasePacket* packetIn );
   void  SendAssetEcho();
   void  SendChatEcho();
   void  SendContactEcho();
   void  SendLoginEcho();
   void  SendGameEcho();
   void  SendPurchaseEcho();
   void  SendNotification( U8 type, string additionalText = "" );

protected:
   void  StartTime();
};

///////////////////////////////////////////////////////

} // namespace Mber

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////