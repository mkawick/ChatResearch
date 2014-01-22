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
      m_adminLevel( 0 ),
      m_gateway( NULL )
{
   m_randomNumberOfPacketsBeforeLogin = 30 + rand() % 20;
}

//-----------------------------------------------------------------------------------------

KhaanConnector::~KhaanConnector()
{
}

//-----------------------------------------------------------------------------------------

void     KhaanConnector::AuthorizeConnection() 
{ 
   m_authorizedConnection = true; 
   m_gateway->TrackStats( DiplodocusGateway::StatTracking_UserBlocked, 0, static_cast< float >( m_connectionId ) );
}

//-----------------------------------------------------------------------------------------

void     KhaanConnector::DenyAllFutureData() 
{ 
   m_denyAllFutureData = true; 
   m_gateway->TrackStats( DiplodocusGateway::StatTracking_UserBlocked, 0, static_cast< float >( m_connectionId ) );
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

bool  KhaanConnector::IsPacketSafe( unsigned char* data, int& offset)
{
   PacketFactory parser;
   // before we parse, which is potentially dangerous, we will do a quick check
   BasePacket testPacket;
   parser.SafeParse( data, offset, testPacket );

   // we only allow a few packet types
   bool allow = false;
   if( testPacket.packetType == PacketType_Login &&
      ( testPacket.packetSubType == PacketLogin::LoginType_Login || 
         testPacket.packetSubType == PacketLogin::LoginType_CreateAccount ) )
      allow = true;

   if( testPacket.packetType == PacketType_Base && testPacket.packetSubType == BasePacket::BasePacket_Hello )
      allow = true;

   if( allow == false )
   {
      m_numPacketsReceivedBeforeAuth ++;
      if( m_numPacketsReceivedBeforeAuth > m_randomNumberOfPacketsBeforeLogin )
      {
         DenyAllFutureData ();
         Log( "Gateway: hacker alert. Too many packet received without a login.", 3 );
         CloseConnection();
      }

      // we never proceed beyond here. If you are not authorized and you are not passing login packets, you get no pass, whatsoever.
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------

bool  KhaanConnector::IsHandshaking( const BasePacket* packetIn )
{
   if( packetIn->packetType == PacketType_Base && 
       packetIn->packetSubType == BasePacket::BasePacket_Hello )
   {
      //<< do nothing

      if( packetIn->versionNumber != GlobalNetworkProtocolVersion )
      {
         m_gateway->TrackStats( DiplodocusGateway::StatTracking_BadPacketVersion, packetIn->versionNumber, 0 );
      }

      // we are only sending version numbers at this point.
      PacketHello* hello = new PacketHello();
      AddOutputChainData( hello );
      return true;
   }
   return false;
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

      DenyAllFutureData ();
      Log( "Gateway: hacker alert. Packet length is far too long", 3 );
      return false;
   }

   if( m_authorizedConnection == false )
   {
      /*if( IsHandshaking( data, length ) == true )
      {
         return false;
      }*/

      if( IsPacketSafe( data, offset ) == false )
      {
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
         TrackInwardPacketType( packetIn );
      }
      catch( ... )
      {
         Log( "parsing in KhaanConnector threw an exception" );
         DenyAllFutureData ();
         break;
      }

      if( result == true )
      {
         bool packetCleanupRequired = false;
         if( m_authorizedConnection == false )
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
               m_gateway->AddInputChainData( packetIn, m_connectionId );
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
         /*if( packet->packetSubType == BasePacket::BasePacket_RerouteRequest )
         {
            return true;
         }*/
         if( packet->packetSubType == BasePacket::BasePacket_Hello )
         {
            return true;
         }
         return false;
      }
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
   case PacketType_Purchase:
      return true;
   case PacketType_Tournament:
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool  KhaanConnector::TrackInwardPacketType( const BasePacket* packet )
{
   switch( packet->packetType )
   {
      case PacketType_Gameplay:
         m_gateway->TrackStats( DiplodocusGateway::StatTracking_GamePacketsSentToGame, 1, static_cast< float >( m_connectionId ) );
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool  KhaanConnector::TrackOutwardPacketType( const BasePacket* packet )
{
   switch( packet->packetType )
   {
      case PacketType_Gameplay:
         m_gateway->TrackStats( DiplodocusGateway::StatTracking_GamePacketsSentToClient, 1, static_cast< float >( m_connectionId ) );
      return true;
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
