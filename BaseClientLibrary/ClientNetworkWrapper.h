#pragma once
// ClientNetworkWrapper.h

#include "../NetworkCommon/ServerConstants.h" // defines game product ids

#include "../NetworkCommon/NetworkOut/Fruitadens.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"

#include "../NetworkCommon/Packets/ContactPacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
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


namespace Mber
{



///////////////////////////////////////////////////////

class ClientNetworkWrapper :  public Fruitadens
{
public:
   ClientNetworkWrapper( U8 gameProductId, bool processOnlyOneIncommingPacketPerLoop = false );
   ~ClientNetworkWrapper();

   void     EnableMultithreadedCallbackSystem();
   void     CheckForReroutes( bool checkForRerouts );
   void     OverrideSocketPort( int port ) { m_connectionPort = port; }
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

   bool     IsReadyToLogin() const { return !m_isLoggingIn & !m_isLoggedIn; }
   bool     IsLoggingIn() const { return m_isLoggingIn; }
   bool     IsLoggedIn() const { return m_isLoggedIn; }   
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
   // ********************   Friends/Chat     *******************
   bool     RequestListOfFriends() const;
   bool     RequestFriendDemographics( const string& username ) const;
   bool     RequestUserWinLossRecord( const string& username ) const;

   bool     RequestListOfInvitationsSent() const;
   bool     RequestListOfInvitationsReceived() const;

   bool     RequestChatChannelHistory( const string& channelUuid, int numRecords = 20, int startingIndex = 0 ) const;
   bool     RequestChatP2PHistory( const string& userUuid, int numRecords = 20, int startingIndex = 0 ) const;

   //--------------------------------------------------------------

   bool     AcceptInvitation( const string& uuid ) const;
   bool     AcceptInvitationFromUsername( const string& userName ) const;
   bool     DeclineInvitation( const string& uuid, string message ) const;
   bool     RemoveSentInvitation( const string& uuid ) const;
   bool     GetListOfInvitationsReceived( list< InvitationInfo >& keyValues );
   bool     GetListOfInvitationsSent( list< InvitationInfo >& keyValues );

   bool     SendSearchForUsers( const string& searchString, int numRequested, int offset ) const; // min 2 char
   bool     InviteUserToBeFriend( const string& uuid, const string& username, const string& message );
   bool     RemoveContact( const string& uuid, const string message = "" );

   //bool     ChangeGame( const string& gameName );
   bool	   SendP2PTextMessage( const string& message, const string& destinationUserUuid );
   bool	   SendChannelTextMessage( const string& message, const string& chatChannelUuid, U32 gameTurn = 0 );

   //--------------------------------------------------------------

   bool     CreateNewChatChannel( const string& channelName );
   bool     RenameChannel( const string& channelUuid, const string& newName );
   bool     AddUserToChannel( const string& userUuid, const string& channelUuid ); // PacketChatAddUserToChatChannel
   //bool     DeleteChannel( const string& channelUuid ); // PacketChatDeleteChatChannel
   bool     LeaveChannel( const string& channelUuid );// remove self from channel .. PacketChatRemoveUserFromChatChannel

   //--------------------------------------------------------------
   // ********************   Purchases/Products   *******************
   bool     RequestListOfProducts() const; // everything
   bool     RequestListOfProductsInStore() const; // just things that you can buy in our store
   bool     RequestListOfPurchases( const string userUuid = "" ) const;
   bool     MakePurchase( const string& exchangeUuid ) const;

   int      GetNumPurchases() const { return m_purchases.size(); }
   bool     GetPurchase( int index, PurchaseEntry& purchase ) const;
   bool     RequestListOfTournaments();
   bool     PurchaseEntryIntoTournament( const string& tournamentUuid );

   int      GetNumAvailableProducts() const { return m_products.size(); }
   int      GetAvailableProduct( int index, ProductBriefPacketed& purchase ) const; // not complete

   bool     RequestListOfAssetCategories();
   
   bool     RequestListOfAssets( const string& category, int platformId = Platform_ios );
   bool     RequestAssetByHash( const string& assetHash );
   bool     RequestAssetByName( const string& assetName );
   bool     RequestAvatarById( U32 id );

