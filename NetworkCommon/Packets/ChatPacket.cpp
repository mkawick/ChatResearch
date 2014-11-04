
// ChatPacket.cpp

#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "Serialize.h"
#include "BasePacket.h"
#include "ChatPacket.h"

///////////////////////////////////////////////////////////////

bool  PacketChatToServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, channelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameTurn, minorVersion );

   return true;
}

bool  PacketChatToServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, channelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameTurn, minorVersion );
  
   return true;
}

///////////////////////////////////////////////////////////////


bool  ChannelInfo::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, channelName, minorVersion );
   Serialize::In( data, bufferOffset, channelUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameProduct, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, numNewChats, minorVersion );
   Serialize::In( data, bufferOffset, isActive, minorVersion );

   return true;
}

bool  ChannelInfo::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, channelName, minorVersion );
   Serialize::Out( data, bufferOffset, channelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameProduct, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, numNewChats, minorVersion );
   Serialize::Out( data, bufferOffset, isActive, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  ChannelInfoFullList::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   ChannelInfo::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userList, minorVersion );

   return true;
}

bool  ChannelInfoFullList::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   ChannelInfo::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userList, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

PacketChatChannelList::~PacketChatChannelList()
{
   channelList.clear();
}

bool  PacketChatChannelList::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   channelList.SerializeIn( data, bufferOffset, minorVersion );
   //Serialize::In( data, bufferOffset, channelList, minorVersion );

   return true;
}

bool  PacketChatChannelList::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   channelList.SerializeOut( data, bufferOffset, minorVersion );
   //Serialize::Out( data, bufferOffset, channelList, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
/*
bool  PacketChangeChatChannel::SerializeIn( const U8* data, int& bufferOffset, int minorVersion ) 
{ 
   PacketChatToServer::SerializeIn( data, bufferOffset, minorVersion ); 
   Serialize::In( data, bufferOffset, chatChannelUuid );

   return true;
}

bool  PacketChangeChatChannel::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketChatToServer::SerializeOut( data, bufferOffset, minorVersion ); 
   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  PacketChatToClient::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   PacketChatToServer::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, timeStamp, minorVersion );
   Serialize::In( data, bufferOffset, userTempId, minorVersion );

   return true;
}

bool  PacketChatToClient::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   PacketChatToServer::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, timeStamp, minorVersion );
   Serialize::Out( data, bufferOffset, userTempId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatUserStatusChangeBase::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, statusChange, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );

   return true;
}

bool  PacketChatUserStatusChangeBase::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, statusChange, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
/*
bool  PacketChangeChatChannelToClient::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, userName );
   Serialize::In( data, bufferOffset, chatChannel );

   return true;
}

bool  PacketChangeChatChannelToClient::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, chatChannel, minorVersion );

   return true;
}*/

///////////////////////////////////////////////////////////////

bool  PacketChatChannelListToClient::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   int numItems = 0;
   Serialize::In( data, bufferOffset, numItems, minorVersion );
   chatChannel.clear();
   for( int i=0; i<numItems; i++ )
   {
      string newChannel;
      Serialize::In( data, bufferOffset, newChannel, minorVersion );
      chatChannel.push_back( newChannel );
   }

   return true;
}

bool  PacketChatChannelListToClient::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   int numItems = static_cast< int >( chatChannel.size(), minorVersion );
   Serialize::Out( data, bufferOffset, numItems, minorVersion );

   for( int i=0; i<numItems; i++ )
   {
      string channel = chatChannel[i];
      Serialize::Out( data, bufferOffset, channel, minorVersion );
   }

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatUserAddedToChatChannelFromGameServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, gameName, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, channelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userList, minorVersion );

   return true;
}

bool  PacketChatUserAddedToChatChannelFromGameServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, gameName, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, channelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userList, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatHistoryRequest::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, startingTimestamp, minorVersion );
   Serialize::In( data, bufferOffset, numRecords, minorVersion );   
   Serialize::In( data, bufferOffset, startingIndex, minorVersion );

   return true;
}

