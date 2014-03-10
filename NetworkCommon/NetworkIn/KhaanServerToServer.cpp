// KhaanServerToServer.cpp

#include <iostream>

#include "../Packets/PacketFactory.h"
#include "../Packets/BasePacket.h"
#include "../Packets/GamePacket.h"
#include "../Packets/ServerToServerPacket.h"
#include "../Utils/CommandLineParser.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "KhaanServerToServer.h"
#include "Diplodocus.h"
#include "DiplodocusServerToServer.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

bool	KhaanServerToServer :: OnDataReceived( unsigned char* data, int length )
{
   BasePacket* packetIn = NULL;
   int offset = 0;
   PacketFactory factory;

   while( offset < length )
   {
      if( factory.Parse( data, offset, &packetIn ) == true )
      {
         int packetType = packetIn->packetType;
         if( packetType != PacketType_GatewayWrapper &&
             packetType != PacketType_GatewayInformation &&
             packetType != PacketType_ServerToServerWrapper )
         {
            assert( 0 );
         }
         
#ifdef VERBOSE
         cout<<  " KhaanServerToServer :: OnDataReceived( p=" << packetType << ":" << packetIn->packetSubType << ")"<< endl;
#endif

         Threading::MutexLock  locker( m_inputChainListMutex );
         m_packetsIn.push_back( packetIn );
         RequestUpdate();
      }         
   }

   return true;
}


//------------------------------------------------------------------------------

void	KhaanServerToServer :: UpdateInwardPacketList()
{
   if( m_packetsIn.size() == 0 )
      return;

   int numOutputs = static_cast< int >( m_listOfOutputs.size() );
   if( numOutputs > 1 )
   {
      assert( 0 );// need support for multiple outputs, each packet should be copied because of the memory ownership, or use shared pointers
   }

   PacketFactory factory;

   while( m_packetsIn.size() > 0 )
   {
      BasePacket* packetIn = m_packetsIn.front();
      int packetType = packetIn->packetType;
      
      if( packetType == PacketType_ServerToServerWrapper )
      {
         PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packetIn );
         BasePacket* subPacket = wrapper->pPacket;

         if( subPacket->packetType == PacketType_ServerInformation &&
            subPacket->packetSubType == PacketServerConnectionInfo::PacketServerIdentifier_TypicalInfo )
         {
            PacketServerIdentifier* serverId = static_cast< PacketServerIdentifier * > ( subPacket );
            SaveOffServerIdentification( serverId );
         }
         else
         {
            m_connectionId = wrapper->serverId;
           /* if( m_connectionId == 0 )// badly formed
            {
               m_connectionId = subPacket->serverId;
            }*/
            if( PassPacketOn( wrapper, m_connectionId ) == true )
            {
               packetIn = NULL;// do not delete
            }
            else
            {
               assert( 0 );
            }
         }
      }
      else if( packetType == PacketType_GatewayWrapper )// here we simply push the server packet up to the next layer
      {
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );
         m_connectionId = wrapper->connectionId;
         if( PassPacketOn( wrapper, m_connectionId ) == true )
         {
            packetIn = NULL;// do not delete
         }
      }
      else if ( packetType == PacketType_GatewayInformation )
      {
         if( HandleCommandFromGateway( packetIn, m_connectionId ) == false )// needs substance
         {
            assert( 0 );// incomplete
         }
      }
      else
      {
         assert( 0 );
      }

      factory.CleanupPacket( packetIn );
      m_packetsIn.pop_front();
   }
}

//---------------------------------------------------------------

void  KhaanServerToServer :: RequestUpdate()
{
#ifdef VERBOSE
   cout << " KhaanServerToServer :: RequestUpdate(" << endl;
#endif

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
      ThreadEvent te;
      te.type = ThreadEvent_NeedsService;
      te.identifier = m_chainId;
      static_cast< ChainType*> ( interfacePtr )->PushInputEvent( &te );
   }
   
}

//---------------------------------------------------------------

bool  KhaanServerToServer :: PassPacketOn( BasePacket* packet, U32 connectionId )
{
#ifdef VERBOSE
   cout << " KhaanServerToServer :: PassPacketOn(" << endl;
#endif

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      return middle->AddInputChainData( packet, connectionId );
   }
   return false;
}

//---------------------------------------------------------------

bool  KhaanServerToServer :: HandleCommandFromGateway( BasePacket* packet, U32 connectionId )
{
#ifdef VERBOSE
   cout << " KhaanServerToServer :: HandleCommandFromGateway(" << endl;
#endif

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      return middle->HandleCommandFromGateway( packet, connectionId );
   }
   return false;
}
//---------------------------------------------------------------

void  KhaanServerToServer :: SaveOffServerIdentification( const PacketServerIdentifier* packet )
{
   //if( m_serverName == packet->serverName && m_serverId == packet->serverId ) // prevent dups from reporting.
   //   return;

   m_serverName = packet->serverName;
   m_serverAddress = packet->serverAddress;
   m_serverId = packet->serverId;
   m_serverPort = packet->serverPort;
   m_isGameServer = packet->isGameServer;
   m_isController = packet->isController;
   m_isGateway = packet->isGateway;
   U8 gameProductId = packet->gameProductId;

   cout << "---------  Connected as server to " << m_serverName << "  ------------------" << endl;
   cout << "    " << m_serverAddress << " : " << static_cast<U32>( m_serverPort ) << endl;
   cout << "    type " << static_cast<U32>( gameProductId ) << " -- server ID = " << m_serverId << endl;
   cout << "    isGame = " << boolalpha << m_isGameServer << ", isController : " << m_isController << noboolalpha << endl;
   cout << "------------------------------------------------------" << endl;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
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
