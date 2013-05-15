// KhaanLogin.cpp

#include <iostream>

#include "KhaanLogin.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

bool	KhaanLogin::OnDataReceived( unsigned char* data, int length )
{
   BasePacket* packetIn = NULL;
   int offset = 0;
   PacketFactory parser;

   if( parser.Parse( data, offset, &packetIn ) == true )
   {
      int type = packetIn->packetType;
      if( type != PacketType_ServerToServerWrapper && 
         type != PacketType_GatewayWrapper )
      {
         assert( 0 );
      }

      if( packetIn->packetType == PacketType_ServerToServerWrapper )
      {
         PacketServerToServerWrapper* wrapper = reinterpret_cast< PacketServerToServerWrapper* >( packetIn );
         if( wrapper->pPacket->packetType == PacketType_ServerInformation )
         {
            PacketServerIdentifier* serverId = static_cast< PacketServerIdentifier * > ( wrapper->pPacket );

            m_serverName = serverId->serverName;
            m_serverId = serverId->serverId;
            m_isGameServer = serverId->isGameServer;
            m_isController = serverId->isController;

            cout << "Server has connected, name = '" << m_serverName << "' : " << m_serverId << endl;

            delete serverId;
         }
         else
         {
            delete wrapper->pPacket;
            delete packetIn;
         }
      }
      else if( packetIn->packetType == PacketType_GatewayWrapper )
      {
         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            const ChainLink& chain = *itOutputs++;
            ChainedInterface* interfacePtr = chain.m_interface;
            interfacePtr->AddInputChainData( packetIn, m_chainId );

         }
      }
      else
      {
         delete packetIn;
      }
   }

   return true;
}
//---------------------------------------------------------------

void   KhaanLogin ::PreCleanup()
{
   Khaan::PreCleanup();
   cout << "Server has disconnected, name = '" << m_serverName << "' : " << m_serverId << endl;

}
///////////////////////////////////////////////////////////////////
