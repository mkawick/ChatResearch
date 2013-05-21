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
      int packetType = packetIn->packetType;
      if( packetType != PacketType_ServerToServerWrapper )
      {
         assert( 0 );
      }
      
      PacketServerToServerWrapper* wrapper = reinterpret_cast< PacketServerToServerWrapper* >( packetIn );
      BasePacket* unwrappedPacket = wrapper->pPacket;

      if( unwrappedPacket->packetType == PacketType_ServerInformation )
      {
         SaveOffServerIdentification( static_cast< PacketServerIdentifier * > ( unwrappedPacket ) );        
         delete unwrappedPacket;
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

void  KhaanServerToServer :: SaveOffServerIdentification( const BasePacket* packet )
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
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      middle->ServerWasIdentified( this );

   }
}

//---------------------------------------------------------------

void   KhaanServerToServer ::PreCleanup()
{
   Khaan::PreCleanup();
   cout << "Server has disconnected, name = '" << m_serverName << "' : " << m_serverId << endl;

}
///////////////////////////////////////////////////////////////////
