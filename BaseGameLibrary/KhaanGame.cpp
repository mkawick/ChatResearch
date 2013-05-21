// KhaanGame.cpp

#include <iostream>

#include "KhaanGame.h"
#include "DiplodocusGame.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

//-----------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////

bool	KhaanGame::OnDataReceived( unsigned char* data, int length )
{
   BasePacket* packetIn;
   int offset = 0;
   PacketFactory parser;
   if( parser.Parse( data, offset, &packetIn ) == true )
   {
      int type = packetIn->packetType;
      if( type != PacketType_GatewayWrapper &&
          type != PacketType_GatewayInformation &&
          type != PacketType_ServerToServerWrapper )
      {
         assert( 0 );
      }

      if( type == PacketType_ServerToServerWrapper )
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
            assert( 0 );
         }
         delete wrapper;
      }
      else
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );

         bool wasHandled = false;
         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            const ChainLink& chain = *itOutputs++;
            ChainedInterface* interfacePtr = chain.m_interface;
            DiplodocusGame * middle = static_cast<DiplodocusGame*>( interfacePtr );

            m_connectionId = wrapper->connectionId;
            if( interfacePtr->AddInputChainData( wrapper, m_connectionId ) == true )
            {
               wasHandled = true; 
               //break;
            }
         }
         if( wasHandled == false )
         {
            delete wrapper->pPacket;
            delete wrapper;
         }
      }
   }
   return true;
}


//---------------------------------------------------------------

void  KhaanGame :: SaveOffServerIdentification( const PacketServerIdentifier* packet )
{
   const PacketServerIdentifier* serverId = static_cast< const PacketServerIdentifier * > ( packet );

   m_serverName = serverId->serverName;
   m_serverId = serverId->serverId;
   m_isGameServer = serverId->isGameServer;
   m_isController = serverId->isController;

   cout << "Server has connected, name = '" << m_serverName << "' : " << m_serverId << endl;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      ChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusGame * middle = static_cast<DiplodocusGame*>( interfacePtr );

      middle->ServerWasIdentified( this );
      middle->AddGatewayConnection( m_serverId );

   }
}

//-----------------------------------------------------------------------------------------
