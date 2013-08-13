#include "NetworkLayer.h"

#include "../ServerStack/NetworkCommon/Utils/Utils.h"
#include "../ServerStack/NetworkCommon/Packets/PacketFactory.h"
#include "../ServerStack/NetworkCommon/Packets/ContactPacket.h"
#include "../ServerStack/NetworkCommon/Packets/ChatPacket.h"

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
      m_isLoggedIn( false ),
      m_connectionId( 0 ), 
      m_selectedGame( 0 )
{
   //m_serverDns = "chat.mickey.playdekgames.com";
   m_serverDns = "gateway.internal.playdekgames.com";
   InitializeSockets();
}

NetworkLayer::~NetworkLayer()
{
   Exit();
   ShutdownSockets();
}

//------------------------------------------------------------
//------------------------------------------------------------

void  NetworkLayer::Init( const char* serverDNS )
{
   if( m_clientSocket != SOCKET_ERROR || m_isConnected == true )
      return;

   if( serverDNS && strlen( serverDNS ) > 5 )
   {
      m_serverDns = serverDNS;
   }
   Connect( m_serverDns.c_str(), 9600 );
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
      Init( NULL );
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
  
bool  NetworkLayer::RequestChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex ) const
{
   PacketChatHistoryRequest history;
   history.chatChannelUuid = channelUuid;
   history.numRecords = numRecords;
   history.startingIndex = startingIndex;
   SerializePacketOut( &history );
   return true;
}

//-----------------------------------------------------------------------------
  
bool  NetworkLayer::RequestChatP2PHistory( const string& userUuid, int numRecords, int startingIndex ) const
{
   PacketChatHistoryRequest history;
   history.userUuid = userUuid;
   history.numRecords = numRecords;
   history.startingIndex = startingIndex;
   SerializePacketOut( &history );
   return true;
}



//-----------------------------------------------------------------------------
/*
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
*/
//-----------------------------------------------------------------------------

string   NetworkLayer::FindFriend( const string& name ) const 
{
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      const BasicUser&  kvpFriend = itFriends->value;
      
      if( kvpFriend.userName == name )
      {
         return itFriends->key;
      }
      itFriends++;
   }
   if( name == m_username )// this method is commonly used as a lookup.. let's make it simple to use
      return m_uuid;

   return string();
}

//-----------------------------------------------------------------------------

string   NetworkLayer::FindFriendFromUuid( const string& uuid ) const 
{
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      if( itFriends->key == uuid )
      {
         return itFriends->value.userName;
      }
   }
   if( uuid == m_uuid )// this method is commonly used as a lookup.. let's make it simple to use
      return m_username;

   return string();
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::GetFriend( int index, const BasicUser*& user )
{
   if( index < 0 || index >= m_friends.size() )
   {
      user = NULL;
      return false;
   }

   int i = 0;
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      if( i == index )
      {
         user = &itFriends->value;
         return true;
      }
      i ++;
      itFriends++;
   }

   return false;

}

//-----------------------------------------------------------------------------