   bool     RegisterDevice( const string& deviceId, const string& deviceName, int platformId = Platform_ios ); // int gameProductId = GameProductId_SUMMONWAR, 
   bool     RequestListOfDevicesForThisGame( int platformId = Platform_ios );
   bool     ChangeDevice( const string& deviceUuid, const string& deviceNewName, bool isEnabled, int iconId );
   int      GetNumDevices() const { return m_devices.size(); }
   bool     GetDevice( int index, RegisteredDevice& device ) const;

   bool     SendPurchases( const vector< RegisteredProduct >& purchases, int platformId = Platform_ios ); 
   bool     GiveProduct( const string& userName, const string& productUuid, int quantity, const string& notes, int platformId = Platform_ios );
   bool     SendCheat( const string& cheat );

   //--------------------------------------------------------------

   bool     SendRawPacket( const char* buffer, int length ) const;

   //--------------------------------------------------------------
   // utility functions

   U32      FindGame( const string& name ) const;
   string   FindGameNameFromGameId( U32 id ) const;
   string   FindFriend( const string& name ) const;
   string   FindFriendFromUuid( const string& uuid ) const;

   int      GetNumFriends() const { return static_cast<int>( m_friends.size() ); }
   bool     GetFriend( int index, BasicUser& user );
   bool     IsFriend( const string& userUuid );
   bool     IsFriendByName( const string& userName );

   int      GetNumUserSearchResults() const { return m_lastUserSearch.size(); }
   bool     GetUserSearchResult( int index, BasicUser& user );

   int      GetNumChannels() const { return static_cast<int>( m_channels.size() ); }
   bool     GetChannel( const string& uuid, ChatChannel& channel ) const;
   bool     GetChannel( int index, ChatChannel& channel ) const;
   bool     RemoveChannel( const string& uuid );
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


   string                                    m_userName, m_attemptedUsername;
   string                                    m_email;
   string                                    m_uuid;
   string                                    m_serverDns;
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
   SerializedVector< ProductBriefPacketed >     m_products;
   SerializedVector< RegisteredDevice >         m_devices;
   UserNameKeyValue                             m_friends;
   UserNameKeyValue                             m_lastUserSearch;
   ChatChannelVector                            m_channels;
   GameList                                     m_gameList;

   SerializedVector< PurchaseEntry >            m_purchases;

   U8                                        m_gameProductId;
   bool                                      m_isLoggingIn;
   bool                                      m_isLoggedIn;
   bool                                      m_isCreatingAccount;
   U32                                       m_connectionId;
   string                                    m_lastLoggedOutTime;
   int                                       m_lastRawDataIndex;
   int                                       m_connectionPort;

   mutable U32                               m_beginTime, m_endTime;

   int                                       m_normalSleepTime, m_boostedSleepTime;
   bool                                      m_isThreadPerformanceBoosted;
   time_t                                    m_timeWhenThreadPerformanceBoosted;

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
   
   vector< ChatChannel >::iterator    GetChannel( const string& channelUuid );
   void     CleanupLingeringMemory();
   bool     SerializePacketOut( BasePacket* packet ) const;// helper
   
   void     HasBeenConnectedCallback();
   void     HasBeenDisconnectedCallback();

   bool     HandlePacketReceived( BasePacket* packetIn );
   void     InheritedUpdate();
   void     HandleChatChannelUpdate( BasePacket* packetIn );

   int      MainLoop_InputProcessing();
   void     HandleAssetData( PacketGameplayRawData* data );
   void     HandleData( PacketGameplayRawData* );

   bool     AddChatChannel( const ChatChannel& channel );
   bool     RemoveChatChannel( const string& channelUuid );
   bool     AddUserToChatChannel( const string& channelUuid, const string& userUuid, const string& userName );
   bool     RemoveUserfromChatChannel( const string& channelUuid, const string& userUuid );

   bool     RemoveInvitationFromSent( const string& uuid );
   bool     RemoveInvitationFromReceived( const string& uuid );

   void     BoostThreadPerformance();
   void     RestoreNormalThreadPerformance();
   void     ExpireThreadPerformanceBoost();

