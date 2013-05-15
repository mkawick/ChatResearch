// KhaanConnector.cpp
#include "KhaanConnector.h"
#include "DiplodocusGateway.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

//-----------------------------------------------------------------------------------------

KhaanConnector::KhaanConnector( int id, bufferevent* be ): 
      Khaan( id, be ), 
      m_numPacketsReceivedBeforeAuth( 0 ),
      m_authorizedConnection( false ),
      m_denyAllFutureData( false ),
      m_gateway( NULL )
{
}

//-----------------------------------------------------------------------------------------

KhaanConnector::~KhaanConnector()
{
}

//-----------------------------------------------------------------------------------------

void  KhaanConnector::PreCleanup()
{
   if( m_gateway )
   {
      m_gateway->ClientConnectionIsAboutToRemove( this );
   }
}

//-----------------------------------------------------------------------------------------

bool	KhaanConnector::OnDataReceived( unsigned char* data, int length )
{
   BasePacket* packetIn;
   int offset = 0;
   PacketFactory parser;
   if( m_authorizedConnection == false )
   {
      m_numPacketsReceivedBeforeAuth ++;
   }
   if( m_denyAllFutureData == true )
   {
      FlushReadBuffer();
      return false;
   }
   if( length > MaximumInputBufferSize )// special case
   {
      FlushReadBuffer();
      return false;
      // todo, log hacker
   }
   if( parser.Parse( data, offset, &packetIn ) == true )
   {
      if( IsWhiteListedIn( packetIn ) )
      {
         m_gateway->AddInputChainData( packetIn , m_connectionId );
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
   }
   return true;
}

//-----------------------------------------------------------------------------------------

bool  KhaanConnector::IsWhiteListedIn( const BasePacket* packet ) const
{
   switch( packet->packetType )
   {
   case  PacketType_Base:
      return false;
   case PacketType_Login:
      return true;
   case PacketType_Chat:
      return true;
   case PacketType_UserInfo:
      return true;
   case PacketType_UserStateChange: // from server to client, usually
      return false;
   case PacketType_DbQuery:
      return false;
   case PacketType_Gameplay:// todo, turn these off. This should never come from the client.
      return true;
   case PacketType_GatewayWrapper:
      return false;
   case PacketType_GatewayInformation: // user logged out, prepare to shutdown, etc.
      return false;
   case PacketType_ErrorReport:
      return false;
   }

   return false;
}

//-----------------------------------------------------------------------------------------
