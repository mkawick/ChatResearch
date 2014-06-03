
// ChatPacket.cpp

#include "../ServerConstants.h"
#include "Serialize.h"
#include "BasePacket.h"
#include "ChatPacket.h"

///////////////////////////////////////////////////////////////

bool  PacketChatToServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameTurn );

   return true;
}

bool  PacketChatToServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameTurn );
  
   return true;
}

///////////////////////////////////////////////////////////////


bool  ChannelInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, channelName );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, gameProduct );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, numNewChats );
   Serialize::In( data, bufferOffset, isActive );

   return true;
}

bool  ChannelInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, channelName );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, gameProduct );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, numNewChats );
   Serialize::Out( data, bufferOffset, isActive );

   return true;
}

///////////////////////////////////////////////////////////////

bool  ChannelInfoFullList::SerializeIn( const U8* data, int& bufferOffset )
{
   ChannelInfo::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userList );

   return true;
}

bool  ChannelInfoFullList::SerializeOut( U8* data, int& bufferOffset ) const
{
   ChannelInfo::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userList );

   return true;
}

///////////////////////////////////////////////////////////////

PacketChatChannelList::~PacketChatChannelList()
{
   channelList.clear();
}

bool  PacketChatChannelList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   channelList.SerializeIn( data, bufferOffset );

   return true;
}

bool  PacketChatChannelList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   channelList.SerializeOut( data, bufferOffset );

   return true;
}

