// KhaanGame.cpp

#include <iostream>

#include "KhaanGame.h"

#include "../NetworkCommon/ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "DiplodocusGame.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

//-----------------------------------------------------------------------------------------

///////////////////////////////////////////////////////////////////

bool	KhaanGame::OnDataReceived( const U8* data, int length )
{
   if( length > 2*1024 )
   {
      cout << "base game has not been extended to large data packets yet." << endl;
      assert( 0 );
   }
   BasePacket* packetIn;
   int offset = 0;
   PacketFactory parser;
   if( parser.Parse( data, offset, &packetIn, NetworkVersionMinor ) == true )
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

            m_serverName = serverId->serverName.c_str();
            m_serverId = serverId->serverId;
            m_isGameServer = serverId->isGameServer;
            m_isController = serverId->isController;

            cout << "Server has connected, name = '" << m_serverName << "' : " << m_serverId << endl;

            delete serverId;
         }
         else
         {
            assert( 0 );
         }
      }
      else
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );

         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            const ChainLink& chain = *itOutputs++;
            IChainedInterface* interfacePtr = chain.m_interface;
            DiplodocusGame * middle = static_cast<DiplodocusGame*>( interfacePtr );

            m_connectionId = wrapper->connectionId;
            middle->AddInputChainData( wrapper, m_connectionId );

         }
         delete wrapper;// will not delete the package
      }
   }
   return true;
}


//-----------------------------------------------------------------------------------------
