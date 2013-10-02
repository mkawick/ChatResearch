#include "NetworkLayer.h"

#include "../networkcommon/Utils/utils.h"
#include "../networkcommon/Packets/LoginPacket.h"
#include "../networkcommon/Packets/PacketFactory.h"

#include <assert.h>

///////////////////////////////////////////////////////////////////////////////////

NetworkLayer::NetworkLayer() : Fruitadens( "Networking Layer" )
{
}

NetworkLayer::~NetworkLayer()
{
}

void  NetworkLayer::Init()
{
   string serverName = "chat.mickey.playdekgames.com";

   InitializeSockets();
   Connect( serverName.c_str(), 9600 );
}

void  NetworkLayer::Exit()
{
   ShutdownSockets();
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestLogin( const string& userName, const string& password )
{
   PacketLogin login;
   login.loginKey = "deadbeef";// currently unused
   login.uuid = userName;
   login.userName = userName;
   login.loginKey = password;
   m_attemptedUsername = userName;

   SerializePacketOut( &login );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestLogout() const
{
   PacketLogout logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestListOfFriends() const
{
   PacketFriendsListRequest logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::ChangeGame( const string& gameName )// either name or shortname
{
   U32 id = FindGame( gameName );
   if( id == 0 )
   {
      return false;
   }
   else
   {
      m_selectedGame = id;
   }
   return true;
}

//-----------------------------------------------------------------------------

U32   NetworkLayer::FindGame( const string& name ) const
{
   GameList::const_iterator it = m_gameList.begin();
   while( it != m_gameList.end() )
   {
      const PacketGameIdentification& gameId = *it++;
      if( gameId.name == name || gameId.shortName == name )
      {
         return gameId.gameId;
      }
   }
   return 0;
}

//-----------------------------------------------------------------------------

string   NetworkLayer::FindGameNameFromGameId( U32 id ) const
{
   GameList::const_iterator it = m_gameList.begin();
   while( it != m_gameList.end() )
   {
      const PacketGameIdentification& gameId = *it++;
      if( gameId.gameId == id )
      {
         return gameId.name;
      }
   }
   return 0;
}

//-----------------------------------------------------------------------------

string   NetworkLayer::FindFriend( const string& name ) const 
{
   KeyValueVector::const_iterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      const KeyValueString&  kvpFriend = *itFriends++;
      if( kvpFriend.value == name )
      {
         return kvpFriend.key;
      }
   }
   if( name == m_userName )// this method is commonly used as a lookup.. let's make it simple to use
      return m_uuid;

   return string();
}

//-----------------------------------------------------------------------------

string   NetworkLayer::FindFriendFromUuid( const string& uuid ) const 
{
   KeyValueVector::const_iterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      const KeyValueString&  kvpFriend = *itFriends++;
      if( kvpFriend.key == uuid )
      {
         return kvpFriend.value;
      }
   }
   if( uuid == m_uuid )// this method is commonly used as a lookup.. let's make it simple to use
      return m_userName;

   return string();
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfGames() const
{
   PacketRequestListOfGames listOfGames;
   SerializePacketOut( &listOfGames );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfUsersLoggedIntoGame( ) const
{
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestFriendDemographics( const string& username ) const
{
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestUserWinLossRecord( const string& username ) const
{
   PacketRequestUserWinLoss packet;
   packet.userUuid;
   packet.gameUuid;
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SerializePacketOut( const BasePacket* packet ) const 
{
   U8 buffer[ MaxBufferSize ];
   int offset = 0;

   packet->SerializeOut( buffer, offset );
   m_beginTime = GetCurrentMilliseconds();
   return SendPacket( buffer, offset );
}


//-----------------------------------------------------------------------------

int   NetworkLayer::ProcessInputFunction()
{
   if( m_isConnected == false )
   {
      AttemptConnection();
   }

   U8 buffer[ MaxBufferSize ];

   int numBytes = static_cast< int >( recv( m_clientSocket, (char*) buffer, MaxBufferSize, NULL ) );
	if( numBytes != SOCKET_ERROR )
	{
		buffer[ numBytes ] = 0;// NULL terminate
      string str = "RECEIVED: ";
      str += (char*) buffer;
		Log( str );

      PacketFactory factory;
      int offset = 0;
      while( offset < numBytes )
      {
         BasePacket* packetIn = NULL;
         if( factory.Parse( buffer, offset, &packetIn ) == true )
         {
            m_endTime = GetCurrentMilliseconds();          
            HandlePacketIn( packetIn );
         }
         else 
         {
            offset = numBytes;
         }
         /*if( packetIn )
         {
            delete packetIn;
         }*/
      }
	}
   
   return 1;
}

//-----------------------------------------------------------------------------

void  NetworkLayer::HandlePacketIn( BasePacket* packetIn )
{
   switch( packetIn->packetType )
   {
      case PacketType_Login:
      {
         switch( packetIn->packetSubType )
         {
            case PacketLogin::LoginType_InformClientOfLoginStatus:
               {
                  PacketLoginToClient* login = static_cast<PacketLoginToClient*>( packetIn );
                  if( login->wasLoginSuccessful == true )
                  {
                     m_userName = login->userName;
                     m_uuid = login->uuid;
                     if( callbacks )
                        callbacks->UserLogin( true );
                  }
                  else
                  {
                     if( callbacks )
                        callbacks->UserLogin( false );
                     RequestLogout();
                  }
               }
               break;
            case PacketLogin::LoginType_Logout:
               {
                  PacketLogout* logout = static_cast<PacketLogout*>( packetIn );
                  if( callbacks )
                        callbacks->UserLogout();
                  //Cleanup();
               }
               break;
         }
         
      }
      case PacketType_UserInfo:
      {
         switch( packetIn->packetSubType )
         {
          case PacketUserInfo::InfoType_FriendsList:
            {
               PacketFriendsList* login = static_cast<PacketFriendsList*>( packetIn );
               m_friends = login->friendList.GetData();
               //DumpFriends();
            }
            break;
         case PacketUserInfo::InfoType_GroupsList:
            {
               
            }
            break;
            // demographics, winloss, etc.
         }
      }
      break;
      case PacketType_Gameplay:
      {
         switch( packetIn->packetSubType )
         {
         case PacketGameToServer::GamePacketType_GameIdentification:
            {
               PacketGameIdentification* gameId = 
                  static_cast<PacketGameIdentification*>( packetIn );

             /*  cout << "Available game: " << gameId->gameId << endl;
               cout << "   **** game name: " << gameId->name << endl;
               cout << "   ***  game nik name: " << gameId->shortName << endl;*/

               m_gameList.push_back( PacketGameIdentification( *gameId ) );
            }
            break;
         case PacketGameToServer::GamePacketType_RawGameData:
            {
               PacketGameplayRawData* data = 
                  static_cast<PacketGameplayRawData*>( packetIn );
               if( callbacks )
               {
                   callbacks->GameData( data->size, data->data );
               }
            }
            break;
         }
         case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
         {
            PacketRequestUserWinLossResponse* response = 
                  static_cast<PacketRequestUserWinLossResponse*>( packetIn );
            if( callbacks )
               callbacks->UserWinLoss( response->userUuid, response->winLoss );
            assert( 0 );// not finished, wrong user data
         }
         break;
      }
      break;
   }
}

//-----------------------------------------------------------------------------

// here for reference only
/*void     NetworkLayer::DumpFriends()
{
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   
   string text = "friends[ ";
   text += boost::lexical_cast< string >( m_friends.size() );
   text += " ] = {";
   m_pyro->Log( text );

   KeyValueVectorIterator it = m_friends.begin();
   while( it != m_friends.end() )
   {
      KeyValueSerializer<string>& kv = *it++;
      m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
      m_pyro->Log( kv.value, false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro->Log( " : ", false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
      m_pyro->Log( kv.key );
   }
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   m_pyro->Log( "}" );
}
//-----------------------------------------------------------------------------

void     NetworkLayer::DumpListOfGames( const GameList& gameList )
{
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   
   string text = "games: ";

   text += "[ ";
   text += boost::lexical_cast< string >( gameList.size() );
   text += " ] = {";
   m_pyro->Log( text );

   GameList::const_iterator it = gameList.begin();
   while( it != gameList.end() )
   {
      const PacketGameIdentification& gameId = *it++;
      m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
      m_pyro->Log( gameId.name, false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro->Log( ", ", false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
      m_pyro->Log( gameId.shortName );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro->Log( " : ", false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
      m_pyro->Log( gameId.gameId );
   }
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   m_pyro->Log( "}" );
}*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
