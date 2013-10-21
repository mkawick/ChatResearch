// KhaanConnector.cpp
#include "KhaanConnector.h"
#include "DiplodocusGateway.h"

#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

//-----------------------------------------------------------------------------------------

KhaanConnector::KhaanConnector( int id, bufferevent* be ): 
      Khaan( id, be ), 
      m_numPacketsReceivedBeforeAuth( 0 ),
      m_authorizedConnection( false ),
      m_denyAllFutureData( false ),
      m_gateway( NULL ),
      m_adminLevel( 0 )
{
   m_randomNumberOfPacketsBeforeLogin = 30 + rand() % 20;
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
      m_gateway->InputRemovalInProgress( this );
   }
}

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

   if( m_authorizedConnection == false )
   {
      // before we parse, which is potentially dangerous, we will do a quick check
      BasePacket testPacket;
      parser.SafeParse( data, offset, testPacket );
      if( testPacket.packetType != PacketType_Login || 
         ( testPacket.packetSubType != PacketLogin::LoginType_Login && testPacket.packetSubType != PacketLogin::LoginType_CreateAccount ) )
      {
         m_numPacketsReceivedBeforeAuth ++;
         if( m_numPacketsReceivedBeforeAuth > m_randomNumberOfPacketsBeforeLogin )
         {
            m_denyAllFutureData = true;
            Log( "Gateway: hacker alert. Too many packet received without a login.", 3 );
            CloseConnection();
         }

         // we never proceed beyond here. If you are not authorized and you are not passing login packets, you get no pass, whatsoever.
         return false;
      }
   }

   

   bool result = false;
   // catch bad packets, buffer over runs, or other badly formed data.
   while( offset < length )
   {
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
         if( IsWhiteListedIn( packetIn ) || HasPermission( packetIn ) )
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
      return false;
   case PacketType_Login:
      return true;
   case PacketType_Chat:
      return true;
   case PacketType_UserInfo:
      return true;
    case PacketType_Contact:
      return true;
   case PacketType_Asset:
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

bool  KhaanConnector::HasPermission( const BasePacket* packet ) const
{
   if( m_adminLevel > 0 )
   {
      switch( packet->packetType )
      {
      case  PacketType_Cheat:
         return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------------------
