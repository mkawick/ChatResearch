//KhaanProtected.cpp
#include "KhaanProtected.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;
#include <iostream>
using namespace std;

#include "../Packets/LoginPacket.h"
#include "../Packets/PacketFactory.h"
#include "../Utils/CommandLineParser.h"
#include "../Logging/server_log.h"

//-----------------------------------------------------------------------------------------

KhaanProtected::KhaanProtected( int id, bufferevent* be ): Khaan( id, be ),
                  m_mainOutputChain( NULL )
{
}

KhaanProtected::~KhaanProtected()
{
}

void  KhaanProtected::SendPacketToApp( BasePacket* packet )
{
   if( m_mainOutputChain )
   {
      m_mainOutputChain->AddInputChainData( packet, m_connectionId );
      return;
   }

   BaseOutputContainer::iterator itOutput = m_listOfInputs.begin();
   if( itOutput != m_listOfInputs.end() )
   {
      //IChainedInterface* outputPtr = (*itOutput).m_interface;
      //FruitadensGateway* fruity = static_cast< FruitadensGateway* >( outputPtr );
      ChainType* chain = static_cast< ChainType* >( (*itOutput).m_interface );
      chain->AddInputChainData( packet, m_connectionId );
   }
   else
   {
      LogMessage( LOG_PRIO_ERR, "Major problem: KhaanProtected::SendPacketToApp is not connected to output" );
      PacketFactory factory;
      factory.CleanupPacket( packet );
   }
}

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

bool  KhaanProtected::HandleInwardSerializedPacket( const U8* data, int& offset )
{
   bool result = false;
   BasePacket* packetIn;

   PacketFactory parser;
   try 
   {
      result = parser.Parse( data, offset, &packetIn, m_versionNumberMinor );
      if( packetIn != NULL )
      {
         TrackInwardPacketType( packetIn );
      }
   }
   catch( ... )
   {
      Log( "parsing in KhaanGateway threw an exception" );
      DenyAllFutureData ();
      return false;
   }

   if( result == true )
   {
      bool packetCleanupRequired = false;
      if( IsAuthorized() == false )
      {
         if( IsHandshaking( packetIn ) == true )
         {
            packetCleanupRequired = true;
         }
      }
      if( packetCleanupRequired == false )// we still have work to do
      {
         if( IsWhiteListedIn( packetIn ) || HasPermission( packetIn ) )
         {
            //m_gateway->AddInputChainData( packetIn, m_connectionId );
            SendPacketToApp( packetIn );
            SetupOutputDelayTimestamp();
         }
         else
         {
            FlushReadBuffer();// apparently bad data, let's prevent buffer overruns, etc
            packetCleanupRequired = true;
         }
      }

      if( packetCleanupRequired )
      {
         parser.CleanupPacket( packetIn );
      }
   }
   else
   {
      FlushReadBuffer();// apparently bad data, let's prevent buffer overruns, etc
      return false;;
   }

   return true;
}

void  DumpBadData( const U8* buffer, U32 numReceivedBytes, U32 bytesParsed )
{
   LogMessage( LOG_PRIO_ERR, "error on Gateway receiving packet info" );
   LogMessage( LOG_PRIO_ERR, "size of parsed packet: %d > length of bytes input: %d", bytesParsed, numReceivedBytes );
   LogMessage( LOG_PRIO_ERR, "This is clearly a hacker packet " );
   LogMessage( LOG_PRIO_ERR, "Terminating all future data" );

   if( numReceivedBytes > 100 )
      numReceivedBytes = 100;

   int bytesPerRow = 10;
   int numRows = numReceivedBytes / bytesPerRow + 1;
   
   for( int i=0; i<numRows; i++ )
   {
      string str;
      for( int j=0; j< bytesPerRow; j++ )
      {
         str += " " + ToHexString( *buffer );
         buffer ++;
      }
      
      LogMessage( LOG_PRIO_ERR, str.c_str() );
   }

}

//-----------------------------------------------------------------------------------------

bool	KhaanProtected::OnDataReceived( const U8* data, int length )
{
   if( m_isInTelnetMode== false && length < BasePacket::GetSize() )// why not sizeof? Because of the virtual pointer
      //&& m_authorizedConnection )
   {
      m_isInTelnetMode = true;
      SendTelnetInstructions();
   }

   if( m_isInTelnetMode == true )
   {
      return HandleTelnetModeData( data, length );
   }

   if( m_denyAllFutureData == true )
   {
      FlushReadBuffer();
      return false;
   }

   if( length > MaxBufferSize )// special case
   {
      FlushReadBuffer();

      DenyAllFutureData ();
      Log( "Gateway: hacker alert. Packet length is far too long", 3 );
      return false;
   }

   if( IsAuthorized() == false )
   {
      /*if( IsHandshaking( data, length ) == true )
      {
         return false;
      }*/

      U16 size = 0;
      int offset = 0;
      Serialize::In( data, offset, size, m_versionNumberMinor );
      if( size > length )
      { 
         DumpBadData( data, length, size );
         
         m_denyAllFutureData = true;
         return false;
      }
      //assert( size <= length );

      if( IsPacketSafe( data, offset ) == false )
      {
         return false;
      }      
   }

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

   bool result = false;
   
   //BasePacket* packetIn;
   // catch bad packets, buffer over runs, or other badly formed data.
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

      HandleInwardSerializedPacket( data, offset );
   }
   return true;
}
