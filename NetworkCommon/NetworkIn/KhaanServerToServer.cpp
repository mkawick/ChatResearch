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
#include "../Utils/StringUtils.h"

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "KhaanServerToServer.h"
#include "Diplodocus.h"
#include "DiplodocusServerToServer.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

KhaanServerToServer::KhaanServerToServer() : 
                     Khaan( 0, NULL ), 
                     m_serverId( 0 ), 
                     m_serverType( 0 ), 
                     m_isGameServer( false ), 
                     m_isController( false ), 
                     m_gatewayType( PacketServerIdentifier::GatewayType_None ) 
                     {}
KhaanServerToServer::KhaanServerToServer( int id, bufferevent* be ) : 
                     Khaan( id, be ), 
                     m_serverId( 0 ), 
                     m_serverType( 0 ), 
                     m_isGameServer( false ), 
                     m_isController( false ), 
                     m_gatewayType( PacketServerIdentifier::GatewayType_None )  
                     {}

bool  KhaanServerToServer::HandleInwardSerializedPacket( const U8* data, int& offset )
{
   BasePacket* packetIn = NULL;
   PacketFactory factory;

   //cout << "KhaanServerToServer::HandleInwardSerializedPacket" << endl;

   if( factory.Parse( data, offset, &packetIn, m_versionNumberMinor ) == true )
   {
      int packetType = packetIn->packetType;
      if( packetType != PacketType_GatewayWrapper &&
          packetType != PacketType_GatewayInformation &&
          packetType != PacketType_ServerToServerWrapper )
      {
         assert( 0 );
      }
      
#ifdef VERBOSE
      LogMessage( LOG_PRIO_INFO, " KhaanServerToServer :: OnDataReceived( p=%d:%d )", packetType, packetIn->packetSubType );
#endif

      //cout << "KhaanServerToServer :: HandleInwardSerializedPacket:: lock" << endl;
      m_inputChainListMutex.lock();
         m_packetsIn.push_back( packetIn );
         m_hasPacketsReceived = true;
      m_inputChainListMutex.unlock();
      //cout << "KhaanServerToServer :: HandleInwardSerializedPacket:: unlock" << endl;
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

      LogMessage( LOG_PRIO_ERR, "**************************************************" );
      LogMessage( LOG_PRIO_ERR, "*************** critical failure *****************" );
      LogMessage( LOG_PRIO_ERR, "Cannot parse packet: %s  len=%d", temp, offset );
      LogMessage( LOG_PRIO_ERR, "[" );
     
      for(int i=0; i< strLength; i += 20 )
      {
         int remaining = strLength - i;
         if ( remaining > 10 )
            remaining = 10;

         string str;
         for( int j=0; j<remaining; j++ )
         {
            str += ToHexString( temp[i+j] ) + " ";
         }
         LogMessage( LOG_PRIO_ERR, str.c_str() );
      }
      LogMessage( LOG_PRIO_ERR, "]" );
      LogMessage( LOG_PRIO_ERR, "**************************************************" );
      LogMessage( LOG_PRIO_ERR, "**************************************************" );
      
      return false;
   }
   return true;
}

bool	KhaanServerToServer :: OnDataReceived( const U8* data, int length )
{
   //cout << "KhaanServerToServer :: OnDataReceived: " << length << endl;
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
      Serialize::In( data, offset, size, m_versionNumberMinor );
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
   if( m_hasPacketsReceived == false )
      return;

   U32 numPacketsToProcess = m_packetsIn.size();
   //cout << "KhaanServerToServer::UpdateInwardPacketList::numPacketsToProcess = " << numPacketsToProcess << endl;

   int numOutputs = static_cast< int >( m_listOfOutputs.size() );
   if( numOutputs > 1 )
   {
      assert( 0 );// need support for multiple outputs, each packet should be copied because of the memory ownership, or use shared pointers
   }

   //cout << "KhaanServerToServer :: UpdateInwardPacketList:: lock" << endl;
   m_inputChainListMutex.lock();
      deque< BasePacket* > localQueue = m_packetsIn;
      m_packetsIn.clear();
      m_hasPacketsReceived = false;
   m_inputChainListMutex.unlock();
   //cout << "KhaanServerToServer :: UpdateInwardPacketList:: unlock" << endl;

   PacketFactory factory;

   while( localQueue.size() > 0 )
   {
      BasePacket* packetIn = localQueue.front();
      int packetType = packetIn->packetType;
      
      if( packetType == PacketType_ServerToServerWrapper )
      {
         //cout << "KhaanServerToServer::UpdateInwardPacketList::PacketType_ServerToServerWrapper" << endl;
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
               LogMessage( LOG_PRIO_ERR, "ERROR: S2Sconnection packet was not dealt with 'list of outputs'" );
               LogMessage( LOG_PRIO_ERR, " Type: %d", (int) subPacket->packetType );
               LogMessage( LOG_PRIO_ERR, " SubType: %d", (int) subPacket->packetSubType );
            }
         }
      }
      else if( packetType == PacketType_GatewayWrapper )// here we simply push the server packet up to the next layer
      {
         //cout << "KhaanServerToServer::UpdateInwardPacketList::PacketType_GatewayWrapper" << endl;
         PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packetIn );
         //m_connectionId = wrapper->connectionId;
         if( PassPacketOn( wrapper, wrapper->connectionId ) == true )
         {
            packetIn = NULL;// do not delete
         }
         else
         {
            U8 type = wrapper->pPacket->packetType;
            U8 subType = wrapper->pPacket->packetSubType;
            LogMessage( LOG_PRIO_ERR, "ERROR: S2Sconnection gateway wrapped packet was not dealt with" );
            LogMessage( LOG_PRIO_ERR, " Type: %d", (int) type );
            LogMessage( LOG_PRIO_ERR, " SubType: %d", (int) subType );
         }
      }
      else if ( packetType == PacketType_GatewayInformation )
      {
         if( HandleCommandFromGateway( packetIn, m_connectionId ) == false )// needs substance
         {
            U8 type = packetIn->packetType;
            U8 subType = packetIn->packetSubType;

            LogMessage( LOG_PRIO_ERR, "ERROR: S2Sconnection packet was not dealt with as a Gateway command" );
            LogMessage( LOG_PRIO_ERR, " Type: %d", (int) type );
            LogMessage( LOG_PRIO_ERR, " SubType: %d", (int) subType );
         }
      }
      else
      {
         //assert( 0 );
         LogMessage( LOG_PRIO_ERR, "ERROR: S2Sconnection packet was not dealt with .. no handler for type" );
         LogMessage( LOG_PRIO_ERR, " Type: %d", (int) packetIn->packetType );
         LogMessage( LOG_PRIO_ERR, " SubType: %d", (int) packetIn->packetSubType );
      }

      factory.CleanupPacket( packetIn );
      localQueue.pop_front();
   }
}