bool  PacketChatHistoryRequest::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, startingTimestamp, minorVersion );
   Serialize::Out( data, bufferOffset, numRecords, minorVersion );
   Serialize::Out( data, bufferOffset, startingIndex, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  ChatEntry::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, useruuid, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, userTempId, minorVersion );
   Serialize::In( data, bufferOffset, timestamp, minorVersion );
   Serialize::In( data, bufferOffset, gameTurn, minorVersion );

   return true;
}

bool  ChatEntry::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, useruuid, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, userTempId, minorVersion );
   Serialize::Out( data, bufferOffset, timestamp, minorVersion );
   Serialize::Out( data, bufferOffset, gameTurn, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatHistoryResult::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, startingTimestamp, minorVersion );
   Serialize::In( data, bufferOffset, startingIndex, minorVersion );
   Serialize::In( data, bufferOffset, chat, minorVersion );
   //chat.SerializeIn( data, bufferOffset, minorVersion );

   return true;
}

bool  PacketChatHistoryResult::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, startingTimestamp, minorVersion );
   Serialize::Out( data, bufferOffset, startingIndex, minorVersion );
   Serialize::Out( data, bufferOffset, chat, minorVersion );
   //chat.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  MissedChatChannelEntry::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, senderUuid, minorVersion );
   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, isGamechannel, minorVersion );
   Serialize::In( data, bufferOffset, senderTempId, minorVersion );


   return true;
}

bool  MissedChatChannelEntry::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, senderUuid, minorVersion );
   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, isGamechannel, minorVersion );
   Serialize::Out( data, bufferOffset, senderTempId, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatMissedHistoryResult::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, history, minorVersion );
   //history.SerializeIn( data, bufferOffset, minorVersion );
   

   return true;
}

bool  PacketChatMissedHistoryResult::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, history, minorVersion );
   //history.SerializeOut( data, bufferOffset, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatCreateChatChannel::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, name, minorVersion );

   return true;
}

bool  PacketChatCreateChatChannel::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, name, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatCreateChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, name, minorVersion );
   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, successfullyCreated, minorVersion );

   return true;
}

bool  PacketChatCreateChatChannelResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, name, minorVersion );
   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, successfullyCreated, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatCreateChatChannelFromGameServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, gameName, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, userUuidList, minorVersion );

   return true;
}

bool  PacketChatCreateChatChannelFromGameServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, gameName, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, userUuidList, minorVersion );
   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatCreateChatChannelFromGameServerResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, channelUuid, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketChatCreateChatChannelFromGameServerResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, channelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatDeleteChatChannel::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, uuid, minorVersion );

   return true;
}

bool  PacketChatDeleteChatChannel::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, uuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatDeleteChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, uuid, minorVersion );
   Serialize::In( data, bufferOffset, successfullyDeleted, minorVersion );
   Serialize::In( data, bufferOffset, numUsersRemoved, minorVersion );

   return true;
}

bool  PacketChatDeleteChatChannelResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, uuid, minorVersion );
   Serialize::Out( data, bufferOffset, successfullyDeleted, minorVersion );
   Serialize::Out( data, bufferOffset, numUsersRemoved, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatDeleteChatChannelFromGameServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, gameName, minorVersion );

   return true;
}

bool  PacketChatDeleteChatChannelFromGameServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, gameName, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatDeleteChatChannelFromGameServerResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, gameName, minorVersion );
   Serialize::In( data, bufferOffset, successfullyDeleted, minorVersion );
   Serialize::In( data, bufferOffset, numUsersRemoved, minorVersion );

   return true;
}

bool  PacketChatDeleteChatChannelFromGameServerResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, gameName, minorVersion );
   Serialize::Out( data, bufferOffset, successfullyDeleted, minorVersion );
   Serialize::Out( data, bufferOffset, numUsersRemoved, minorVersion );

   return true;
}
///////////////////////////////////////////////////////////////

