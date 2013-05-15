// Khaanchat.cpp

#include <iostream>

#include "KhaanChat.h"
#include "DiplodocusChat.h"

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

///////////////////////////////////////////////////////////////////

bool	KhaanChat::OnDataReceived( unsigned char* data, int length )
{
   BasePacket* packetIn = NULL;
   int offset = 0;

   PacketFactory parser;
   if( parser.Parse( data, offset, &packetIn ) == true )
   {
      U32 type = packetIn->packetType;
      if( type != PacketType_GatewayWrapper &&
          type != PacketType_GatewayInformation && 
          type != PacketType_ServerToServerWrapper )
      {
         assert( 0 );
      }
      if( type == PacketType_GatewayWrapper )
      {
         PacketGatewayWrapper* wrapper = reinterpret_cast< PacketGatewayWrapper* >( packetIn );

         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            const ChainLink& chain = *itOutputs++;
            ChainedInterface* interfacePtr = chain.m_interface;
            //DiplodocusChat * middle = static_cast<DiplodocusChat*>( interfacePtr );

            m_connectionId = wrapper->connectionId;
            if( interfacePtr->AddInputChainData( wrapper->pPacket, m_connectionId ) == false )
            {
               delete wrapper->pPacket;
            }
         }
         delete wrapper;// will not delete the package
         packetIn = NULL;
      }
      else if( type == PacketType_ServerToServerWrapper )
      {
         PacketServerToServerWrapper* wrapper = reinterpret_cast< PacketServerToServerWrapper* >( packetIn );
         // so this is just server information about the gateway since we only accept connections about the gateway on this port
         PacketServerIdentifier* serverId = static_cast< PacketServerIdentifier * > ( wrapper->pPacket );

         m_serverName = serverId->serverName;
         m_serverId = serverId->serverId;
         m_isGameServer = serverId->isGameServer;
         m_isController = serverId->isController;

         cout << "Gateway has connected, name = '" << m_serverName << "' : " << m_serverId << endl;

         delete serverId;
         delete wrapper;// will not delete the package
         packetIn = NULL;
      }
   }
   if( packetIn )
      delete packetIn;
   return true;
}


//---------------------------------------------------------------

void  KhaanChat::PreStart()
{
   Khaan::PreStart();
   if( m_listOfOutputs.size() == 0 )
      return;

   ChainLinkIteratorType output = m_listOfOutputs.begin();

   ChainedInterface*	chain = (*output).m_interface;
   if( chain )
   {
      DiplodocusChat* chatServer = static_cast<DiplodocusChat*>( chain );
      chatServer->TurnOnUpdates( true );
   }
}
//---------------------------------------------------------------

void   KhaanChat ::PreCleanup()
{
   Khaan::PreCleanup();
   if( m_listOfOutputs.size() == 0 )
      return;

   ChainLinkIteratorType output = m_listOfOutputs.begin();

   ChainedInterface*	chain = (*output).m_interface;
   if( chain )
   {
      DiplodocusChat* chatServer = static_cast< DiplodocusChat * >( chain );
      chatServer->TurnOnUpdates( false );
   }

   cout << "Gateway has disconnected, name = '" << m_serverName << "' : " << m_serverId << endl;

}
//---------------------------------------------------------------

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////