   bool     GetAsset( const string& category, const string& hash, AssetInfoExtended& asset );
   bool     GetAsset( const string& hash, AssetInfoExtended& asset );
   AssetInfoExtended*   GetAsset( const string& hash );
   bool     UpdateAssetData( const string& hash, AssetInfoExtended& asset );

   void     NotifyClientToBeginSendingRequests();
   void     InitalConnectionCallback();
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

class NetworkLayer2 : public PacketHandlerInterface
{
public:
   NetworkLayer2( U8 gameProductId, bool isTestingOnly );
   ~NetworkLayer2();

   //void     CheckForReroutes( bool checkForRerouts );
   bool     RegisterCallbackInterface( ClientSideNetworkCallback2* _callbacks );

   void     Exit();

   //--------------------------------------------------------------
   // ********************   Login/Profile   *******************
   bool     RequestLogin( const string& username, const string& password, const string& languageCode );
   bool     RequestAccountCreate( const string& username, const string& useremail, const string& password, int languageId, const string& deviceId, const string& gkHash ); // deviceId could be NULL except in andriod world
   bool     RequestLogout() const;

   bool     IsReadyToLogin() const;
   bool     IsLoggingIn() const { return m_isLoggingIn; }
   bool     IsLoggedIn() const { return m_isLoggedIn; }   
   string   GetUsername() const { return m_userName; }

   bool     RequestProfile( const string userName ); //if empty, profile for currently logged in user is used. For other users, you must have admin
   bool     RequestOtherUserInGameProfile( const string& userName ); // friends, games list, etc

   // note that changing a username, email, or uuid is not possible. This is for lookup only.
   bool     UpdateUserProfile( const string userName, const string& email, const string& userUuid, int adminLevel, int languageId, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile );

   void     Update();
   //--------------------------------------------------------------

   //--------------------------------------------------------------
   // ********************   Friends/Chat     *******************
   bool     RequestListOfFriends() const;
   //bool     RequestListOfGames() const;
   bool     RequestFriendDemographics( const string& username ) const;
   bool     RequestUserWinLossRecord( const string& username ) const;

   bool     RequestListOfInvitationsSent() const;
   bool     RequestListOfInvitationsReceived() const;

   bool     RequestChatChannelHistory( const string& channelUuid, int numRecords = 20, int startingIndex = 0 ) const;
   bool     RequestChatP2PHistory( const string& userUuid, int numRecords = 20, int startingIndex = 0 ) const;

   //--------------------------------------------------------------

   bool     AcceptInvitation( const string& uuid ) const;
   bool     AcceptInvitationFromUsername( const string& userName ) const;
   bool     DeclineInvitation( const string& uuid, string message ) const;
   bool     GetListOfInvitationsReceived( list< InvitationInfo >& keyValues );
   bool     GetListOfInvitationsSent( list< InvitationInfo >& keyValues );

   bool     SendSearchForUsers( const string& searchString, int numRequested, int offset ) const; // min 2 char
   bool     InviteUserToBeFriend( const string& uuid, const string& username, const string& message );

   //bool     ChangeGame( const string& gameName );
   bool	   SendP2PTextMessage( const string& message, const string& destinationUserUuid );
   bool	   SendChannelTextMessage( const string& message, const string& chatChannelUuid, U32 gameTurn = 0 );

   //--------------------------------------------------------------
   // ********************   Purchases/Products   *******************
   bool     RequestListOfProducts() const; // everything
   bool     RequestListOfProductsInStore() const; // just things that you can buy in our store
   bool     RequestListOfPurchases( const string userUuid = "" ) const;
   bool     MakePurchase( const string& exchangeUuid ) const;

   bool     RequestListOfTournaments();
   bool     PurchaseEntryIntoTournament( const string& tournamentUuid );

   bool     RequestListOfStaticAssets( int platformId = Platform_ios );
   bool     RequestListOfDynamicAssets( int platformId = Platform_ios );
   bool     RequestAsset( const string& assetName );

   bool     SendPurchases( const vector< RegisteredProduct >& purchases, int platformId = Platform_ios );
   bool     GiveProduct( const string& userName, const string& productUuid, int quantity, const string& notes, int platformId = Platform_ios );
   bool     SendCheat( const string& cheat );

   //--------------------------------------------------------------

