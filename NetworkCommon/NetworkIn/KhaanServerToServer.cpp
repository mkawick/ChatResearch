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
   if( parser.Parse( data, offset, &packetIn ) == true )
   {
      int type = packetIn->packetType;
      if( type != PacketType_ServerToServerWrapper &&
         type != PacketType_Login )// from the login server
      {
         assert( 0 );
      }
      PacketServerToServerWrapper* wrapper = reinterpret_cast< PacketServerToServerWrapper* >( packetIn );

      if(wrapper->pPacket->packetType == PacketType_ServerInformation )
      {
         PacketServerIdentifier* serverId = static_cast< PacketServerIdentifier * > ( wrapper->pPacket );

         m_serverName = serverId->serverName;
         m_serverId = serverId->serverId;
         m_isGameServer = serverId->isGameServer;
         m_isController = serverId->isController;

         cout << "Server has connected, name = '" << m_serverName << "' : " << m_serverId << endl;

         delete serverId;
      }
      else// here we simply push the server packet up to the next layer
      {
         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            const ChainLink& chain = *itOutputs++;
            ChainedInterface* interfacePtr = chain.m_interface;
            DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

            m_connectionId = wrapper->serverId;
            interfacePtr->AddInputChainData( wrapper, m_connectionId );

         }
      }
      delete wrapper;// will not delete the package
      packetIn = NULL;
   }
   if( packetIn )
   {
      delete packetIn;
   }

   return true;
}
//---------------------------------------------------------------

void   KhaanServerToServer ::PreCleanup()
{
   Khaan::PreCleanup();
   cout << "Server has disconnected, name = '" << m_serverName << "' : " << m_serverId << endl;

}
///////////////////////////////////////////////////////////////////
