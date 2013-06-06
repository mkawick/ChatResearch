#pragma once
#include "..\networkcommon\networkout\fruitadens.h"
#include "..\networkcommon\packets\BasePacket.h"
#include "..\networkcommon\packets\GamePacket.h"

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



//-----------------------------------------------------

class UserNetworkEventNotifier
{
public:
   virtual void  UserLogin( bool success ) = 0;
   virtual void  UserLogout() = 0;

   virtual void  UserDemographics( const string& username, const Demographics& userDemographics ) = 0;
   virtual void  UserWinLoss( const string& username, const WinLoss& userWinLoss ) = 0;

   virtual void  GameData( U16 length, const U8* const rawdata ) = 0;
};


///////////////////////////////////////////////////////

class NetworkLayer :  public Fruitadens
{
public:
   NetworkLayer();
   ~NetworkLayer();

   void     Init();
   void     Exit();

   void     RegisterCallbackInterface( UserNetworkEventNotifier* _callbacks ) { callbacks = _callbacks; }

   string   GetUsername() const { return m_username; }

   bool     RequestLogin( const string& username, const string& password );
   bool     RequestLogout() const;

   bool     RequestListOfFriends() const;
   bool     RequestListOfGames() const;
   bool     RequestListOfUsersLoggedIntoGame( ) const;
   bool     RequestFriendDemographics( const string& username ) const;
   bool     RequestUserWinLossRecord( const string& username ) const;


   bool     ChangeGame( const string& gameName );

   // utility functions

   U32      FindGame( const string& name ) const;
   string   FindGameNameFromGameId( U32 id ) const;
   string   FindFriend( const string& name ) const;
   string   FindFriendFromUuid( const string& uuid ) const;

private:

   // datatypes
   typedef SerializedKeyValueVector< string >      UserNameKeyValue;
   typedef vector< PacketGameIdentification >      GameList;

   string            m_username, m_attemptedUsername;
   string            m_uuid;
   U32               m_selectedGame;

   KeyValueVector    m_friends;
   KeyValueVector    m_availableGames;
   GameList          m_gameList;

   mutable U32       m_beginTime, m_endTime;

   UserNetworkEventNotifier* callbacks;

private:
   bool     SerializePacketOut( const BasePacket* packet ) const;
   int      ProcessInputFunction();
   void     HandlePacketIn( BasePacket* packetIn );
};
///////////////////////////////////////////////////////