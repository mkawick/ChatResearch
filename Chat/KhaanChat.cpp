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
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );

         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            const ChainLink& chain = *itOutputs++;
            ChainType* interfacePtr = static_cast< ChainType*> ( chain.m_interface );
            //DiplodocusChat * middle = static_cast<DiplodocusChat*>( interfacePtr );

            m_connectionId = wrapper->connectionId;
            if( interfacePtr->AddInputChainData( wrapper->pPacket, m_connectionId ) == false )
            {
               delete wrapper->pPacket;
               wrapper->pPacket = NULL;
            }
         }
      }
      else if( type == PacketType_ServerToServerWrapper )
      {
         PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packetIn );
         // so this is just server information about the gateway since we only accept connections about the gateway on this port
         PacketServerIdentifier* packet = static_cast< PacketServerIdentifier * > ( wrapper->pPacket );

         SaveOffServerIdentification( packet );

      }
   }
   parser.CleanupPacket( packetIn );
   return true;
}

//---------------------------------------------------------------

void  KhaanChat :: SaveOffServerIdentification( const PacketServerIdentifier* packet )
{
   //if( m_serverName == packet->serverName && m_serverId == packet->serverId ) // prevent dups from reporting.
   //   return;

   m_serverName = packet->serverName;
   m_serverId = packet->serverId;
   m_isGameServer = packet->isGameServer;
   m_isController = packet->isController;
   m_isGateway = packet->isGateway;
   U8 gameProductId = packet->gameProductId;

   std::string ip_txt( inet_ntoa( m_ipAddress.sin_addr ) );
   cout << "---------  Connected as server to " << m_serverName << "  ------------------" << endl;
   cout << "    " << ip_txt << " : " << static_cast<U32>( GetPort() ) << endl;
   cout << "    type " << static_cast<U32>( gameProductId ) << " -- server ID = " << m_serverId << endl;
   cout << "    isGame = " << boolalpha << m_isGameServer << ", isController : " << m_isController << noboolalpha << endl;
   cout << "------------------------------------------------------" << endl;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusChat * middle = static_cast<DiplodocusChat*>( interfacePtr );

      middle->ServerWasIdentified( this );

      if( m_isGateway )
      {
         middle->AddGatewayConnection( m_serverId );
      }
   }
}

//---------------------------------------------------------------

void  KhaanChat::PreStart()
{
   Khaan::PreStart();
   if( m_listOfOutputs.size() == 0 )
      return;

   ChainLinkIteratorType output = m_listOfOutputs.begin();

   IChainedInterface*	chain = (*output).m_interface;
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

   IChainedInterface*	chain = (*output).m_interface;
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