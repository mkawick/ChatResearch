#pragma once
#include "../NetworkCommon/NetworkOut/Fruitadens.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"

#include "../NetworkCommon/Packets/ContactPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"

namespace Mber
{
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

class BasicUser
{
public:
   string    userName;
   //tring    userEmail;
   string    UUID;
   bool      isOnline;
};

class ChatChannel
{
public:
   ChatChannel(): gameInstanceId( 0 ) {}
   string   channelName;
   string   channelDetails;
   string   UUID;
   U32      gameInstanceId;

   SerializedKeyValueVector< string >   userList; // not necessarily friends
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
   virtual void  UserLogin( bool success ) {}
   virtual void  UserLogout() {}

   virtual void  UserDemographics( const string& username, const Demographics& userDemographics ) {}
   virtual void  UserWinLoss( const string& username, const WinLoss& userWinLoss ) {}

   virtual void  GameData( U16 length, const U8* rawdata ) {}

   virtual void  FriendUpdate() {}
   virtual void  FriendOnlineStatusChanged( const string& uuid ) {}
   virtual void  ChatChannelUpdate( string uuid ) {}
   virtual void  ListOfFriendUpdate() {}

   virtual void  SearchResults( vector< string >& values ) {}

   virtual void  ChatReceived( const string& message, const string& channelUUID, const string& userUUID ){}
   virtual void  AssetReceived( const U8* buffer, int size, const string& assetId ){}

   virtual void  InvitationReceived( const InvitationInfo& newInvitation ){}
   virtual void  ListOfInvitationsReceived( const list< InvitationInfo >& listOfInvitations ) {}
   virtual void  ListOfInvitationsSent( const list< InvitationInfo >& listOfInvitations ) {}
   virtual void  InvitationAccepted( const string& sender, const string& receiver, bool wasAccepted ){}

   virtual void  ChatChannelHistory( const string& channelUuid, const list< ChatEntry >& listOfChats ) {  }
   virtual void  ChatP2PHistory( const string& userUuid, const list< ChatEntry >& listOfChats ) { }

   virtual void  OnError( int code ){}

   UserNetworkEventNotifier(){}

   
};
/*
derived: public UserNetworkEventNotifier
         {
            void  GameData( U16 length, const U8* rawdata ) {};
         }*/

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

   void     Init( const char* serverDNS = "gateway.internal.playdekgames.com" );
   void     Exit();
   
   bool     IsLoggingIn() const { return m_isLoggingIn; }
   bool     IsLoggedIn() const { return m_isLoggedIn; }

   bool     RegisterCallbackInterface( UserNetworkEventNotifier* _callbacks );
   string   GetUsername() const { return m_username; }

   bool     SendRawPacket( const char* buffer, int length ) const;

   //--------------------------------------------------------------

   bool     RequestLogin( const string& username, const string& password, const string& languageCode );
   bool     RequestAccountCreate( const string& username, const string& useremail, const string& password, int languageId, const string& deviceId, const string& gkHash ); // deviceId could be NULL except in andriod world
   bool     RequestLogout() const;

   //--------------------------------------------------------------

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
   bool     GetListOfInvitationsReceived( list< InvitationInfo >& keyValues );

   bool     SendSearchForUsers( const string& searchString, int numRequested, int offset ) const; // min 2 char
   bool     InviteUserToBeFriend( const string& uuid, const string& username );

   //bool     ChangeGame( const string& gameName );
   bool	   SendTextMessage( const string& message, const string userUuid, const string chatChannel );// either can be empty but not both

   // utility functions

   U32      FindGame( const string& name ) const;
   string   FindGameNameFromGameId( U32 id ) const;
   string   FindFriend( const string& name ) const;
   string   FindFriendFromUuid( const string& uuid ) const;

   int      GetNumFriends() const { return m_friends.size(); }
   bool     GetFriend( int index, const BasicUser*& user );

private:

   // datatypes
   bool  Log( const std::string& text, int priority = 1 ) const { return true; }
   bool  Log( const char* text, int priority = 1 ) const { return true; }
   
   typedef SerializedKeyValueVector< BasicUser >   UserNameKeyValue;
   typedef vector< PacketGameIdentification >      GameList;


   string            m_username, m_attemptedUsername;
   string            m_uuid;
   string            m_serverDns;
   U32               m_selectedGame;

   SerializedKeyValueVector< InvitationInfo > m_invitationsreceived;
   SerializedKeyValueVector< InvitationInfo > m_invitationSent;
   UserNameKeyValue  m_friends;
   vector< ChatChannel >   m_channels;
   //vector< Group >         Groups;
   //KeyValueVector    m_availableGames;
   GameList          m_gameList;

   U8                m_gameProductId;
   bool              m_isLoggingIn;
   bool              m_isLoggedIn;
   bool              m_isCreatingAccount;
   U32               m_connectionId;

   mutable U32       m_beginTime, m_endTime;

   list< UserNetworkEventNotifier*>  m_callbacks;
   RawDataAccumulator m_rawDataBuffer;

private:
   bool     SerializePacketOut( BasePacket* packet ) const;
   int      ProcessInputFunction();
   bool     HandlePacketIn( BasePacket* packetIn );
};
///////////////////////////////////////////////////////

}