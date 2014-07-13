// KhaanServerToServer.cpp

#include <iostream>

#include "../ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "../Packets/PacketFactory.h"
#include "../Packets/BasePacket.h"
#include "../Packets/GamePacket.h"

#include "../Utils/CommandLineParser.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "KhaanServerToServer.h"
#include "Diplodocus.h"
#include "DiplodocusServerToServer.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

bool  KhaanServerToServer::HandleInwardSerializedPacket( const U8* data, int& offset )
{
   BasePacket* packetIn = NULL;
   PacketFactory factory;

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

      m_inputChainListMutex.lock();
      m_packetsIn.push_back( packetIn );
      m_inputChainListMutex.unlock();
      RequestUpdate();
   } 
   else
   {
      const int maxStrLen = 200;
      char temp[ maxStrLen+1 ];
      int strLength = offset;
      if( strLength > maxStrLen )
         strLength = maxStrLen;

      memcpy( temp, data, strLength );
      temp[ maxStrLen ] = 0;

      cout << "**************************************************" << endl;
      cout << "*************** critical failure *****************" << endl;
      cout << "Cannot parse packet: " << temp << " len=" << offset << endl;
      cout << hex << "[" << endl;

      for(int i=0; i< strLength; i += 20 )
      {
         int remaining = strLength - i;
         if ( remaining > 10 )
            remaining = 10;

         for( int j=0; j<remaining; j++ )
         {
            cout << temp[ i+j ] << " ";
         }
         cout << endl;
      }
      cout << "]" << dec << endl;
      cout << "**************************************************" << endl;
      cout << "**************************************************" << endl;
      
      return false;
   }
   return true;
}

bool	KhaanServerToServer :: OnDataReceived( const U8* data, int length )
{
   int offset = 0;
   if( m_isExpectingMoreDataInPreviousPacket )
   {
      int numBytesToCopy = length;
      if( m_expectedBytesReceivedSoFar + numBytesToCopy < m_expectedBytes )
      {
         // here we can only store the data and then return because we still do 
         // not have the full packet yet.
         memcpy( m_tempBuffer+m_expectedBytesReceivedSoFar, data, numBytesToCopy );
         m_expectedBytesReceivedSoFar += numBytesToCopy;
         return false;
      }
      else if( m_expectedBytesReceivedSoFar + length > m_expectedBytes )
      {
         numBytesToCopy = m_expectedBytes - m_expectedBytesReceivedSoFar;
         memcpy( m_tempBuffer + m_expectedBytesReceivedSoFar, data, numBytesToCopy );
         m_expectedBytesReceivedSoFar = m_expectedBytes;

         // we have more bytes as part of a following packet following.
         data += numBytesToCopy;// offset the pointer.. everything should be magical after this
         length -= numBytesToCopy; // this may mean setting up another partial packet.
         int tempOffset = 0;
         HandleInwardSerializedPacket( m_tempBuffer, tempOffset );
      }
      else
      {
         memcpy( m_tempBuffer + m_expectedBytesReceivedSoFar, data, length );
         int tempOffset = 0;
         HandleInwardSerializedPacket( m_tempBuffer, tempOffset );
         m_expectedBytes = 0;
         m_expectedBytesReceivedSoFar = 0;
         length = 0;
      }
      
      m_isExpectingMoreDataInPreviousPacket = false;
   }


   while( offset < length )
   {
      /// before we parse, let's pull off the first two bytes
      U16 size = 0;
      Serialize::In( data, offset, size );
      if( offset + size > length )
      {
         m_isExpectingMoreDataInPreviousPacket = true;
         m_expectedBytes = size;

         m_expectedBytesReceivedSoFar = length - offset;
         memcpy( m_tempBuffer, data+offset, m_expectedBytesReceivedSoFar );
         return false;
      }

      if( HandleInwardSerializedPacket( data, offset ) == false )
      {
         offset = length;// break out of loop.
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

   m_inputChainListMutex.lock();
   deque< BasePacket* > localQueue = m_packetsIn;
   m_packetsIn.clear();
   m_inputChainListMutex.unlock();

   PacketFactory factory;

   while( localQueue.size() > 0 )
   {
      BasePacket* packetIn = localQueue.front();
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
               // assert( 0 );
               cout << "ERROR: S2Sconnection packet was not dealt with 'list of outputs'" << endl;
               cout << " Type: " << (int) subPacket->packetType << endl;
               cout << " SubType: " << (int) subPacket->packetSubType << endl;
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
         else
         {
            U8 type = wrapper->pPacket->packetType;
            U8 subType = wrapper->pPacket->packetSubType;
            cout << "ERROR: S2Sconnection gateway wrapped packet was not dealt with" << endl;
            cout << " Type: " << (int) type << endl;
            cout << " SubType: " << (int) subType << endl;
         }
      }
      else if ( packetType == PacketType_GatewayInformation )
      {
         if( HandleCommandFromGateway( packetIn, m_connectionId ) == false )// needs substance
         {
            U8 type = packetIn->packetType;
            U8 subType = packetIn->packetSubType;

            cout << "ERROR: S2Sconnection packet was not dealt with as a Gateway command" << endl;
            cout << " Type: " << (int) type << endl;
            cout << " SubType: " << (int) subType << endl;
         }
      }
      else
      {
         //assert( 0 );
         cout << "ERROR: S2Sconnection packet was not dealt with .. no handler for type" << endl;
         cout << " Type: " << (int) packetIn->packetType << endl;
         cout << " SubType: " << (int) packetIn->packetSubType << endl;
      }

      factory.CleanupPacket( packetIn );
      localQueue.pop_front();
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
   m_serverType = packet->serverType; 
   m_isGameServer = packet->isGameServer;
   m_isController = packet->isController;
   m_gatewayType = packet->gatewayType;
   m_externalIpAddress = packet->externalIpAddress;
   U8 gameProductId = packet->gameProductId;

   cout << "---------  Connected as S2S server to " << m_serverName << "  ------------------" << endl;
   cout << "    " << m_serverAddress << " : " << static_cast<U32>( m_serverPort ) << endl;
   cout << "    Time stamp: " << GetDateInUTC() << endl;
   cout << "    type " << static_cast<U32>( gameProductId ) << " -- server ID = " << m_serverId << endl;
   cout << "    isGame = " << boolalpha << m_isGameServer << ", isController : " << m_isController << noboolalpha << endl;
   if( m_externalIpAddress.size() )
   {
      cout << "    has external ip address: " << m_externalIpAddress << endl;
   }
   cout << "------------------------------------------------------" << endl;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      middle->ServerWasIdentified( this );

      if( m_gatewayType != PacketServerIdentifier::GatewayType_None )
      {
         middle->AddGatewayConnection( m_serverId );
      }
   }
}

//---------------------------------------------------------------

void   KhaanServerToServer ::PreCleanup()
{
   Khaan::PreCleanup();
   cout << "*********************************************" << endl;
   cout << "    Server has disconnected, name = '" << m_serverName << "' : " << m_serverId << endl;
   cout << "    Time stamp: " << GetDateInUTC() << endl;
   cout << "*********************************************" << endl;
}

///////////////////////////////////////////////////////////////////
