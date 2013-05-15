#pragma once

#include "Diplodocus.h"
#include "KhaanChat.h"


class DiplodocusChat: public Diplodocus < KhaanChat, BasePacketChainHandler >
{
   std::list< KhaanChat* > m_chatters;
public:
   DiplodocusChat( string serverName, ServerType type = ServerType_Chat ) : Diplodocus( serverName, type ){}
   ~DiplodocusChat(){}

   int      ProcessInputFunction();
   int      ProcessOutputFunction();

   void     ClientLogin( const string& clientName, const string& uuid, const string& loginKey, int socketId );
   void     ClientLogout( const string& uuid, int socketId );

   void     PrepForClientLogin( const string& clientName, const string& uuid, const string& loginKey, const string& possibleIpAddress );


   void     ChannelChangeReceived( const string& channel, const string& uuid );
   void     ChatMessageReceived( const string& message, const string& clientName, const string& uuid, const string& channel );
};