///////////////////////////////////////////////////////////////
/*
bool  PacketChangeChatChannel::SerializeIn( const U8* data, int& bufferOffset ) 
{ 
   PacketChatToServer::SerializeIn( data, bufferOffset ); 
   Serialize::In( data, bufferOffset, chatChannelUuid );

   return true;
}

bool  PacketChangeChatChannel::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketChatToServer::SerializeOut( data, bufferOffset ); 
   Serialize::Out( data, bufferOffset, chatChannelUuid );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  PacketChatToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   PacketChatToServer::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, timeStamp );
   Serialize::In( data, bufferOffset, userTempId );

   return true;
}

bool  PacketChatToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   PacketChatToServer::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, timeStamp );
   Serialize::Out( data, bufferOffset, userTempId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatUserStatusChangeBase::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, statusChange );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, chatChannelUuid );

   return true;
}

bool  PacketChatUserStatusChangeBase::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, statusChange );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, chatChannelUuid );

   return true;
}

///////////////////////////////////////////////////////////////
/*
bool  PacketChangeChatChannelToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, chatChannel );

   return true;
}

bool  PacketChangeChatChannelToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, chatChannel );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  PacketChatChannelListToClient::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   int numItems = 0;
   Serialize::In( data, bufferOffset, numItems );
   chatChannel.clear();
   for( int i=0; i<numItems; i++ )
   {
      string newChannel;
      Serialize::In( data, bufferOffset, newChannel );
      chatChannel.push_back( newChannel );
   }

   return true;
}

bool  PacketChatChannelListToClient::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   int numItems = static_cast< int >( chatChannel.size() );
   Serialize::Out( data, bufferOffset, numItems );

   for( int i=0; i<numItems; i++ )
   {
      string channel = chatChannel[i];
      Serialize::Out( data, bufferOffset, channel );
   }

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatUserAddedToChatChannelFromGameServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, gameName );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, userList );

   return true;
}

bool  PacketChatUserAddedToChatChannelFromGameServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, gameName );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, userList );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatHistoryRequest::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, startingTimestamp );
   Serialize::In( data, bufferOffset, numRecords );   
   Serialize::In( data, bufferOffset, startingIndex );

   return true;
}

bool  PacketChatHistoryRequest::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, startingTimestamp );
   Serialize::Out( data, bufferOffset, numRecords );
   Serialize::Out( data, bufferOffset, startingIndex );

   return true;
}

///////////////////////////////////////////////////////////////

bool  ChatEntry::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, useruuid );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, userTempId );
   Serialize::In( data, bufferOffset, timestamp );
   Serialize::In( data, bufferOffset, gameTurn );

   return true;
}

bool  ChatEntry::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, useruuid );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, userTempId );
   Serialize::Out( data, bufferOffset, timestamp );
   Serialize::Out( data, bufferOffset, gameTurn );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatHistoryResult::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, startingTimestamp );
   Serialize::In( data, bufferOffset, startingIndex );
   Serialize::In( data, bufferOffset, chat );

   return true;
}

bool  PacketChatHistoryResult::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, startingTimestamp );
   Serialize::Out( data, bufferOffset, startingIndex );
   Serialize::Out( data, bufferOffset, chat );

   return true;
}


///////////////////////////////////////////////////////////////

bool  MissedChatChannelEntry::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, senderUuid );
   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, isGamechannel );
   Serialize::In( data, bufferOffset, senderTempId );


   return true;
}

bool  MissedChatChannelEntry::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, senderUuid );
   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, isGamechannel );
   Serialize::Out( data, bufferOffset, senderTempId );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatMissedHistoryResult::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, history );

   return true;
}

bool  PacketChatMissedHistoryResult::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, history );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatCreateChatChannel::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, name );

   return true;
}

bool  PacketChatCreateChatChannel::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, name );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatCreateChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, name );
   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, successfullyCreated );

   return true;
}

bool  PacketChatCreateChatChannelResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, name );
   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, successfullyCreated );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatCreateChatChannelFromGameServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, gameName );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, userUuidList );

   return true;
}

bool  PacketChatCreateChatChannelFromGameServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, gameName );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, userUuidList );
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatCreateChatChannelFromGameServerResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketChatCreateChatChannelFromGameServerResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, success );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatDeleteChatChannel::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, uuid );

   return true;
}

bool  PacketChatDeleteChatChannel::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, uuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatDeleteChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, uuid );
   Serialize::In( data, bufferOffset, successfullyDeleted );
   Serialize::In( data, bufferOffset, numUsersRemoved );

   return true;
}

bool  PacketChatDeleteChatChannelResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, uuid );
   Serialize::Out( data, bufferOffset, successfullyDeleted );
   Serialize::Out( data, bufferOffset, numUsersRemoved );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatDeleteChatChannelFromGameServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, gameName );

   return true;
}

bool  PacketChatDeleteChatChannelFromGameServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, gameName );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatDeleteChatChannelFromGameServerResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, gameName );
   Serialize::In( data, bufferOffset, successfullyDeleted );
   Serialize::In( data, bufferOffset, numUsersRemoved );

   return true;
}

bool  PacketChatDeleteChatChannelFromGameServerResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, gameName );
   Serialize::Out( data, bufferOffset, successfullyDeleted );
   Serialize::Out( data, bufferOffset, numUsersRemoved );

   return true;
}
///////////////////////////////////////////////////////////////

bool  PacketChatInviteUserToChatChannel::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketChatInviteUserToChatChannel::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatInviteUserToChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketChatInviteUserToChatChannelResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, success );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAddUserToChatChannel::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketChatAddUserToChatChannel::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAddUserToChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, channelName );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketChatAddUserToChatChannelResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, channelName );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, userName );
   Serialize::Out( data, bufferOffset, success );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatAddUserToChatChannelGameServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, gameName );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketChatAddUserToChatChannelGameServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, gameName );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAddUserToChatChannelGameServerResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketChatAddUserToChatChannelGameServerResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, success );

   return true;
}
///////////////////////////////////////////////////////////////

bool  PacketChatRemoveUserFromChatChannel::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketChatRemoveUserFromChatChannel::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRemoveUserFromChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketChatRemoveUserFromChatChannelResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, success );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRemoveUserFromChatChannelGameServer::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, gameName );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, userUuid );

   return true;
}

bool  PacketChatRemoveUserFromChatChannelGameServer::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, gameName );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, userUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRemoveUserFromChatChannelGameServerResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userUuid );
   Serialize::In( data, bufferOffset, gameId );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketChatRemoveUserFromChatChannelGameServerResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userUuid );
   Serialize::Out( data, bufferOffset, gameId );
   Serialize::Out( data, bufferOffset, success );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatAdminRequestChatChannelList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, isFullList );

   return true;
}

bool  PacketChatAdminRequestChatChannelList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, isFullList );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatAdminRequestChatChannelListResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, chatChannels );

   return true;
}

bool  PacketChatAdminRequestChatChannelListResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, chatChannels );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAdminRequestUsersList::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, isFullList );

   return true;
}

bool  PacketChatAdminRequestUsersList::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, isFullList );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatAdminRequestUsersListResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, users );

   return true;
}

bool  PacketChatAdminRequestUsersListResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, users );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRequestChatters::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );

   return true;
}

bool  PacketChatRequestChatters::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRequestChattersResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userList );

   return true;
}

bool  PacketChatRequestChattersResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userList );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatEnableFiltering::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, isEnabled );

   return true;
}

bool  PacketChatEnableFiltering::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, isEnabled );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatEnableFilteringResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, isEnabled );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketChatEnableFilteringResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, isEnabled );
   Serialize::Out( data, bufferOffset, success );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatListAllMembersInChatChannel::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );

   return true;
}

bool  PacketChatListAllMembersInChatChannel::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatListAllMembersInChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );

   Serialize::In( data, bufferOffset, chatChannelUuid );
   Serialize::In( data, bufferOffset, userList );

   return true;
}

bool  PacketChatListAllMembersInChatChannelResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );

   Serialize::Out( data, bufferOffset, chatChannelUuid );
   Serialize::Out( data, bufferOffset, userList );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAdminLoadAllChannelsResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, success );

   return true;
}

bool  PacketChatAdminLoadAllChannelsResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, success );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRenameChannel::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, newName );

   return true;
}

bool  PacketChatRenameChannel::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, newName );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRenameChannelResponse::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, success );
   Serialize::In( data, bufferOffset, channelUuid );
   Serialize::In( data, bufferOffset, newName );

   return true;
}

bool  PacketChatRenameChannelResponse::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, success );
   Serialize::Out( data, bufferOffset, channelUuid );
   Serialize::Out( data, bufferOffset, newName );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketChat_UserProfileChange::SerializeIn( const U8* data, int& bufferOffset )
{
   BasePacket::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, blockChannelInvites );

   return true;
}

bool  PacketChat_UserProfileChange::SerializeOut( U8* data, int& bufferOffset ) const
{
   BasePacket::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, blockChannelInvites );

   return true;
}

///////////////////////////////////////////////////////////////