bool  PacketChatInviteUserToChatChannel::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  PacketChatInviteUserToChatChannel::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatInviteUserToChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketChatInviteUserToChatChannelResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAddUserToChatChannel::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  PacketChatAddUserToChatChannel::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAddUserToChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, channelName, minorVersion );
   Serialize::In( data, bufferOffset, channelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, userName, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketChatAddUserToChatChannelResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, channelName, minorVersion );
   Serialize::Out( data, bufferOffset, channelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userName, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatAddUserToChatChannelGameServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, gameName, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  PacketChatAddUserToChatChannelGameServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, gameName, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAddUserToChatChannelGameServerResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketChatAddUserToChatChannelGameServerResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}
///////////////////////////////////////////////////////////////

bool  PacketChatRemoveUserFromChatChannel::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  PacketChatRemoveUserFromChatChannel::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRemoveUserFromChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketChatRemoveUserFromChatChannelResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRemoveUserFromChatChannelGameServer::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, gameName, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );

   return true;
}

bool  PacketChatRemoveUserFromChatChannelGameServer::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, gameName, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRemoveUserFromChatChannelGameServerResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userUuid, minorVersion );
   Serialize::In( data, bufferOffset, gameId, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketChatRemoveUserFromChatChannelGameServerResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userUuid, minorVersion );
   Serialize::Out( data, bufferOffset, gameId, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatAdminRequestChatChannelList::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, isFullList, minorVersion );

   return true;
}

bool  PacketChatAdminRequestChatChannelList::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, isFullList, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatAdminRequestChatChannelListResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, chatChannels, minorVersion );

   return true;
}

bool  PacketChatAdminRequestChatChannelListResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, chatChannels, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAdminRequestUsersList::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, isFullList, minorVersion );

   return true;
}

bool  PacketChatAdminRequestUsersList::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, isFullList, minorVersion );

   return true;
}


///////////////////////////////////////////////////////////////

bool  PacketChatAdminRequestUsersListResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, users, minorVersion );

   return true;
}

bool  PacketChatAdminRequestUsersListResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, users, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRequestChatters::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );

   return true;
}

bool  PacketChatRequestChatters::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRequestChattersResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userList, minorVersion );

   return true;
}

bool  PacketChatRequestChattersResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userList, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatEnableFiltering::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, isEnabled, minorVersion );

   return true;
}

bool  PacketChatEnableFiltering::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, isEnabled, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatEnableFilteringResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, isEnabled, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketChatEnableFilteringResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, isEnabled, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatListAllMembersInChatChannel::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );

   return true;
}

bool  PacketChatListAllMembersInChatChannel::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatListAllMembersInChatChannelResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   Serialize::In( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::In( data, bufferOffset, userList, minorVersion );

   return true;
}

bool  PacketChatListAllMembersInChatChannelResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   Serialize::Out( data, bufferOffset, chatChannelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, userList, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatAdminLoadAllChannelsResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true;
}

bool  PacketChatAdminLoadAllChannelsResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRenameChannel::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, channelUuid, minorVersion );
   Serialize::In( data, bufferOffset, newName, minorVersion );

   return true;
}

bool  PacketChatRenameChannel::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, channelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, newName, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChatRenameChannelResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );
   Serialize::In( data, bufferOffset, channelUuid, minorVersion );
   Serialize::In( data, bufferOffset, newName, minorVersion );

   return true;
}

bool  PacketChatRenameChannelResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );
   Serialize::Out( data, bufferOffset, channelUuid, minorVersion );
   Serialize::Out( data, bufferOffset, newName, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketChat_UserProfileChange::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, blockChannelInvites, minorVersion );

   return true;
}

bool  PacketChat_UserProfileChange::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, blockChannelInvites, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PacketChat_MarkChannelHistoryAsRead::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, channelUuid, minorVersion );

   return true;
}

bool  PacketChat_MarkChannelHistoryAsRead::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, channelUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////


bool  PacketChat_MarkFriendHistoryAsRead::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, friendUuid, minorVersion );

   return true;
}

bool  PacketChat_MarkFriendHistoryAsRead::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, friendUuid, minorVersion );

   return true;
}

///////////////////////////////////////////////////////////////
