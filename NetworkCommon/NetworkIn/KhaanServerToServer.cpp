// KhaanServerToServer.cpp

#include <iostream>

#include "../Packets/PacketFactory.h"
#include "../Packets/BasePacket.h"
#include "../Packets/GamePacket.h"
#include "../Packets/ServerToServerPacket.h"

#include "KhaanServerToServer.h"
#include "Diplodocus.h"
#include "DiplodocusServerToServer.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

bool	KhaanServerToServer::OnDataReceived( unsigned char* data, int length )
{
   BasePacket* packetIn = NULL;
   int offset = 0;
   PacketFactory parser;

   while( offset < length )
   {
      if( parser.Parse( data, offset, &packetIn ) == true )
      {
         int packetType = packetIn->packetType;
         if( packetType != PacketType_GatewayWrapper &&
             packetType != PacketType_GatewayInformation &&
             packetType != PacketType_ServerToServerWrapper )
         {
            assert( 0 );
         }
         
         if( packetType == PacketType_ServerToServerWrapper )
         {
            PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packetIn );

            if( wrapper->pPacket->packetType == PacketType_ServerInformation )
            {
               PacketServerIdentifier* serverId = static_cast< PacketServerIdentifier * > ( wrapper->pPacket );
               SaveOffServerIdentification( serverId );

               delete serverId;
            }
            else
            {
               m_connectionId = wrapper->serverId;
               if( PassPacketOn( wrapper, m_connectionId ) == false )// should be rare unless other servers are sending info.
               {
                  assert( 0 );
               }
            }
            delete wrapper;
            packetIn = NULL;
         }
         else if( packetType == PacketType_GatewayWrapper )// here we simply push the server packet up to the next layer
         {
            PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );
            m_connectionId = wrapper->connectionId;
            if( PassPacketOn( wrapper, m_connectionId ) == false )
            {
               delete wrapper;// will not delete the package
            }
            packetIn = NULL;
         }
         else if ( packetType == PacketType_GatewayInformation )
         {
            assert( 0 );// undone work
            ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
            if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
            {
               const ChainLink& chain = *itOutputs++;
               DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( chain.m_interface );// this should call the appropriate class based on the virtual table.

               if( middle->HandleCommandFromGateway( packetIn, m_connectionId ) == false )// needs substance
               {
                  delete packetIn;
                  packetIn = NULL;
                  assert( 0 );// incomplete
               }
            }
         }
         else
         {
            assert( 0 );
         }
      }
      if( packetIn )
      {
         delete packetIn;
      }
   }

   return true;
}

//---------------------------------------------------------------

bool  KhaanServerToServer :: PassPacketOn( BasePacket* packet, U32 connectionId )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      ChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      return interfacePtr->AddInputChainData( packet, connectionId );
   }
   return false;
}

//---------------------------------------------------------------

void  KhaanServerToServer :: SaveOffServerIdentification( const PacketServerIdentifier* packet )
{
   m_serverName = packet->serverName;
   m_serverId = packet->serverId;
   m_isGameServer = packet->isGameServer;
   m_isController = packet->isController;
   m_isGateway = packet->isGateway;

   cout << "Server has connected, name = '" << m_serverName << "' : " << m_serverId << endl;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      ChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      middle->ServerWasIdentified( this );

      if( m_isGateway )
      {
         middle->AddGatewayConnection( m_serverId );
      }
   }
}

//---------------------------------------------------------------

void   KhaanServerToServer ::PreCleanup()
{
   Khaan::PreCleanup();
   cout << "Server has disconnected, name = '" << m_serverName << "' : " << m_serverId << endl;

}
///////////////////////////////////////////////////////////////////
