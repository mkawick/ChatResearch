#include "KhaanChat.h"

#include <iostream>

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "KhaanChat.h"
#include "DiplodocusChat.h"

///////////////////////////////////////////////////////////////////
/*
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

            m_connectionId = wrapper->connectionId;
            if( interfacePtr->AddInputChainData( wrapper->pPacket, m_connectionId ) == false )
            {
               delete wrapper->pPacket;
               wrapper->pPacket = NULL;
            }
            else
            {
               return true;
            }
         }
      }
      else if( type == PacketType_ServerToServerWrapper )
      {
         PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packetIn );
         U32 serverId = wrapper->serverId;
         BasePacket* packet = wrapper->pPacket;

         ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
         if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
         {
            const ChainLink& chain = *itOutputs++;
            ChainType* interfacePtr = static_cast< ChainType*> ( chain.m_interface );

            m_connectionId = serverId;
            if( interfacePtr->AddInputChainData( packet, m_connectionId ) == false )
            {
               delete wrapper->pPacket;
               wrapper->pPacket = NULL;
            }
            else
            {
               return true;
            }
         }
      }
   }
   parser.CleanupPacket( packetIn );
   return true;
}*/

//---------------------------------------------------------------

void  KhaanChat::PreStart()
{
   Khaan::PreStart();
   if( m_listOfOutputs.size() == 0 )
      return;

    cout << "Gateway has connected, name = '" << m_serverName << "' : " << m_serverId << endl;
  /* ChainLinkIteratorType output = m_listOfOutputs.begin();

   IChainedInterface*	chain = (*output).m_interface;
   if( chain )
   {
      DiplodocusChat* chatServer = static_cast<DiplodocusChat*>( chain );
      //chatServer->TurnOnUpdates( true );
   }*/
}
//---------------------------------------------------------------

void   KhaanChat ::PreCleanup()
{
   Khaan::PreCleanup();
   if( m_listOfOutputs.size() == 0 )
      return;

  /* ChainLinkIteratorType output = m_listOfOutputs.begin();

   IChainedInterface*	chain = (*output).m_interface;
   if( chain )
   {
      DiplodocusChat* chatServer = static_cast< DiplodocusChat * >( chain );
      //chatServer->TurnOnUpdates( false );
   }*/

   cout << "Gateway has disconnected, name = '" << m_serverName << "' : " << m_serverId << endl;

}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////