bool    NetworkLayer::GetChannel( int index, ChatChannel& channel )
{
   if( index < 0 || index >= (int) m_channels.size() )
   {
      channel.Clear();
      return false;
   }

   int i = 0;
   vector< ChatChannel >::const_iterator itChannels = m_channels.begin();
   while( itChannels != m_channels.end() )
   {
      if( i == index )
      {
         channel = *itChannels;
         return true;
      }
      i ++;
      itChannels++;
   }

   return false;
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

bool     NetworkLayer::RequestListOfInvitationsSent() const
{
   PacketContact_GetListOfInvitationsSent    invitationRequest;
   return SerializePacketOut( &invitationRequest );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfInvitationsReceived() const
{
   PacketContact_GetListOfInvitations    invitationRequest;
   return SerializePacketOut( &invitationRequest );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::AcceptInvitation( const string& uuid ) const
{
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->key == uuid )
      {
         PacketContact_AcceptInvite invitationAccepted;
         invitationAccepted.invitationUuid = uuid;
        
         return SerializePacketOut( &invitationAccepted );
      }
      it++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::AcceptInvitationFromUsername( const string& userName ) const
{
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->value.inviterName == userName )
      {
         PacketContact_AcceptInvite invitationAccepted;
         invitationAccepted.invitationUuid = it->key;
        
         return SerializePacketOut( &invitationAccepted );
      }
      it++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::DeclineInvitation( const string& uuid, string message ) const
{
   PacketContact_DeclineInvitation invitationDeclined;
   invitationDeclined.invitationUuid = uuid;
   invitationDeclined.message = message;

   return SerializePacketOut( &invitationDeclined );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::GetListOfInvitationsSent( list< InvitationInfo >& listOfInvites )
{
   listOfInvites.clear();
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsSent;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      listOfInvites.push_back( it->value );
      it++;
   }
   return true;
}
//-----------------------------------------------------------------------------

bool     NetworkLayer::GetListOfInvitationsReceived( list< InvitationInfo >& listOfInvites )
{
   listOfInvites.clear();
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      listOfInvites.push_back( it->value );
      it++;
   }
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RegisterCallbackInterface( UserNetworkEventNotifier* _callbacks ) 
{
   for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
   {
      if( *it == _callbacks )
         return false;
   }
   
   m_callbacks.push_back( _callbacks );  
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SendRawPacket( const char* buffer, int length ) const
{
   PacketGameplayRawData raw;
   memcpy( raw.data, buffer, length );
   raw.size = length;

   return SerializePacketOut( &raw );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SendSearchForUsers( const string& searchString, int numRequested, int offset ) const // min 2 char
{
   if( searchString.size() < 2 )
      return false;

   assert( 0 ); // ContactType_Search
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::InviteUserToBeFriend( const string& uuid, const string& username, const string& message )
{
   if( uuid.size() < 2 && username.size() < 2 )
      return false;

   PacketContact_InviteContact invitation;
   invitation.username = username;
   invitation.uuid = uuid;
   invitation.message = message;

   return SerializePacketOut( &invitation );
}

//-----------------------------------------------------------------------------

bool	   NetworkLayer::SendTextMessage( const string& message, const string userUuid, const string channelUuid )
{

   PacketChatToServer chat;
   chat.message = message;
   chat.userUuid = userUuid;
   chat.channelUuid = channelUuid;
   SerializePacketOut( &chat );

   return true;
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
         case PacketContact::ContactType_GetListOfContactsResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfContactsResponse* packet = static_cast< PacketContact_GetListOfContactsResponse* >( packetIn );
               cout << "num records: " << packet->friends.size() << endl;
               if( m_callbacks.size() )
               {
                  m_friends.clear();

                  SerializedKeyValueVector< FriendInfo >::const_KVIterator it = packet->friends.begin();
                  while( it != packet->friends.end() )
                  {
                     BasicUser bu;
                     bu.isOnline = it->value.isOnline;
                     bu.userName = it->value.userName;
                     bu.UUID = it->key;

                     m_friends.insert( it->key, bu );
                     it++;
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendsUpdate();
                  }
               }
            }
            break;
         case PacketContact::ContactType_UserOnlineStatusChange:
            {
               cout << "contacts received" << endl;
               PacketContact_FriendOnlineStatusChange* packet = static_cast< PacketContact_FriendOnlineStatusChange* >( packetIn );
               cout << packet->friendInfo.userName << " online status = " << boolalpha << packet->friendInfo.isOnline << noboolalpha << endl;
               
               if( m_callbacks.size() )
               {
                  bool updated = false;
                  UserNameKeyValue::KVIterator  itFriends = m_friends.begin();
                  while( itFriends != m_friends.end() )
                  {
                     if( itFriends->key == packet->uuid )
                     {
                        itFriends->value.isOnline = packet->friendInfo.isOnline;
                        updated = true;
                     }
                     itFriends++;
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendOnlineStatusChanged( packet->uuid );
                  }
               }
            }
            break;
         case PacketContact::ContactType_GetListOfInvitationsResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfInvitationsResponse* packet = static_cast< PacketContact_GetListOfInvitationsResponse* >( packetIn );
               cout << "Num invites received: " << packet->invitations.size() << endl;

               m_invitationsReceived.clear();
               SerializedKeyValueVector< InvitationInfo >& kvVector = packet->invitations;
               m_invitationsReceived = kvVector;

               SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
               while (it != kvVector.end() )
               {
                  const InvitationInfo& invite = it->value;
                  cout << " **invite from: " << invite.inviterName << endl;
                  cout << "   invite to: " << invite.inviteeName << endl;
                  cout << "   message: " << invite.message << endl;
                  it++;
               }

               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationsReceivedUpdate();
                  }
               }
            }
            break;
         case PacketContact::ContactType_GetListOfInvitationsSentResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfInvitationsSentResponse* packet = static_cast< PacketContact_GetListOfInvitationsSentResponse* >( packetIn );
               cout << "Num invites sent: " << packet->invitations.size() << endl;

               m_invitationsSent.clear();
               SerializedKeyValueVector< InvitationInfo >& kvVector = packet->invitations;
               m_invitationsSent = kvVector;// expensive copy

               SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
               while (it != kvVector.end() )
               {
                  const InvitationInfo& invite = it->value;
                  cout << " **invite from: " << invite.inviterName << endl;
                  cout << "   invite to: " << invite.inviteeName << endl;
                  cout << "   message: " << invite.message << endl;
                  it++;
               }

               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationsSentUpdate();
                  }
               }
               
            }
            break;
         case PacketContact::ContactType_InviteSentNotification:
            {
               cout << "new invite received" << endl;
               PacketContact_InviteSentNotification* packet = static_cast< PacketContact_InviteSentNotification* >( packetIn );
               cout << "From " << packet->info.inviterName << endl;
               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationReceived( packet->info );
                  }
               }
               m_invitationsReceived.insert( packet->info.uuid, packet->info );
            }
            break;
         case PacketContact::ContactType_InvitationAccepted:
            {
               cout << "new invite received" << endl;
               PacketContact_InvitationAccepted* packet = static_cast< PacketContact_InvitationAccepted* >( packetIn );
               cout << "From " << packet->fromUsername << endl;
               cout << "To " << packet->toUsername << endl;
               cout << "Was accepted " << packet->wasAccepted << endl;

               for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->InvitationAccepted( packet->fromUsername, packet->toUsername, packet->wasAccepted );
               }

               cout << "request a new list of friends" << endl;
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

         case PacketErrorReport::ErrorType_UserBadLogin:
            cout << "Login not valid... either user email is wrong or the password." << endl;
            break;

         case PacketErrorReport::ErrorType_Contact_Invitation_success:
         case PacketErrorReport::ErrorType_Contact_Invitation_ProblemFindingUser:
         case PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyInvited:
         case PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyFriend:
         case PacketErrorReport::ErrorType_Contact_Invitation_InvalidUser:
         case PacketErrorReport::ErrorType_Contact_Invitation_CannotInviteSelf:
         case PacketErrorReport::ErrorType_Contact_Invitation_Accepted:
         case PacketErrorReport::ErrorType_Contact_Invitation_BadInvitation:
            cout << "Contacts code: " << (int)packetIn->packetSubType  << endl;
            break;

         case PacketErrorReport::ErrorType_ChatNotCurrentlyAvailable:// reported at the gateway
         case PacketErrorReport::ErrorType_BadChatChannel:
         case PacketErrorReport::ErrorType_NoChatChannel:
         case PacketErrorReport::ErrorType_UserNotOnline:
         case PacketErrorReport::ErrorType_NotAMemberOfThatChatChannel:
         case PacketErrorReport::ErrorType_YouAreTheOnlyPersonInThatChatChannel:
         case PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists:
         case PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel:
         case PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser:
            cout << "Chat code: " << (int)packetIn->packetSubType  << endl;
            break;
         }
         m_isCreatingAccount = false;
         for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
         {
            (*it)->OnError( packetIn->packetSubType );
         }
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
                     m_connectionId = login->connectionId;
                     m_lastLoggedOutTime = login->lastLogoutTime;
                     m_isLoggedIn = true;
                     //PacketChatChannelListRequest request;
                     //SerializePacketOut( &request );
                  }
                  else
                  {
                     //RequestLogout();
                     Disconnect();// server forces a logout.
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->UserLogin( login->wasLoginSuccessful );
                  }
                  m_isLoggingIn = false;
               }
               break;
            case PacketLogin::LoginType_PacketLogoutToClient:
               {
                  PacketLogout* logout = static_cast<PacketLogout*>( packetIn );
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->UserLogout();
                  }
                  Disconnect();
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
               //m_friends = login->friendList.GetData();
               //DumpFriends();
            }
            break;
         case PacketUserInfo::InfoType_ChatChannelList:
            {
               PacketChatChannelList* channelList = static_cast<PacketChatChannelList*>( packetIn );

               cout << " chat channel list received " << channelList->channelList.size() << endl;
               const SerializedKeyValueVector< ChannelInfo >& kvVector = channelList->channelList;
               SerializedKeyValueVector< ChannelInfo >::const_KVIterator channelIt = kvVector.begin();
               while( channelIt != kvVector.end() )
               {
                  ChatChannel newChannel;
                  newChannel.uuid = channelIt->value.channelUuid;
                  newChannel.channelName = channelIt->value.channelName;
                  newChannel.gameProductId = channelIt->value.gameProduct;
                  newChannel.gameInstanceId = channelIt->value.gameId;
                  newChannel.channelDetails= channelIt->value.channelName;
                  //newChannel.userList = channelIt->value.userList;
                  if( AddChatChannel( newChannel ) )
                  {
                     for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                     {
                        (*it)->ChatChannelUpdate( channelIt->value.channelUuid );
                     }
                  }
                  channelIt++;
               }
            }
            break;
            // demographics, winloss, etc.
         }
      }
      break;
      case PacketType_Chat:
      {
         switch( packetIn->packetSubType )
         {
          case PacketChatToServer::ChatType_ChatToClient:
            {
               cout << "You received a message: " << endl;
               PacketChatToClient* chat = static_cast<PacketChatToClient*>( packetIn );
               cout << " *** from: " << chat->username;
               cout << "     message: " << chat->message << endl;
               for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->ChatReceived( chat->message, chat->channelUuid, chat->userUuid, chat->timeStamp );
               }

               // add to some form of history?

               //m_friends = login->friendList.GetData();
               //DumpFriends();
            }
            break;
          case PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer:
             {
               cout << "You received a chat channel addition: " << endl;
               PacketChatUserAddedToChatChannelFromGameServer* chat = static_cast<PacketChatUserAddedToChatChannelFromGameServer*>( packetIn );
               if( m_callbacks.size() )
               {
                  bool found = false;
                  vector< ChatChannel >::iterator it = m_channels.begin();
                  while( it != m_channels.end() )
                  {
                     ChatChannel& channel = *it;
                     if( channel.uuid == chat->channelUuid )
                     {
                        found = true;
                        channel.userList = chat->userList;
                        for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                        {
                           (*it)->ChatChannelUpdate( channel.uuid );
                        }
                        break;
                     }
                  }
                  if( found == false )
                  {
                     ChatChannel newChannel;
                     newChannel.uuid = chat->channelUuid;
                     newChannel.channelName = chat->gameName;
                     newChannel.gameInstanceId = chat->gameId;
                     newChannel.channelDetails= chat->gameName;
                     newChannel.userList = chat->userList;
                     if( AddChatChannel( newChannel ) )
                     {
                        for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                        {
                           (*it)->ChatChannelUpdate( newChannel.uuid );
                        }
                     }
                  }
               }
             }
             break;
          case PacketChatToServer::ChatType_RequestHistoryResult:
            {
               PacketChatHistoryResult* history = static_cast<PacketChatHistoryResult*>( packetIn );
               int num = history->chat.size();

               
               /**/
               if( m_callbacks.size() == 0 || ( history->chatChannelUuid.size() == 0 && history->userUuid.size() == 0 ) )
               {
                  cout << "Chat items for this channel are [" << num << "] = {";

                  for( int i=0; i<num; i++ )
                  {
                     cout << history->chat[i].username << " said " << history->chat[i].message;
                     if( i < num-1 )
                         cout << ", ";
                  }
                  cout << "}" << endl;
               }
               else
               {
                  list< ChatEntry > listOfChats;
                  for( int i=0; i<num; i++ )
                  {
                     listOfChats.push_back( history->chat[i] );
                  }
                  bool invokeChannelNotifier = false;

                  if( history->chatChannelUuid.size() )
                  {
                     invokeChannelNotifier  = true;
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     if( invokeChannelNotifier )
                     {
                        (*it)->ChatChannelHistory( history->chatChannelUuid, listOfChats );
                     }
                     else
                     {
                        (*it)->ChatP2PHistory( history->userUuid, listOfChats );
                     }
                  }
               }
            }
            break;
          case PacketChatToServer::ChatType_UserChatStatusChange:
            {
               cout << "char contacts received" << endl;
               PacketChatUserStatusChangeBase* packet = static_cast< PacketChatUserStatusChangeBase* >( packetIn );
              // cout << "num records: " << packet->m_>friends.size() << endl;
               cout << packet->username << " online status = " << boolalpha << packet->statusChange << noboolalpha << endl;
               
               if( m_callbacks.size() )
               {
                  bool updated = false;
                  UserNameKeyValue::KVIterator  itFriends = m_friends.begin();
                  while( itFriends != m_friends.end() )
                  {
                     if( itFriends->key == packet->uuid )
                     {
                        itFriends->value.isOnline = packet->statusChange ? true : false; // 0=offline, everything else is some kind of online
                        updated = true;
                     }
                     itFriends++;
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendOnlineStatusChanged( packet->uuid );
                  }
               }
             }
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

               m_gameList.push_back( PacketGameIdentification( *gameId ) );
               if( gameId->gameProductId == m_gameProductId ) 
               {
                  m_selectedGame = gameId->gameId;
               }

               if( m_selectedGame )
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->ReadyToStartSendingRequestsToGame();
                  }
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
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->GameData( size, buffer );
                  }

                  delete buffer;
               }
            }
            return false;// this is important... we will delete the packets.
         
            case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
            {
               PacketRequestUserWinLossResponse* response = 
                     static_cast<PacketRequestUserWinLossResponse*>( packetIn );
               for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->UserWinLoss( response->userUuid, response->winLoss );
               }
               assert( 0 );// not finished, wrong user data
            }
            break;
         }
      }
      break;
   }

   return true;
}

//------------------------------------------------------------------------

bool     NetworkLayer::AddChatChannel( const ChatChannel& channel )
{
   vector< ChatChannel >::iterator it =  m_channels.begin();
   while( it != m_channels.end() )
   {
      if( it->uuid == channel.uuid )
      {
         return false;
      }
      it++;
   }
   m_channels.push_back( channel );
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
