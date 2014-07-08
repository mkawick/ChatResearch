// KhaanConnector.cpp

#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "KhaanConnector.h"
#include "DiplodocusLoadBalancer.h"

#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

enum GatewayConstants
{
   MaximumInputBufferSize = 2048
};

//-----------------------------------------------------------------------------------------

KhaanConnector::KhaanConnector( int id, bufferevent* be ): 
      Khaan( id, be ), 
      m_loadBalancer( NULL ),
      m_denyAllFutureData( false ),
      m_markedToBeCleanedup( false )
{
}

//-----------------------------------------------------------------------------------------


KhaanConnector::~KhaanConnector(void)
{
}
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------

bool	KhaanConnector::OnDataReceived( unsigned char* data, int length )
{
   if( m_denyAllFutureData == true )
   {
      FlushReadBuffer();
      return false;
   }

   BasePacket* packetIn;
   int offset = 0;
   PacketFactory parser;

   if( length > MaximumInputBufferSize )// special case
   {
      FlushReadBuffer();

      m_denyAllFutureData = true;
      Log( "Gateway: hacker alert. Packet length is far too long", 3 );
      return false;
   }

   

   bool result = false;
   // catch bad packets, buffer over runs, or other badly formed data.
   while( offset < length )
   {
      /// before we parse, let's pull off the first two bytes
      U16 size = 0;
      Serialize::In( data, offset, size );

      if( size > length )
      { 
         cout << "error on Gateway receiving packet info" << endl;
         cout << "size : " << size << " > length : " << length << endl;
      }
      assert( size <= length );

      try 
      {
         result = parser.Parse( data, offset, &packetIn );
      }
      catch( ... )
      {
         Log( "parsing in KhaanConnector threw an exception" );
         m_denyAllFutureData = true;
         break;
      }

      if( result == true )
      {
         if( IsWhiteListedIn( packetIn ) )
         {
            m_loadBalancer->AddInputChainData( packetIn , m_connectionId );
         }
         else
         {
            FlushReadBuffer();// apparently bad data, let's prevent buffer overruns, etc
            delete packetIn;
         }
      }
      else
      {
         FlushReadBuffer();// apparently bad data, let's prevent buffer overruns, etc
         break;
      }
   }
   return true;
}

//-----------------------------------------------------------------------------------------

bool  KhaanConnector::IsWhiteListedIn( const BasePacket* packet ) const
{
   switch( packet->packetType )
   {
   case  PacketType_Base:
      {
         if( packet->packetSubType == BasePacket::BasePacket_RerouteRequest )
         {
            return true;
         }
         return false;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------------------