// DiplodocusChat.cpp

#include "DiplodocusChat.h"

#include "Packets/BasePacket.h"
#include "Packets/ChatPacket.h"


int       DiplodocusChat::ProcessInputFunction()
{
   // clear out any pending inward data


   // temp
   // walk the list of inputs needing update
   // echo that packet to all connected clients

   // longer term
   // walk the list of inputs needing update
   // update one
   // process all of my new data pushing new packet into an outbox packet
   /*
      OutboxPacket
      {
         list of users to receive the packet
         packet;
      }
   */
   return 0;
}
int       DiplodocusChat::ProcessOutputFunction()
{
   if( m_isNetworkingEnabled == false )
      return 1;

   UpdateAllConnections();

   return 0;
}

void     DiplodocusChat::ClientLogin( const string& clientName, const string& uuid, const string& loginKey, int socketId )
{
   // here we would look up users expecting to login and create a mapping from the newly logged in client to the socket client
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainLink& chainedInput = *itInputs++;
	   ChainedInterface* interfacePtr = chainedInput.m_interface;
      KhaanChat* khaan = reinterpret_cast< KhaanChat* >( interfacePtr );

      PacketLogin* login = new PacketLogin;
      login->gameInstanceId = 3089;
      login->username = uuid;
      login->username = clientName;// must be looked up from the user connection
      login->uuid = uuid;
      login->loginKey = "test";

      khaan->AddOutputChainData( login );
      if( khaan->GetConnectionId() == socketId )
      {
         khaan->OnLoginMessages();
      }
   }

   MarkAllConnectionsAsNeedingUpdate( m_listOfInputs );
}

void     DiplodocusChat::ClientLogout( const string& uuid, int socketId )
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainLink& chainedInput = *itInputs++;
	   ChainedInterface* interfacePtr = chainedInput.m_interface;
      KhaanChat* khaan = reinterpret_cast< KhaanChat* >( interfacePtr );

      PacketLogoutToClient* logout = new PacketLogoutToClient;
      logout->gameInstanceId = 3089;
      logout->username = uuid;
      //logout->username = khaan->GetName();// must be looked up from the user connection
      logout->uuid = uuid;
      logout->loginKey = "test";

      khaan->AddOutputChainData( logout );
   }

   MarkAllConnectionsAsNeedingUpdate( m_listOfInputs );
}

void     DiplodocusChat::PrepForClientLogin( const string& clientName, const string& uuid, const string& loginKey, const string& possibleIpAddress )
{
}

void     DiplodocusChat::ChannelChangeReceived( const string& channel, const string& uuid )
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainLink& chainedInput = *itInputs++;
	   ChainedInterface* interfacePtr = chainedInput.m_interface;
      KhaanChat* khaan = reinterpret_cast< KhaanChat* >( interfacePtr );

      PacketChangeChatChannelToClient* chat = new PacketChangeChatChannelToClient;
      chat->chatChannel = channel;
      chat->gameInstanceId = 3089;
      chat->username = uuid;

      khaan->AddOutputChainData( chat );
   }

   MarkAllConnectionsAsNeedingUpdate( m_listOfInputs );
}

void     DiplodocusChat::ChatMessageReceived( const string& message, const string& username, const string& uuid, const string& channel )
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainLink& chainedInput = *itInputs++;
	   ChainedInterface* interfacePtr = chainedInput.m_interface;
      KhaanChat* khaan = reinterpret_cast< KhaanChat* >( interfacePtr );

      PacketChatToClient* chat = new PacketChatToClient;
      chat->gameTurn = 4;// needs to be looked up
      chat->chatChannelUuid = channel;
      chat->gameInstanceId = 3089;
      chat->message = message;
      chat->username = username;
      chat->uuid = uuid;

      khaan->AddOutputChainData( chat );
   }

   MarkAllConnectionsAsNeedingUpdate( m_listOfInputs );
}