   bool     SendRawPacket( const char* buffer, int length ) const;

   //--------------------------------------------------------------
   // utility functions

   U32      FindGame( const string& name ) const;
   string   FindGameNameFromGameId( U32 id ) const;
   string   FindFriend( const string& name ) const;
   string   FindFriendFromUuid( const string& uuid ) const;

   int      GetNumFriends() const { return m_friends.size(); }
   bool     GetFriend( int index, BasicUser& user );

   int      GetNumChannels() const { return static_cast<int>( m_channels.size() ); }
   bool     GetChannel( int index, ChatChannel& channel );

   int      GetNumStaticAssets() const { return static_cast<int>( m_staticAssets.size() ); }
   bool     GetStaticAssetInfo( int index, AssetInfoExtended& assetInfo );

   int      GetNumDynamicAssets() const { return static_cast<int>( m_dynamicAssets.size() ); }
   bool     GetDynamicAssetInfo( int index, AssetInfoExtended& assetInfo );

   bool     GetAssetInfo( const string& hash, AssetInfoExtended& asset ) { return GetAsset( hash, asset ); }
   bool     ClearAssetInfo( const string& hash );

   int      GetNumAvailableTournaments() const { return static_cast<int>( m_availableTournaments.size() ); }
   bool     GetTournamentInfo( int index, TournamentInfo& tournamentInfo );

   string   GetLocalUUID() { return m_uuid; }

   string   GenerateHash( const string& stringThatIWantHashed );
   
private:

   enum     ConnectionNames
   {
      ConnectionNames_Main,
      //ConnectionNames_Asset,
      ConnectionNames_Num
   };
   Fruitadens* m_fruitadens[ ConnectionNames_Num ];

   void     ReconnectMain();
   void     Disconnect();
   bool     IsConnected() const;

   // datatypes
   bool     Log( const std::string& text, int priority = 1 ) const { return true; }
   bool     Log( const char* text, int priority = 1 ) const { return true; }
   
   typedef SerializedKeyValueVector< BasicUser >   UserNameKeyValue;
   typedef vector< PacketGameIdentification >      GameList;


   string                           m_userName, m_attemptedUsername;
   string                           m_uuid;
   string                           m_serverDns;
   string                           m_loginKey;
   U32                              m_selectedGame;

   SerializedKeyValueVector< InvitationInfo > m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo > m_invitationsSent;
   UserNameKeyValue                 m_friends;
   ChatChannelVector                m_channels;
   GameList                         m_gameList;

   U8                               m_gameProductId;
   bool                             m_requiresGatewayDiscovery;
   bool                             m_isLoggingIn;
   bool                             m_isLoggedIn;
   bool                             m_isCreatingAccount;
   U32                              m_connectionId;
   string                           m_lastLoggedOutTime;
   int                              m_lastRawDataIndex;
   int                              m_connectionPort;

   mutable U32                      m_beginTime, m_endTime;

   int                              m_normalSleepTime, m_boostedSleepTime;
   bool                             m_isThreadPerformanceBoosted;
   time_t                           m_timeWhenThreadPerformanceBoosted;

   MBerNotifierList2                m_callbacks;
   RawDataAccumulator               m_rawDataBuffer[ PacketGameplayRawData::NumDataTypes ];

   vector< AssetInfoExtended >      m_staticAssets;
   vector< AssetInfoExtended >      m_dynamicAssets;
   vector< TournamentInfo >         m_availableTournaments;

private:
   bool     SerializePacketOut( BasePacket* packet ) const;// helper

   bool     HandlePacketReceived( BasePacket* packetIn );

   bool     AddChatChannel( const ChatChannel& channel );

   void     BoostThreadPerformance();
   void     RestoreNormalThreadPerformance();
   void     ExpireThreadPerformanceBoost();

   bool     GetAsset( const string& hash, AssetInfoExtended& asset );
   bool     UpdateAssetData( const string& hash, AssetInfoExtended& asset );

   void     NotifyClientLoginStatus( bool isLoggedIn );
   void     NotifyClientToBeginSendingRequests();

   void     LoadBalancedNewAddress( const PacketRerouteRequestResponse* packet );
};

///////////////////////////////////////////////////////

}
