
#include <string>
#include <vector>
using namespace std;

#include "KhaanChat.h"
#include "DiplodocusChat.h"

#include "Packets/BasePacket.h"
#include "Packets/ChatPacket.h"

KhaanChat::KhaanChat( int id, bufferevent* be ) : Khaan( id, be ), m_connectionId( 0 )
{
   m_currentChannel = "all";
   m_chatChannels.push_back( m_currentChannel );
   m_chatChannels.push_back( "groupies" );
   m_chatChannels.push_back( "cardinals" );
   m_chatChannels.push_back( "stars" );
   m_chatChannels.push_back( "naysayers" );
   m_chatChannels.push_back( "agricola_0xdeadbeef" );
}

KhaanChat::~KhaanChat()
{
}

void  KhaanChat::OnLoginMessages()
{
   PacketChatChannelListToClient* packet = new PacketChatChannelListToClient();
   packet->chatChannel = m_chatChannels;
   AddOutputChainData( packet );
   // we might want to register for an update
}

void  KhaanChat::PreCleanup()
{
   if( m_listOfOutputs.size() == 0 )
      return;

   ChainLinkIteratorType output = m_listOfOutputs.begin();

   ChainedInterface*	chain = (*output).m_interface;
   if( chain )
   {
      DiplodocusChat* chatServer = (DiplodocusChat*) chain;
      chatServer->ClientLogout( m_uuid, m_connectionId );
   }
}

void   KhaanChat::UpdateInwardPacketList()// this class doesn't do much with the data. It's up to the derived classes to decide what to do with it
{
   int numOutputs = m_listOfOutputs.size();
   if( numOutputs > 1 )
   {
      assert( 0 );// need support for multiple outputs
   }

   ChainLinkIteratorType output = m_listOfOutputs.begin();
   if( output == m_listOfOutputs.end() )
      return;

   ChainedInterface*	chain = (*output).m_interface;
   DiplodocusChat* chatServer = (DiplodocusChat*) chain;

   if( chain )
   {
      int num = m_packetsIn.size();
      for( int i=0; i< num; i++ )
      {
         BasePacket* packet = m_packetsIn.front();
         switch( packet->packetType )
         {
         case PacketType_Login:
            {
               switch( packet->packetSubType )
               {
               case PacketLogin::LoginType_Login:
                  {
                     PacketLogin* login = (PacketLogin*) packet;
                     chatServer->ClientLogin( login->username, login->uuid, login->loginKey, m_connectionId );
                     
                     // perhaps some validation here is in order like is this user valid based on the key
                     m_username = login->username;
                     m_uuid = login->uuid;
                     m_connectionId = m_socketId;// we are cheating here.
                  }
                  break;
               case PacketLogin::LoginType_Logout:
                  {
                     PacketLogout* login = (PacketLogout*) packet;
                     chatServer->ClientLogout( m_uuid, m_connectionId );
                  }
                  break;
               }
            }
            break;

         case PacketType_Chat:
            {
               switch( packet->packetSubType )
               {
               case PacketChatToServer::ChatType_ChatToServer:
                  {
                     PacketChatToServer* chat = (PacketChatToServer*) packet;
                     chatServer->ChatMessageReceived( chat->message, m_username, m_uuid, m_currentChannel );
                  }
                  break;
               case PacketChatToServer::ChatType_ChangeChatChannel:
                  {
                     PacketChangeChatChannel* channelChange = (PacketChangeChatChannel*) packet;
                     if( channelChange->chatChannelUuid != m_currentChannel )// ignore changes to the same channel
                     {
                        chatServer->ChannelChangeReceived( channelChange->chatChannelUuid, m_uuid );
                        m_currentChannel = channelChange->chatChannelUuid;
                     }
                  }
                  break;
               }
            }
            break;
         }
      
         //chain->AddInputChainData( packet, m_connectionId );

         m_packetsIn.pop_front();
      }
   }
}

void   KhaanChat::UpdateOutwardPacketList()
{
  /* const int bufferSize = 2048;
   U8 buffer[2048];
   int offset = 0;
   chat.SerializeOut( buffer, offset );
   SendPacket( buffer, offset );

    int num = m_packetsOut.size();
    for( int i=0; i< num; i++ )
    {
       Packet* ptr = m_packetsOut.front();
       m_packetsOut.pop_front();
       delete ptr;
    }*/

   Khaan::UpdateOutwardPacketList();
}