//---------------------------------------------------------------

void  KhaanServerToServer :: RequestUpdate()
{
#ifdef VERBOSE
   LogMessage( LOG_PRIO_INFO, " KhaanServerToServer :: RequestUpdate(" );
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


/////////////////////////////////////////////////////////////////

bool  KhaanServerToServer :: PassPacketOn( BasePacket* packet, U32 connectionId )
{
#ifdef VERBOSE
   LogMessage( LOG_PRIO_INFO, " KhaanServerToServer :: PassPacketOn" );
#endif

   //cout << "KhaanServerToServer::PassPacketOn begin" << endl;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      bool result = middle->AddInputChainData( packet, m_serverId );
      //cout << "KhaanServerToServer::PassPacketOn finished" << endl;
      return result;
   }
   
   return false;
}

//---------------------------------------------------------------

bool  KhaanServerToServer :: HandleCommandFromGateway( BasePacket* packet, U32 connectionId )
{
   //cout << "DiplodocusServerToServer::HandleCommandFromGateway" << endl;
#ifdef VERBOSE
   LogMessage( LOG_PRIO_ERR, " KhaanServerToServer :: HandleCommandFromGateway(" );
#endif

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      return middle->HandleCommandFromGateway( packet, m_serverId );
      //return middle->HandleCommandFromGateway( packet, connectionId );
   }
   return false;
}
//---------------------------------------------------------------

void  KhaanServerToServer :: SaveOffServerIdentification( const PacketServerIdentifier* packet )
{
   //cout << "KhaanServerToServer :: SaveOffServerIdentification <<<" << endl;
   //if( m_serverName == packet->serverName && m_serverId == packet->serverId ) // prevent dups from reporting.
   //   return;

   m_serverName = packet->serverName.c_str();
   m_serverAddress = packet->serverAddress.c_str();
   m_serverId = packet->serverId;
   m_serverPort = packet->serverPort;
   m_serverType = packet->serverType; 
   m_isGameServer = packet->isGameServer;
   m_isController = packet->isController;
   m_gatewayType = packet->gatewayType;
   m_externalIpAddress = packet->externalIpAddress.c_str();
   U8 gameProductId = packet->gameProductId;

   LogMessage( LOG_PRIO_INFO, "---------  Connected as S2S server to %s  ------------------", m_serverName.c_str() );
   LogMessage( LOG_PRIO_INFO, "    IP:PORT                     %s : %d", m_serverAddress.c_str(), static_cast<U32>( m_serverPort ) );
   LogMessage( LOG_PRIO_INFO, "    Time stamp:                 %s", GetDateInUTC().c_str() );
   LogMessage( LOG_PRIO_INFO, "    type                        %d  ", static_cast<U32>( gameProductId ) );
   LogMessage( LOG_PRIO_INFO, "    server ID =                 %u", m_serverId );
   LogMessage( LOG_PRIO_INFO, "    isGame = %s, isController = %s", ConvertToTrueFalseString( m_isGameServer ), ConvertToTrueFalseString( m_isController ) );
   if( m_externalIpAddress.size() )
   {
      LogMessage( LOG_PRIO_INFO, "    has external ip address: %s", m_externalIpAddress.c_str() );
   }
   LogMessage( LOG_PRIO_INFO, "------------------------------------------------------" ) ;

   U32 numOutputs = m_listOfOutputs.size();
   //cout << "Num outputs to loop over = " << numOutputs << endl;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   if( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      //cout << "1" << endl;
      const ChainLink& chain = *itOutputs++;
      IChainedInterface* interfacePtr = chain.m_interface;
      DiplodocusServerToServer * middle = static_cast<DiplodocusServerToServer*>( interfacePtr );

      middle->ServerWasIdentified( this );
      //cout << "2" << endl;

      if( m_gatewayType != PacketServerIdentifier::GatewayType_None )
      {
         //cout << "3" << endl;
         middle->AddGatewayConnection( m_serverId );
         //cout << "4" << endl;
      }
   }
   //cout << "KhaanServerToServer :: SaveOffServerIdentification >>>" << endl;
}

//---------------------------------------------------------------

void   KhaanServerToServer ::PreCleanup()
{
   Khaan::PreCleanup();
   LogMessage( LOG_PRIO_ERR, "*********************************************" );
   LogMessage( LOG_PRIO_ERR, "    Server has disconnected, name = \'%s\' : %u", m_serverName.c_str(), m_serverId );
   LogMessage( LOG_PRIO_ERR, "    Time stamp: %s", GetDateInUTC().c_str() );
   LogMessage( LOG_PRIO_ERR, "*********************************************" );
}

///////////////////////////////////////////////////////////////////
