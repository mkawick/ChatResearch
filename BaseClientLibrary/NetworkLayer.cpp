#include "NetworkLayer.h"

#include "../ServerStack/NetworkCommon/Utils/Utils.h"
#include "../ServerStack/NetworkCommon/Packets/PacketFactory.h"
#include "../ServerStack/NetworkCommon/Packets/ContactPacket.h"

#include <assert.h>
#include <iostream>
#pragma warning ( disable: 4996 )
#include <boost/lexical_cast.hpp>

using namespace Mber;

///////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------

NetworkLayer::NetworkLayer( U8 gameProductId ) : Fruitadens( "Networking Layer" ),
      m_gameProductId( gameProductId ), 
      m_isLoggingIn( false ),
      m_isLoggedIn( false )
      //, m_selectedGame( 0 ), m_numEchoBytesSent( 0 ), m_packetEchoPacketsSent( 0 ) {}
{
   InitializeSockets();
}

NetworkLayer::~NetworkLayer()
{
   Exit();
   ShutdownSockets();
}

//------------------------------------------------------------
//------------------------------------------------------------

void  NetworkLayer::Init()
{
   if( m_clientSocket != NULL || m_isConnected == true )
      return;
   
   string serverName = "chat.mickey.playdekgames.com";
   Connect( serverName.c_str(), 9600 );
}

void  NetworkLayer::Exit()
{
   Disconnect();
   m_isLoggingIn = false;
   m_isLoggedIn = false;
}

//------------------------------------------------------------

