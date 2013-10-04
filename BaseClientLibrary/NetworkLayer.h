#pragma once
#include "../NetworkCommon/NetworkOut/Fruitadens.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"

#include "../NetworkCommon/Packets/ContactPacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"

namespace Mber
{

   typedef void (__stdcall * SignalReady)(int);
///////////////////////////////////////////////////////

struct Demographics
{
   bool     isPrivate;// if true, most of the rest of this info is invalid.
   string   username;
   string   email;
   time_t   lastLoginTime;
   // ...
   list< string > listOfGamesOwned;
   int      gender;
   int      avatar;
   string   location;
   int      timeZoneGmt;

};

struct RegisteredProduct
{
   string   id; 
   string   title; 
   string   description; 
   string   price; 
   string   localized_price; 
   double   number_price;
   float    quantity;
};

//-------------------------------------------

class BasicUser
{
public:
   string    userName;
   //tring    userEmail;
   string    UUID;
   bool      isOnline;
};

//-------------------------------------------

class ChatChannel
{
public:
   ChatChannel(): gameInstanceId( 0 ) {}
   string   channelName;
   string   channelDetails;
   string   uuid;
   U8       gameProductId;
   U32      gameInstanceId;

   SerializedKeyValueVector< string >   userList; // not necessarily friends

   void  Clear()
   {
      channelName.clear();
      channelDetails.clear();
      uuid.clear();
      gameProductId = 0;
      gameInstanceId = 0;
      userList.clear();
   }
};

//-------------------------------------------

typedef vector< ChatChannel >    ChatChannelVector;

class Group
{
   string groupName;
   string groupMotto;
   int    avatarId;
   string chatChannel; // the venn diagrap for groups and channels may not be 1-1
   vector< BasicUser > usersInGroup;
};

struct AssetInfoExtended : public AssetInfo
{
   U8*   data;
   int   size;

   AssetInfoExtended();
   AssetInfoExtended( const AssetInfo& asset );
   ~AssetInfoExtended();

   void  operator = ( const AssetInfo& asset );
   void  operator = ( const AssetInfoExtended& asset );

   void  SetData( const U8* data, int size );
   void  ClearData();
   bool  IsDataValid() const { if( data && size ) return true; return false; }
   void  MoveData( AssetInfoExtended& source );

   bool  SerializeIn( const U8* data, int& bufferOffset ) { return false; } // do not serialize
   bool  SerializeOut( U8* data, int& bufferOffset ) const { return false; }

private:
   

};
/*

const char* productNames [] = {
   "",
   "ascension",
   "dominion",
   "thunderstone",
   "wowcmg",
   "summonwar",
   "foodfight",
   "nightfall",
   "pennyarcade",
   "infinitecity",
   "agricola",
   "fluxx",
   "smashup"
};

const char* platformStrings[] = {
   "",
   "ios",
   "android",
   "pc",
   "mac",
   "blackberry"
};*/

///////////////////////////////////////////////////////

class NetworkLayer;
class UserNetworkEventNotifier
{
public:
   virtual void  UserLogin( bool success ) {}
   virtual void  UserLogout() {}
   virtual void  RequestListOfUserPurchases() {}
   virtual void  UserProfileResponse( string username, string email, string userUuid, string lastLoginTime, string loggedOutTime, int adminLevel, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile ) {}

   virtual void  ListOfAvailableProducts( const SerializedVector< ProductBriefPacketed >& products, int platformId ) {}
   virtual void  ListOfAggregateUserPurchases( const SerializedVector< PurchaseEntry >& purchases, int platformId ) {}

   virtual void  UserDemographics( const string& username, const Demographics& userDemographics ) {}
   virtual void  UserWinLoss( const string& username, const WinLoss& userWinLoss ) {}

   virtual void  GameData( U16 length, const U8* rawdata ) {}
   virtual void  AssetDataAvailable( const string& assetHash ) {}

   virtual void  FriendsUpdate() {}
   virtual void  FriendOnlineStatusChanged( const string& uuid ) {}
   virtual void  ChatChannelUpdate( const string& uuid ) {}
   virtual void  ListOfFriendUpdate() {}

   virtual void  SearchResults( vector< string >& values ) {}

   virtual void  ReadyToStartSendingRequestsToGame() {}

   virtual void  ChatReceived( const string& message, const string& channelUUID, const string& userUUID, const string& timeStamp ){}
   virtual void  AssetReceived( const U8* buffer, int size, const string& assetId ){}

   virtual void  InvitationReceived( const InvitationInfo& newInvitation ){}
   virtual void  InvitationsReceivedUpdate() {}
   virtual void  InvitationsSentUpdate() {}
   virtual void  InvitationAccepted( const string& sender, const string& receiver, bool wasAccepted ){}

   virtual void  ChatChannelHistory( const string& channelUuid, const list< ChatEntry >& listOfChats ) {  }
   virtual void  ChatP2PHistory( const string& userUuid, const list< ChatEntry >& listOfChats ) { }

   virtual void  StaticAssetManifestAvalable() const {}
   virtual void  DynamicicAssetManifestAvalable() const {}
   virtual bool  AssetLoaded( const string& name, const U8* buffer, int size ) { return true; }

   virtual void  OnError( int code, int subCode ){}

   UserNetworkEventNotifier(){}

   NetworkLayer* network;
};

///////////////////////////////////////////////////////

typedef list< UserNetworkEventNotifier* > MBerNotifierList;

///////////////////////////////////////////////////////

class RawDataAccumulator
{
public:
   RawDataAccumulator() : numBytes( 0 ) {}

