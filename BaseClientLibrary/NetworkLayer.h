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



//-----------------------------------------------------

class UserNetworkEventNotifier
{
public:
   virtual void  UserLogin( bool success ) = 0;
   virtual void  UserLogout() = 0;

   virtual void  UserDemographics( const string& username, const Demographics& userDemographics ) = 0;
   virtual void  UserWinLoss( const string& username, const WinLoss& userWinLoss ) = 0;

   virtual void  GameData( U16 length, const U8* rawdata ) = 0;

   UserNetworkEventNotifier() : connectionId ( 0 ) {}
   U32      connectionId;
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
   bool     RequestLogout() const;

   bool     RequestListOfFriends() const;
   bool     RequestListOfGames() const;
   bool     RequestListOfUsersLoggedIntoGame( ) const;
   bool     RequestFriendDemographics( const string& username ) const;
   bool     RequestUserWinLossRecord( const string& username ) const;

   bool     SendRawPacket( const char* buffer, int length ) const;


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