static inline U64 CreatePasswordHash( const char *pPassword )
{
   char salted_password[256];
   sprintf( salted_password, "pd%-14s", pPassword );

   const char *pPasswordText = salted_password;

   U64 hash = 5381; // definitely a magic number
   U64 c;

   while( (c = *pPasswordText++) != 0 )
   {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
   }

   return hash;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestLogin( const string& username, const string& password, const string& languageCode )
{
   if( m_isConnected == false )
   {
      Init();
   }
   PacketLogin login;
   login.loginKey = "deadbeef";// currently unused
   login.uuid = username;
   login.username = username;
   login.loginKey = password;
   login.languageCode = languageCode;
   login.password = boost::lexical_cast< string >( CreatePasswordHash( password.c_str() ) );
   m_attemptedUsername = username;

   m_isLoggingIn = true;
   SerializePacketOut( &login );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestAccountCreate( const string& username, const string& useremail, const string& password, int languageId, const string& deviceId, const string& gkHash )
{
   if( m_isConnected == false )
   {
      Init();
   }
   assert( m_isLoggedIn == false );

   PacketCreateAccount createAccount;
   createAccount.username = username;
   createAccount.useremail = useremail;
   createAccount.password = boost::lexical_cast< string >( CreatePasswordHash( password.c_str() ) );
   createAccount.deviceId = deviceId;
   createAccount.deviceAccountId = boost::lexical_cast< string >( CreatePasswordHash( gkHash.c_str() ) );
   createAccount.languageId = languageId;

   m_attemptedUsername = username;

   m_isCreatingAccount = true;
   SerializePacketOut( &createAccount );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestLogout() const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketLogout logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestListOfFriends() const
{
   PacketContact_GetListOfContacts friends;
   SerializePacketOut( &friends );

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
   if( name == m_username )// this method is commonly used as a lookup.. let's make it simple to use
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
      return m_username;

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

bool     NetworkLayer::SendRawPacket( const char* buffer, int length ) const
{
   PacketGameplayRawData raw;
   memcpy( raw.data, buffer, length );
   raw.size = length;
   //;
   return SerializePacketOut( &raw );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SerializePacketOut( BasePacket* packet ) const 
{
   U8 buffer[ MaxBufferSize ];
   int offset = 0;

   packet->gameInstanceId = m_selectedGame;
   packet->gameProductId = m_gameProductId;
   packet->SerializeOut( buffer, offset );
   m_beginTime = GetCurrentMilliseconds();
   return Fruitadens::SendPacket( buffer, offset );
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
         bool  shouldDelete = true;
         if( factory.Parse( buffer, offset, &packetIn ) == true )
         {
            m_endTime = GetCurrentMilliseconds();          
            shouldDelete = HandlePacketIn( packetIn );
         }
         else 
         {
            offset = numBytes;
         }
         if( packetIn && shouldDelete == true )
         {
            delete packetIn;
         }
      }
	}
   else
   {
#if PLATFORM == PLATFORM_WINDOWS
         U32 error = WSAGetLastError();
         if( error != WSAEWOULDBLOCK )
         {
            if( error == WSAECONNRESET )
            {
               m_isConnected = false;
               //m_hasFailedCritically = true;
               cout << "***********************************************************" << endl;
               cout << "Socket error was: " << hex << error << dec << endl;
               cout << "Socket has been reset" << endl;
               cout << "attempting a reconnect" << endl;
               cout << "***********************************************************" << endl;
               closesocket( m_clientSocket );
               CreateSocket();
            }
         }
#endif
   }
   
   return 1;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::HandlePacketIn( BasePacket* packetIn )
{
   switch( packetIn->packetType )
   {
      case PacketType_Contact:
      {
         switch( packetIn->packetSubType )
         {
         case PacketContact::ContactType_GetListOfContacts:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfContactsResponse* packet = static_cast< PacketContact_GetListOfContactsResponse* >( packetIn );
               cout << "num records: " << packet->friends.size() << endl;
            }
            break;
         }
         
      }
      break;
      case PacketType_ErrorReport:
      {
         switch( packetIn->packetSubType )
         {
         case PacketErrorReport::ErrorType_CreateFailed_BadPassword:
            cout << "Bad password" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DisallowedUsername:
            cout << "Bad username" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DuplicateUsername:
            cout << "Duplicate username" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DuplicateEmail:
            cout << "Duplicate email" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateAccount_Success:
            cout << "Account created" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateAccount_AccountUpdated:
            cout << "Account update" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_UserCreateAccountPending:
            cout << "Account creation pending. Check your email." << endl;
            break;
         }
         m_isCreatingAccount = false;
      }
      break;
      case PacketType_Login:
      {
         switch( packetIn->packetSubType )
         {
            case PacketLogin::LoginType_InformClientOfLoginStatus:
               {
                  PacketLoginToClient* login = static_cast<PacketLoginToClient*>( packetIn );
                  if( login->wasLoginSuccessful == true )
                  {
                     m_username = login->username;
                     m_uuid = login->uuid;
                     if( callbacks )
                     {
                        callbacks->UserLogin( true );
                        callbacks->connectionId = login->connectionId;
                        m_isLoggedIn = true;
                     }
                  }
                  else
                  {
                     if( callbacks )
                        callbacks->UserLogin( false );
                     //RequestLogout();
                     Disconnect();// server forces a logout.
                  }
                  m_isLoggingIn = false;
               }
               break;
            case PacketLogin::LoginType_PacketLogoutToClient:
               {
                  PacketLogout* logout = static_cast<PacketLogout*>( packetIn );
                  if( callbacks )
                  {
                     callbacks->UserLogout();
                     Disconnect();
                  }
                  //Cleanup();
                  m_isLoggingIn = false;
                  m_isLoggedIn = false;
               }
               break;

         }
         
      }
      break;
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
               if( gameId->gameProductId == m_gameProductId ) 
               {
                  m_selectedGame = gameId->gameId;
               }
            }
            break;
         case PacketGameToServer::GamePacketType_RawGameData:
            {
               PacketGameplayRawData* data = 
                  static_cast<PacketGameplayRawData*>( packetIn );

               m_rawDataBuffer.AddPacket( data );
               if( m_rawDataBuffer.IsDone() )
               {
                  U8* buffer = NULL;
                  int size = 0;
                  m_rawDataBuffer.PrepPackage( buffer, size );
                  if( callbacks )
                  {
                      callbacks->GameData( size, buffer );
                  }

                  delete buffer;
               }
            }
            return false;// this is important... we will delete the packets.
         
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
      }
      break;
   }

   return true;
}

///////////////////////////////////////////////////////////////////////////////////

void  RawDataAccumulator:: AddPacket( const PacketGameplayRawData * ptr )
{
   numBytes += ptr->size;
   packetsOfData.push_back( ptr );
}

bool  RawDataAccumulator:: IsDone()
{
   if( packetsOfData.size() < 1 )
      return false;

   const PacketGameplayRawData * ptr = packetsOfData.back();
   if( ptr->index > 1 )// allows for 1 or 0. 1 is proper.
      return false;

   return true;
}

void  RawDataAccumulator:: PrepPackage( U8* & data, int& size )
{
   assert( IsDone() );

   size = numBytes;
   data = new U8[size+1];
   U8* workingPointer = data;

   while( packetsOfData.size() )
   {
      const PacketGameplayRawData * ptr = packetsOfData.front();
      packetsOfData.pop_front();
      memcpy( workingPointer, ptr->data, ptr->size );
      workingPointer += ptr->size;
   }
   data[size] = 0;


   numBytes = 0;
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