   void  AddPacket( PacketGameplayRawData * );
   bool  IsDone();
   void  PrepPackage( U8* & data, int& size );
   void  PrepPackage( AssetInfoExtended& asset );

   int   GetRemainingSize() { 
      if( packetsOfData.size() > 1 )
      {
         return (*packetsOfData.begin())->index * PacketGameplayRawData::MaxBufferSize;// only an estimate
      }
      return 0;
   }

   int   numBytes;
   deque< PacketGameplayRawData* > packetsOfData;
};

///////////////////////////////////////////////////////

class NetworkLayer :  public Fruitadens
{
public:
   NetworkLayer( U8 gameProductId, bool processOnlyOneIncommingPacketPerLoop = false );
   ~NetworkLayer();

   void     Init( const char* serverDNS = "gateway.internal.playdekgames.com" );
   bool     RegisterCallbackInterface( UserNetworkEventNotifier* _callbacks );

   void     Exit();

   //--------------------------------------------------------------
   // ********************   Login/Profile   *******************
   bool     RequestLogin( const string& username, const string& password, const string& languageCode );
   bool     RequestAccountCreate( const string& username, const string& useremail, const string& password, int languageId, const string& deviceId, const string& gkHash ); // deviceId could be NULL except in andriod world
   bool     RequestLogout() const;

   bool     IsLoggingIn() const { return m_isLoggingIn; }
   bool     IsLoggedIn() const { return m_isLoggedIn; }   
   string   GetUsername() const { return m_userName; }

   bool     RequestProfile( const string userName ); //if empty, profile for currently logged in user is used.

   // note that changing a username, email, or uuid is not possible. This is for lookup only.
   bool     UpdateUserProfile( const string userName, const string& email, const string& userUuid, int adminLevel, int languageId, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile );

   //--------------------------------------------------------------

   //--------------------------------------------------------------
   // ********************   Friends/Chat     *******************
   bool     RequestListOfFriends() const;
   bool     RequestListOfGames() const;
   bool     RequestListOfUsersLoggedIntoGame( ) const;
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
   bool     RequestListOfProducts() const;
   bool     RequestListOfPurchases( const string userUuid = "" ) const;
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
   bool     GetFriend( int index, const BasicUser*& user );

   int      GetNumChannels() const { return m_channels.size(); }
   bool     GetChannel( int index, ChatChannel& channel );

   int      GetNumStaticAssets() const { return m_staticAssets.size(); }
   bool     GetStaticAssetInfo( int index, AssetInfoExtended& assetInfo );

   int      GetNumDynamicAssets() const { return m_dynamicAssets.size(); }
   bool     GetDynamicAssetInfo( int index, AssetInfoExtended& assetInfo );

   bool     GetAssetInfo( const string& hash, AssetInfoExtended& asset ) { return GetAsset( hash, asset ); }
   bool     ClearAssetInfo( const string& hash );

   string   GetLocalUUID() { return m_uuid; }

   string   GenerateHash( const string& stringThatIWantHashed );
   
private:

   void   InputConnected( IChainedInterface * ) {}
   void   OutputConnected( IChainedInterface * ) {}
   void   InputRemovalInProgress( IChainedInterface * ) {}
   void   OutputRemovalInProgress( IChainedInterface * ) {}

   // datatypes
   bool  Log( const std::string& text, int priority = 1 ) const { return true; }
   bool  Log( const char* text, int priority = 1 ) const { return true; }
   
   typedef SerializedKeyValueVector< BasicUser >   UserNameKeyValue;
   typedef vector< PacketGameIdentification >      GameList;


   string            m_userName, m_attemptedUsername;
   string            m_uuid;
   string            m_serverDns;
   string            m_loginKey;
   U32               m_selectedGame;

   SerializedKeyValueVector< InvitationInfo > m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo > m_invitationsSent;
   UserNameKeyValue  m_friends;
   ChatChannelVector m_channels;
   //vector< Group >         Groups;
   //KeyValueVector    m_availableGames;
   GameList          m_gameList;

   U8                m_gameProductId;
   bool              m_isLoggingIn;
   bool              m_isLoggedIn;
   bool              m_isCreatingAccount;
   U32               m_connectionId;
   string            m_lastLoggedOutTime;
   int               m_lastRawDataIndex;

   mutable U32       m_beginTime, m_endTime;

   int               m_normalSleepTime, m_boostedSleepTime;
   bool              m_isThreadPerformanceBoosted;
   time_t            m_timeWhenThreadPerformanceBoosted;

   MBerNotifierList  m_callbacks;
   RawDataAccumulator m_rawDataBuffer[ PacketGameplayRawData::NumDataTypes ];

   vector< AssetInfoExtended >  m_staticAssets;
   vector< AssetInfoExtended >  m_dynamicAssets;

private:
   bool     SerializePacketOut( BasePacket* packet ) const;// helper

   bool     HandlePacketReceived( BasePacket* packetIn );
   int      ProcessInputFunction();
   //bool     HandlePacketIn( BasePacket* packetIn );

   bool     AddChatChannel( const ChatChannel& channel );

   void     BoostThreadPerformance();
   void     RestoreNormalThreadPerformance();
   void     ExpireThreadPerformanceBoost();

   bool     GetAsset( const string& hash, AssetInfoExtended& asset );
   bool     UpdateAssetData( const string& hash, AssetInfoExtended& asset );
};
///////////////////////////////////////////////////////

}