
#include <assert.h>
#include <boost/lexical_cast.hpp>

#include "FruitadensClientChat.h"

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/ServerConstants.h"
#include "../NetworkCommon/Packets/GamePacket.h"

#include <mmsystem.h>
//-----------------------------------------------------------------------------

// remember to wrap all of these in Gateway wrappers.
bool  FruitadensClientChat::Login( const string& username, const string& password )
{
   PacketLogin login;
   login.loginKey = "deadbeef";
   login.uuid = username;
   login.username = username;
   login.loginKey = password;
   m_attemptedUsername = username;

   SerializePacketOut( &login );

   return true;
}

//-----------------------------------------------------------------------------

bool  FruitadensClientChat::FinalFixup()
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainedInterface* outputPtr = (*itOutputs).m_interface;
      m_pyro = static_cast< Pyroraptor* > ( outputPtr );
      return true;
   }

   assert( 0 );
   return false;
}

//-----------------------------------------------------------------------------

bool  FruitadensClientChat::Logout()
{
   PacketLogout logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------

bool  FruitadensClientChat::RequestFriends()
{
   PacketFriendsListRequest logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------

bool  FruitadensClientChat::RequestGroups()
{
   PacketGroupsListRequest logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------

bool  FruitadensClientChat::ChangeChannel( string& channel )
{
   string channelUuid= FindChatChannel( channel );
   if( channelUuid.length() == 0 )
   {
      Log( "error: bad chat channel" );
      Log( " try one of these " );
      DumpChatChannels();
   }
   else
   {
      PacketChangeChatChannel channelChange;
      channelChange.chatChannelUuid = channelUuid;
      SerializePacketOut( &channelChange );
   }
   return true;
}

//-----------------------------------------------------------------------------

bool  FruitadensClientChat::RequestMissedHistoryRequest()
{
   PacketChatMissedHistoryRequest logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------

bool     FruitadensClientChat::CreateChatChannel( const string& chatChannelName )
{
   PacketChatCreateChatChannel packet;
   packet.name = chatChannelName;
   SerializePacketOut( &packet );

   /*m_pyro->SetConsoleColor( Pyroraptor::ColorsError );
   m_pyro->Log( "error: not implemented" );*/
   return false;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::DeleteChatChannel( const string& chatChannelUuid )
{
   PacketChatDeleteChatChannel packet;
   packet.uuid = chatChannelUuid;
   SerializePacketOut( &packet );

 /*  m_pyro->SetConsoleColor( Pyroraptor::ColorsError );
   m_pyro->Log( "error: not implemented" );*/
   return false;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::InviteUserToChat( const string& userUuid, const string& chatChannelUuid )
{
   //PacketChatInviteUserToChatChannel
   m_pyro->SetConsoleColor( Pyroraptor::ColorsError );
   m_pyro->Log( "error: not implemented" );
   return false;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::AddUserToChat( const string& userUuid, const string& chatChannelUuid )
{
   PacketChatAddUserToChatChannel add;
   add.chatChannelUuid = chatChannelUuid;
   add.userUuid = userUuid;
   SerializePacketOut( &add );

  /* m_pyro->SetConsoleColor( Pyroraptor::ColorsError );
   m_pyro->Log( "error: not implemented" );*/
   return false;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::RemoveUserFromChat( const string& userUuid, const string& chatChannelUuid )
{
   PacketChatRemoveUserFromChatChannel  remove;
   remove.chatChannelUuid = chatChannelUuid;
   remove.userUuid = userUuid;
   SerializePacketOut( &remove );
   //
/*   m_pyro->SetConsoleColor( Pyroraptor::ColorsError );
   m_pyro->Log( "error: not implemented" );*/
   return false;
}

//-----------------------------------------------------------------------------
  
bool     FruitadensClientChat::RequestChatHistory( const string& channel )
{
   string channelUuid= FindChatChannel( channel );
   if( channelUuid.length() == 0 )
   {
      m_pyro->Log( "error: bad chat channel" );
      m_pyro->Log( " try one of these " );
      DumpChatChannels();
   }
   else
   {
      PacketChatHistoryRequest history;
      history.chatChannelUuid = channelUuid;
      SerializePacketOut( &history );
   }
   return true;
}

//-----------------------------------------------------------------------------

void     FruitadensClientChat::AddChatChannelsToList( const ChannelKeyValue& newChannels )
{
   ChannelKeyValue::const_KVIterator      itNewChannels = newChannels.begin();
   while( itNewChannels != newChannels.end() )
   {
      bool found = false;
      const ChannelKeyValue::KeyValue&    kvpNewChannel = *itNewChannels++;

      ChannelKeyValue::KVIterator   itGroups = m_groups.begin();
      while( itGroups != m_groups.end() )
      {
         ChannelKeyValue::KeyValue& kvpGroup = *itGroups++;
         if( kvpNewChannel.key == kvpGroup.key )
         {
            kvpGroup.value.channelName = kvpNewChannel.value.channelName;
            kvpGroup.value.isActive = kvpNewChannel.value.isActive;// copy over any new properties
            found = true;
            break;
         }
      }
      if( found == false )
      {
         m_groups.insert( kvpNewChannel.key, kvpNewChannel.value );
      }
   }
}

//-----------------------------------------------------------------------------

void     FruitadensClientChat::AddFriendsToList( const UserNameKeyValue& users )
{
   KeyValueList::const_iterator     itUsers = users.begin();
   while( itUsers != users.end() )
   {
      bool found = false;
      const KeyValue& kvpUsers = *itUsers++;
      KeyValueList::const_iterator  itFriends = m_friends.begin();
      while( itFriends != m_friends.end() )
      {
         const KeyValue&  kvpFriend = *itFriends++;
         if( kvpUsers.key == kvpFriend.key )
         {
            found = true;
            break;
         }
      }
      if( found == false )
      {
         m_friends.push_back( KeyValue( kvpUsers.key, kvpUsers.value ) );
      }
   }
}

//-----------------------------------------------------------------------------

string   FruitadensClientChat::FindChatChannel( const string& name ) const
{
   ChannelKeyValue::const_KVIterator   itGroups = m_groups.begin();
   while( itGroups != m_groups.end() )
   {
      const ChannelKeyValue::KeyValue& kvpGroup = *itGroups++;

      if( kvpGroup.value.channelName == name )
      {
         return kvpGroup.key;
      }
   }
   return string();
}

//-----------------------------------------------------------------------------

string   FruitadensClientChat::FindChatChannelFromUuid( const string& uuid ) const
{
   ChannelKeyValue::const_KVIterator   itGroups = m_groups.begin();
   while( itGroups != m_groups.end() )
   {
      const ChannelKeyValue::KeyValue& kvpGroup = *itGroups++;

      if( kvpGroup.key == uuid )
      {
         return kvpGroup.value.channelName;
      }
   }
   return string();
}

//-----------------------------------------------------------------------------

string   FruitadensClientChat::FindFriend( const string& name ) const 
{
   KeyValueList::const_iterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      const KeyValue&  kvpFriend = *itFriends++;
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

string   FruitadensClientChat::FindFriendFromUuid( const string& uuid ) const 
{
   KeyValueList::const_iterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      const KeyValue&  kvpFriend = *itFriends++;
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

bool     FruitadensClientChat::RequestAllChatChannels( const string& chatChannelUuid )
{
   //PacketChatRequestChatChannelList
   m_pyro->SetConsoleColor( Pyroraptor::ColorsError );
   m_pyro->Log( "error: not implemented" );
   return false;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::RequestChatters( const string& chatChannelUuid )
{
   PacketChatRequestChatters chatters;
   chatters.chatChannelUuid = chatChannelUuid;
   SerializePacketOut( &chatters );
   return false;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::EnableDisableFiltering( const string& chatChannelUuid, bool enable )
{
   //PacketChatEnableFiltering
   m_pyro->SetConsoleColor( Pyroraptor::ColorsError );
   m_pyro->Log( "error: not implemented" );
   return false;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::RequestMembers( const string& chatChannelUuid )
{
   PacketChatListAllMembersInChatChannel allMembers;
   allMembers.chatChannelUuid = chatChannelUuid;
   SerializePacketOut( &allMembers );
   return false;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::LoadAllChannels()
{
   PacketChatAdminLoadAllChannels loadAll;
   SerializePacketOut( &loadAll );
   return true;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::RequestFullListOfChatChannels( bool isFullList )
{
   PacketChatAdminRequestChatChannelList chatChannels;
   chatChannels.isFullList = isFullList;
   SerializePacketOut( &chatChannels );
   return true;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::RequestFullListOfUsers( bool isFullList )
{
   PacketChatAdminRequestUsersList chatUsers;
   chatUsers.isFullList = isFullList;
   SerializePacketOut( &chatUsers );
   return true;
}

//-----------------------------------------------------------------------------

bool	FruitadensClientChat::SendMessage( const string& message )
{
   PacketChatToServer chat;
   chat.message = message;
   SerializePacketOut( &chat );

   return true;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::CreateGame( const string& gameName )
{
   PacketCreateGame game;
   game.name = gameName;
   SerializePacketOut( &game );
   return true;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::AddUserToGame( const string& userUuid, const string& gameUuid )
{
   PacketAddUserToGame game;
   game.userUuid = userUuid;
   game.gameUuid = gameUuid;
   SerializePacketOut( &game );

   return true;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::LoadListOfGames()
{
   PacketRequestListOfGames listOfGames;
   SerializePacketOut( &listOfGames );

   return true;
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::DeleteGame( const string& gameUuid )
{
   PacketDeleteGame game;
   game.uuid = gameUuid;

   SerializePacketOut( &game );

   return true;
}

//-----------------------------------------------------------------------------

string   FruitadensClientChat::FindGame( const string& name )
{
   KeyValueList::const_iterator  itGames = m_availableGames.begin();
   while( itGames != m_availableGames.end() )
   {
      const KeyValue&  kvpGame = *itGames++;
      if( kvpGame.value == name )
      {
         return kvpGame.key;
      }
   }

   return string();
}

//-----------------------------------------------------------------------------

string   FruitadensClientChat::FindGameByUuid( const string& uuid )
{
   KeyValueList::const_iterator  itGames = m_availableGames.begin();
   while( itGames != m_availableGames.end() )
   {
      const KeyValue&  kvpGame = *itGames++;
      if( kvpGame.key == uuid )
      {
         return kvpGame.value;
      }
   }

   return string();
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void     FruitadensClientChat::DumpFriends()
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

void     FruitadensClientChat::DumpChatChannels()
{
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );

   string text = "groups[ ";
   text += boost::lexical_cast< string >( m_groups.size() );
   text += " ] = {";
   m_pyro->Log( text );

   ChannelKeyValue::KVIterator it = m_groups.begin();
   while( it != m_groups.end() )
   {
      ChannelKeyValue::KeyValue& kvp = *it++;
      m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
      m_pyro->Log( kvp.value.channelName, false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro->Log( " : ", false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
      m_pyro->Log( kvp.key, false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro->Log( ", isActive=", false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
      m_pyro->Log( kvp.value.isActive );
   }
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   m_pyro->Log( "}" );
}


//-----------------------------------------------------------------------------

void     FruitadensClientChat::DumpListOfUsers( const string& channelUuid, const SerializedKeyValueVector< string >& users )
{
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   
   string text = "chat channel: ";
   text += FindChatChannelFromUuid( channelUuid );

   text += " ... users[ ";
   text += boost::lexical_cast< string >( users.size() );
   text += " ] = {";
   m_pyro->Log( text );

   KeyValueConstIterator it = users.begin();
   while( it != users.end() )
   {
      const KeyValueSerializer<string>& kv = *it++;
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

bool     FruitadensClientChat::SerializePacketOut( const BasePacket* packet )
{
   U8 buffer[ MaxBufferSize ];
   int offset = 0;

   packet->SerializeOut( buffer, offset );
   m_beginTime = timeGetTime();
   return SendPacket( buffer, offset );
}

//-----------------------------------------------------------------------------

bool     FruitadensClientChat::SendRawData( U8* buffer, int len )
{
   return SendPacket( buffer, len );
}

//-----------------------------------------------------------------------------

int   FruitadensClientChat::ProcessInputFunction()
{
   if( m_isConnected == false )
   {
      AttemptConnection();
   }

   U8 buffer[ MaxBufferSize ];

   int numBytes = recv( m_clientSocket, (char*) buffer, MaxBufferSize, NULL );
	if( numBytes != SOCKET_ERROR)
	{
      m_pyro->SetConsoleColor( Pyroraptor::ColorsText );
      m_pyro->Log( "Data has come in" );
		buffer[ numBytes ] = 0;// NULL terminate
      string str = "RECEIVED: ";
      str += (char*) buffer;
		Log( str );
      PacketFactory factory;
      int offset = 0;
      while( offset < numBytes )
      {
         BasePacket* packetIn;
         if( factory.Parse( buffer, offset, &packetIn ) == true )
         {
            m_endTime = timeGetTime();

            m_pyro->SetConsoleColor( Pyroraptor::ColorsDarkBlue );
            
            m_pyro->Log( "Time passed between send and receive was : ", false );
            m_pyro->Log( (int) (m_endTime - m_beginTime), false );
            m_pyro->Log( " ms" );
            HandlePacketIn( packetIn );
            m_pyro->SetConsoleColor( Pyroraptor::ColorsText );
         }
         else 
         {
            offset = numBytes;
         }
      }
	}
   
   return 1;
}

//-----------------------------------------------------------------------------

void  FruitadensClientChat::HandlePacketIn( BasePacket* packetIn )
{
   switch( packetIn->packetType )
   {
      case PacketType_Login:
      {
         switch( packetIn->packetSubType )
         {
         case PacketLogin::LoginType_Login:
            {
               PacketLogin* login = static_cast<PacketLogin*>( packetIn );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "User login ", false ); 
               m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
               m_pyro->Log( login->username );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_username = login->username;
               m_uuid = login->uuid;
            }
            break;
         case PacketLogin::LoginType_PacketLogoutToClient:
            {
               PacketLogoutToClient* login = static_cast<PacketLogoutToClient*>( packetIn );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "User logged out ", false ); 
               m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
               m_pyro->Log( login->username );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
            }
            break;
         case PacketLogin::LoginType_InformClientOfLoginStatus:
            {
               PacketLoginToClient* login = static_cast<PacketLoginToClient*>( packetIn );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "User login success = ", false ); 
               m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
               m_pyro->Log( login->wasLoginSuccessful );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsText );
               if( login->wasLoginSuccessful == true )
               {
                  //RequestFriends();
                  //RequestGroups();
                  //RequestMissedHistoryRequest();
                  m_username = login->username;
                  m_uuid = login->uuid;
               }
               else
               {
                  m_pyro->Log( "User has been logged out" ); 
                  Logout();
               }
            }
            break;
         case PacketLogin::LoginType_Logout:
            {
               PacketLogout* login = static_cast<PacketLogout*>( packetIn );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "User logged out " ); 
               Cleanup();
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
               DumpFriends();
            }
            break;
         case PacketUserInfo::InfoType_GroupsList:
            {
               PacketGroupsList* login = static_cast<PacketGroupsList*>( packetIn );
               m_groups = login->groupList.GetData();
               DumpChatChannels();
            }
            break;
         }
      }
      break;
   case PacketType_Chat:
      {
         switch( packetIn->packetSubType )
         {
        /* case PacketChatToServer::ChatType_ChatToServer:
            {
               PacketChatToServer* chat = static_cast<PacketChatToServer*>( packetIn );
            }
            return true;*/
         case PacketChatToServer::ChatType_ChatToClient:
            {
               PacketChatToClient* chat = static_cast<PacketChatToClient*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
               m_pyro->Log( chat->username, false ) ;

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " says ", false ); 

               m_pyro->SetConsoleColor( Pyroraptor::ColorsChatText );
               m_pyro->Log( chat->message, false );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " on chat channel: ", false );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               m_pyro->Log( FindChatChannelFromUuid( chat->chatChannelUuid ) );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
            }
            break;

         case PacketChatToServer::ChatType_ChangeChatChannelToClient:
            {
               PacketChangeChatChannelToClient* channel = static_cast<PacketChangeChatChannelToClient*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "Channel change ", false ); 
               m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
               m_pyro->Log( channel->username, false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " changed to channel: ", false ); 
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               m_currentChannel = FindChatChannelFromUuid( channel->chatChannel );
               m_pyro->Log( m_currentChannel );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
            }
            break;
         case PacketChatToServer::ChatType_SendListOfChannelsToClient:
            {
               PacketChatChannelListToClient* channelList = static_cast<PacketChatChannelListToClient*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               int num = channelList->chatChannel.size();
               m_pyro->Log( "Channels for this user are [", false );
               m_pyro->Log( num, false );
               m_pyro->Log( "] = {" ); 

               m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
               for( int i=0; i<num; i++ )
               {
                  m_pyro->Log( FindChatChannelFromUuid( channelList->chatChannel[i] ), false );
                  if( i < num-1 )
                      m_pyro->Log( ", ", false );
               }
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "}" );
            }
            break;
         case PacketChatToServer::ChatType_RequestHistoryResult:
            {
               PacketChatHistoryResult* history = static_cast<PacketChatHistoryResult*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               int num = //history->chatChannel.size();
               history->chat.size();
               m_pyro->Log( "Chat items for this channel are [", false );
               m_pyro->Log( num, false );
               m_pyro->Log( "] = {" ); 

               m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
               for( int i=0; i<num; i++ )
               {
                  m_pyro->Log( history->chat[i].username, false );
                  m_pyro->Log( " said " , false );
                  m_pyro->Log( history->chat[i].message, false );
                  if( i < num-1 )
                      m_pyro->Log( ", " );
               }
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "}" );
            }
            break;
         case PacketChatToServer::ChatType_RequestHistorySinceLastLoginResponse:
            {
               PacketChatMissedHistoryResult* history = static_cast<PacketChatMissedHistoryResult*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "Log of all chat received while logged off " );
               int num = history->history.size();
               m_pyro->Log( "total chat items for this channel are [", false );
               m_pyro->Log( num, false );
               m_pyro->Log( "] = {" ); 

               m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
               for( int i=0; i<num; i++ )
               {
                  m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
                  string friendName = FindFriendFromUuid( history->history[i].senderUuid );
                  if( friendName.size() == 0 )
                     friendName = history->history[i].senderUuid;
                  m_pyro->Log( friendName, false );
                  
                  m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
                  m_pyro->Log( " on chat channel " , false );
                  m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
                  m_pyro->Log( FindChatChannelFromUuid( history->history[i].chatChannelUuid ), false );

                  m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
                  m_pyro->Log( " said " , false );
                  m_pyro->SetConsoleColor( Pyroraptor::ColorsText );
                  m_pyro->Log( history->history[i].message, false );

                  m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
                  if( i < num-1 )
                      m_pyro->Log( ", " );
               }
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( "}" );
            }
            break;
         case PacketChatToServer::ChatType_CreateChatChannelResponse:
            {
               PacketChatCreateChatChannelResponse* response = 
                  static_cast<PacketChatCreateChatChannelResponse*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " Create channel ", false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               if( response->successfullyCreated == true )
               {
                  m_pyro->Log( " succeeded: ", false );
                  m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
                  m_pyro->Log( CreatePrintablePair( response->name, response->uuid ) );
               }
               else
               {
                  m_pyro->Log( " failed " );
               }
            }
            break;
         case PacketChatToServer::ChatType_DeleteChatChannelResponse:
            {
               PacketChatDeleteChatChannelResponse* response = 
                  static_cast<PacketChatDeleteChatChannelResponse*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " Delete channel ", false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               if( response->successfullyDeleted == true )
               {
                  m_pyro->Log( " succeeded: ", false );
                  m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
                  m_pyro->Log( FindChatChannelFromUuid( response->uuid ) );
               }
               else
               {
                  m_pyro->Log( " failed " );
               }
            }
            break;
         case PacketChatToServer::ChatType_InviteUserToChatChannelResponse:
            {
               PacketChatInviteUserToChatChannelResponse* response = 
                  static_cast<PacketChatInviteUserToChatChannelResponse*>( packetIn );
            }
            break;
         case PacketChatToServer::ChatType_AddUserToChatChannelResponse:
            {
               PacketChatAddUserToChatChannelResponse* response = 
                  static_cast<PacketChatAddUserToChatChannelResponse*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " User ", false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               string username = FindFriendFromUuid( response->userUuid );
               if( username.size() > 0 )
               {
                  m_pyro->Log( CreatePrintablePair( username, response->userUuid ), false );
               }
               else
               {
                  m_pyro->Log( response->userUuid, false );
               }
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );

               if( response->success == true )
               {
                  m_pyro->Log( " was successfully added to chat channel ", false );
               }
               else
               {
                  m_pyro->Log( " FAILED to be added to chat channel ", false );
               }
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               string chatChannel = FindChatChannelFromUuid( response->chatChannelUuid );
               if( chatChannel.size() > 0 )
               {
                  m_pyro->Log( CreatePrintablePair( chatChannel, response->chatChannelUuid ), true );
               }
               else
               {
                  m_pyro->Log( response->chatChannelUuid , true );
               }
            }
            break;
         case PacketChatToServer::ChatType_RemoveUserFromChatChannelResponse:
            {
               PacketChatRemoveUserFromChatChannelResponse* response = 
                  static_cast<PacketChatRemoveUserFromChatChannelResponse*>( packetIn );

               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " User ", false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               string username = FindFriendFromUuid( response->userUuid );
               if( username.size() > 0 )
               {
                  m_pyro->Log( CreatePrintablePair( username, response->userUuid ), false );
               }
               else
               {
                  m_pyro->Log( response->userUuid, false );
               }
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );

               if( response->success == true )
               {
                  m_pyro->Log( " was successfully removed from chat channel ", false );
               }
               else
               {
                  m_pyro->Log( " FAILED to be removed from chat channel ", false );
               }
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               string chatChannel = FindChatChannelFromUuid( response->chatChannelUuid );
               if( chatChannel.size() > 0 )
               {
                  m_pyro->Log( CreatePrintablePair( chatChannel, response->chatChannelUuid ), true );
               }
               else
               {
                  m_pyro->Log( response->chatChannelUuid, true );
               }
            }
            break;
         case PacketChatToServer::ChatType_RequestChattersResponse:
            {
               PacketChatRequestChattersResponse* response = 
                  static_cast<PacketChatRequestChattersResponse*>( packetIn );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " Chatters of group list " );
               DumpListOfUsers( response->chatChannelUuid, response->userList );
            }
            break;
         case PacketChatToServer::ChatType_EnableDisableFilteringResponse:
            {
               PacketChatEnableFilteringResponse* response = 
                  static_cast<PacketChatEnableFilteringResponse*>( packetIn );
            }
            break;
         case PacketChatToServer::ChatType_ListAllMembersInChatChannelResponse:
            {
               PacketChatListAllMembersInChatChannelResponse* response = 
                  static_cast<PacketChatListAllMembersInChatChannelResponse*>( packetIn );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " Members list " );
               DumpListOfUsers( response->chatChannelUuid, response->userList );
            }
            break;
         case PacketChatToServer::ChatType_AdminLoadAllChannelsResponse:
            {
               PacketChatAdminLoadAllChannelsResponse* response = 
                  static_cast<PacketChatAdminLoadAllChannelsResponse*>( packetIn );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " Load all channels ", false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               if( response->success == true )
               {
                  m_pyro->Log( " succeeded " );
               }
               else
               {
                  m_pyro->Log( " failed " );
               }
            }
            break;
         case PacketChatToServer::ChatType_UserChatStatusChange:
            {
               PacketChatUserStatusChangeBase* status = 
                  static_cast<PacketChatUserStatusChangeBase*>( packetIn );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " Another User has changed his/her status ", false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
               m_pyro->Log( CreatePrintablePair( status->username, status->uuid ), false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
               m_pyro->Log( " has changed status to ", false );
               m_pyro->SetConsoleColor( Pyroraptor::ColorsText );
               m_pyro->Log( (int) status->statusChange );
            }
            break;
         case PacketChatToServer::ChatType_AdminRequestChatChannelListResponse:
            {
               PacketChatAdminRequestChatChannelListResponse* channelList =
                  static_cast<PacketChatAdminRequestChatChannelListResponse*>( packetIn );

               AddChatChannelsToList( channelList->chatChannels );
               DumpChatChannels();
               
            }
            break;
         case PacketChatToServer::ChatType_AdminRequestUsersListResponse:
            {
               PacketChatAdminRequestUsersListResponse* friendsList =
                  static_cast<PacketChatAdminRequestUsersListResponse*>( packetIn );

               AddFriendsToList( friendsList->users );
               DumpFriends();
            }
            break;
         }
      }
      break;
   case PacketType_GatewayWrapper:
      {
         PacketGatewayWrapper* gwPacket = static_cast<PacketGatewayWrapper*>( packetIn );
         BasePacket* tempPacket = gwPacket->pPacket;

         m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
         m_pyro->Log( "Gateway packet received, type: ", false );
         m_pyro->Log( tempPacket->packetType );
         m_pyro->Log( "   subtype: ", false );
         m_pyro->Log( tempPacket->packetSubType );
         m_pyro->Log( "   connectionId: ", false );
         m_pyro->Log( gwPacket->connectionId );
         HandlePacketIn( tempPacket );
      }
      break;
   case PacketType_ErrorReport:
      {
          PacketErrorReport* errorPacket = static_cast<PacketErrorReport*>( packetIn );

          m_pyro->SetConsoleColor( Pyroraptor::ColorsError );
          m_pyro->Log( "     **************************************** " );
          m_pyro->Log( "      ERROR type: ", false );
          m_pyro->Log( (int) (errorPacket->packetSubType) );
          switch( errorPacket->packetSubType )
          {
          case PacketErrorReport::ErrorType_Generic:
             m_pyro->Log( "      generic " );
             break;
          case PacketErrorReport::ErrorType_UserBadLogin:
             m_pyro->Log( "      bad login " );
             break;
          case PacketErrorReport::ErrorType_UserDoesNotExist:
             m_pyro->Log( "      user DNE " );
             break;
          case PacketErrorReport::ErrorType_UserBlocked:
             m_pyro->Log( "      user blocked " );
             break;
          case PacketErrorReport::ErrorType_UserLoggedOut:
             m_pyro->Log( "      user logged out " );
             break;
          case PacketErrorReport::ErrorType_ChatNotCurrentlyAvailable:
             m_pyro->Log( "      chat not available " );
             break;
          case PacketErrorReport::ErrorType_BadChatChannel:
             m_pyro->Log( "      bad chat channel. Name cannot be found. " );
             break;
          case PacketErrorReport::ErrorType_NoChatChannel:
             m_pyro->Log( "      chat channel not specified " );
             break;
          case PacketErrorReport::ErrorType_NotAMemberOfThatChatChannel:
             m_pyro->Log( "      user not a member of that chat channel " );
             break;
          case PacketErrorReport::ErrorType_YouAreTheOnlyPersonInThatChatChannel:
             m_pyro->Log( "      nobody else is on that channel " );
             break;
          case PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists:
             m_pyro->Log( "      user already on that channel " );
             break;
          case PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel:
             m_pyro->Log( "      no chat history available on that channel " );
             break;
          case PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser:
             m_pyro->Log( "      no chat history available for this user on all of his/her chat channels " );
             break;
          }
          m_pyro->Log( "     **************************************** " );
          m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      }
      break;
   }
   delete packetIn;
}

//--------------------------------------------------------------------
