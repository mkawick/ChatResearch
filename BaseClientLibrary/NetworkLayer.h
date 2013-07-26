#pragma once
#include "../NetworkCommon/NetworkOut/Fruitadens.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
//#include "../ServerStack/NetworkCommon/NetworkOut/Fruitadens.h"
//#include "../ServerStack/NetworkCommon/Packets/BasePacket.h"
//#include "../ServerStack/NetworkCommon/Packets/GamePacket.h"

namespace Mber
{
///////////////////////////////////////////////////////

struct Demographics
{
   bool     isPrivate;// if true, most of the rest of this info is invalid.
   string   username;
   string   email;
   // ...
   list< string > listOfGamesOwned;
   int      gender;
   int      avatar;
   string   location;
   int      timeZoneGmt;
};

class BasicUser
{
   string    userName;
   string    userEmail;
   string    UUID;
   bool      isOnline;
};

class ChatChannel
{
   string   channelName;
   string   channelDetails;
   string   UUID;
   bool     isGameCreatedChannel;

   vector< BasicUser > usersInChannel; // not necessarily friends
};

class Group
{
   string groupName;
   string groupMotto;
   int    avatarId;
   string chatChannel; // the venn diagrap for groups and channels may not be 1-1
   vector< BasicUser > usersInGroup;
};

//-----------------------------------------------------

class UserNetworkEventNotifier
{
public:
   virtual void  UserLogin( bool success ) = 0;
   virtual void  UserLogout() = 0;

   virtual void  UserDemographics( const string& username, const Demographics& userDemographics ) = 0;
   virtual void  UserWinLoss( const string& username, const WinLoss& userWinLoss ) = 0;

   virtual void  GameData( U16 length, const U8* rawdata ) = 0;

   virtual void  FriendUpdate() {}
   virtual void  FriendOnlineStatusChanged( const string& uuid ) {}
   virtual void  ChatChannelUpdate() {}
   virtual void  ListOfFriendUpdate() {}

   virtual void  SearchResults( vector< string >& values ) {}

   virtual void  ChatReceived( const string& message, const string& channelUUID, const string& userUUID ){}
   virtual void  AssetReceived( const U8* buffer, int size, const string& assetId ){}

   UserNetworkEventNotifier() : connectionId ( 0 ) {}
   U32      connectionId;

   vector< BasicUser >     Friends;
   vector< ChatChannel >   Channels;
   vector< Group >         Groups;
};

///////////////////////////////////////////////////////

class RawDataAccumulator
{
public:
   RawDataAccumulator() : numBytes( 0 ) {}

   void  AddPacket( const PacketGameplayRawData * );
   bool  IsDone();
   void  PrepPackage( U8* & data, int& size );

   int   numBytes;
   deque< const PacketGameplayRawData* > packetsOfData;
};

///////////////////////////////////////////////////////

class NetworkLayer :  public Fruitadens
{
public:
   NetworkLayer( U8 gameProductId = GameProductId_MONKEYS_FROM_MARS );
   ~NetworkLayer();

   void     Init();
   void     Exit();
   
   bool     IsLoggingIn() const { return m_isLoggingIn; }
   bool     IsLoggedIn() const { return m_isLoggedIn; }

   void     RegisterCallbackInterface( UserNetworkEventNotifier* _callbacks ) { callbacks = _callbacks; }

   string   GetUsername() const { return m_username; }

   bool     RequestLogin( const string& username, const string& password, const string& languageCode );
   bool     RequestAccountCreate( const string& username, const string& useremail, const string& password, int languageId, const string& deviceId, const string& gkHash ); // deviceId could be NULL except in andriod world
   bool     RequestLogout() const;

   bool     RequestListOfFriends() const;
   bool     RequestListOfGames() const;
   bool     RequestListOfUsersLoggedIntoGame( ) const;
   bool     RequestFriendDemographics( const string& username ) const;
   bool     RequestUserWinLossRecord( const string& username ) const;

   bool     SendRawPacket( const char* buffer, int length ) const;

   bool     SendSearchForUsers( const string& searchString, int numRequested, int offset ) const; // min 2 char


   bool     ChangeGame( const string& gameName );

   // utility functions

   U32      FindGame( const string& name ) const;
   string   FindGameNameFromGameId( U32 id ) const;
   string   FindFriend( const string& name ) const;
   string   FindFriendFromUuid( const string& uuid ) const;

private:

   // datatypes
   bool  Log( const std::string& text, int priority = 1 ) const { return true; }
   bool  Log( const char* text, int priority = 1 ) const { return true; }
   
   typedef SerializedKeyValueVector< string >      UserNameKeyValue;
   typedef vector< PacketGameIdentification >      GameList;

   string            m_username, m_attemptedUsername;
   string            m_uuid;
   U32               m_selectedGame;

   KeyValueVector    m_friends;
   KeyValueVector    m_availableGames;
   GameList          m_gameList;
   U8                m_gameProductId;
   bool              m_isLoggingIn;
   bool              m_isLoggedIn;
   bool              m_isCreatingAccount;

   mutable U32       m_beginTime, m_endTime;

   UserNetworkEventNotifier* callbacks;
   RawDataAccumulator m_rawDataBuffer;

private:
   bool     SerializePacketOut( BasePacket* packet ) const;
   int      ProcessInputFunction();
   bool     HandlePacketIn( BasePacket* packetIn );
};
///////////////////////////////////////////////////////

}