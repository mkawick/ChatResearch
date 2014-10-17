// KhaanGateway.cpp

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "KhaanGateway.h"
#include "MainGatewayThread.h"

//-----------------------------------------------------------------------------------------

KhaanGateway::KhaanGateway( int id, bufferevent* be ): 
      KhaanProtected( id, be ), 
      m_numPacketsReceivedBeforeAuth( 0 ),
      m_authorizedConnection( false ),
      m_logoutPacketSent( false ),
      m_adminLevel( 0 ),
      m_languageId( 0 ),
      m_timeoutMs( 0 ),
      m_lastSentToClientTimestamp( 0 ),
      m_gameId( 0 )
{
   m_randomNumberOfPacketsBeforeLogin = 30 + rand() % 20;
   SendThroughLibEvent( true );

   SetOutboudBufferSize( 7600 ); /// slower outbound buffer sending
}

//-----------------------------------------------------------------------------------------

KhaanGateway::~KhaanGateway()
{
}

//-----------------------------------------------------------------------------------------

void     KhaanGateway::AuthorizeConnection() 
{ 
   m_authorizedConnection = true; 
}

void     KhaanGateway::SetLanguageId( U8 languageId ) 
{ 
   m_languageId = languageId;
}

//-----------------------------------------------------------------------------------------

void     KhaanGateway::DenyAllFutureData() 
{ 
   m_denyAllFutureData = true;
   if( m_mainOutputChain )
   {
      static_cast< MainGatewayThread* >( m_mainOutputChain )->TrackCountStats( StatTrackingConnections::StatTracking_UserBlocked, 1, 0 );
   }
}

//-----------------------------------------------------------------------------------------

void  KhaanGateway::PreCleanup()
{
   if( m_mainOutputChain )
   {
      m_isDisconnected = true;
      DenyAllFutureData();
      time_t currentTime;
      time( &currentTime );
      SetTimeForDeletion( currentTime );
   }
}

//-----------------------------------------------------------------------------

bool	KhaanGateway :: Update()
{
   if( m_isDisconnected ) // no update for you
   {

      if( m_mainOutputChain )
      {
         static_cast< MainGatewayThread* >( m_mainOutputChain )->TrackCountStats( StatTrackingConnections::StatTracking_UsersLostConnection, 1, 0 );
         // order really matters here... this function must come last
         static_cast< MainGatewayThread* >( m_mainOutputChain )->InputRemovalInProgress( this );
      }

      m_mainOutputChain = NULL;

      return false;
   }

   if( m_hasPacketsReceived == true )
   {
      UpdateInwardPacketList();
   }

   if(  m_hasPacketsToSend == true && ShouldDelayOutput() == false )
   {
      UpdateOutwardPacketList();
   }

   // I think that this makes sense
   CleanupAllEvents();

   if( m_hasPacketsToSend || m_hasPacketsReceived )// we didn't finish
   {
      string loggingText = "Remaining packet out count: ";
      loggingText += boost::lexical_cast< string >( m_packetsOut.size() );
      LogMessage( LOG_PRIO_INFO, loggingText.c_str() );
      return false;
   }

   if( m_denyAllFutureData && m_packetsOut.size() == 0 ) // shut it down
   {
      CloseConnection();
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------

void     KhaanGateway :: SetupOutputDelayTimestamp()
{
   if( m_timeoutMs == 0 )
      return;

   U32 currentTime = GetCurrentMilliseconds();

   U32 diffTime = currentTime - m_lastSentToClientTimestamp;
   if( diffTime < m_timeoutMs && m_packetsOut.size() )// do not keep increasing the delay if you are already waiting to send back data
   {
      return;
   }

   m_lastSentToClientTimestamp = currentTime;
}

//-----------------------------------------------------------------------------------------

bool  KhaanGateway :: ShouldDelayOutput()
{
   if( m_timeoutMs == 0 )
   {
      return false;
   }
 
   U32 currentTime = GetCurrentMilliseconds();

   U32 diffTime = currentTime - m_lastSentToClientTimestamp;
   if( diffTime >= m_timeoutMs )
   {
      
      m_lastSentToClientTimestamp = currentTime;
      return false;
   }

   return true;

}

//-----------------------------------------------------------------------------------------

bool  KhaanGateway::IsPacketSafe( const U8* data, int& offset)
{
   PacketFactory parser;
   // before we parse, which is potentially dangerous, we will do a quick check
   BasePacket testPacket;
   parser.SafeParse( data, offset, testPacket, 0 );// always pass the simplest possible - 0

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
         LogMessage( LOG_PRIO_INFO, "Gateway: hacker alert. Too many packet received without a login." );
         CloseConnection();
      }

      // we never proceed beyond here. If you are not authorized and you are not passing login packets, you get no pass, whatsoever.
      return false;
   }

   return true;
}

//-----------------------------------------------------------------------------------------

bool  KhaanGateway::IsHandshaking( const BasePacket* packetIn )
{
   if( packetIn->packetType == PacketType_Base && 
       packetIn->packetSubType == BasePacket::BasePacket_Hello )
   {
      //<< do nothing

      if( packetIn->versionNumberMajor != NetworkVersionMajor )
      {
         if( m_mainOutputChain )
         {
            static_cast< MainGatewayThread* >( m_mainOutputChain )->TrackCountStats( StatTrackingConnections::StatTracking_BadPacketVersion, 1, packetIn->versionNumberMajor );
         }
         DenyAllFutureData();
      }

      m_versionNumberMinor = packetIn->versionNumberMinor;

      // we are only sending version numbers at this point.
      PacketHello* hello = new PacketHello();
      //hello->test = "this is a long string meant to prove out the viability of accepting packets of pracically any size and to not worry too much about packet versioning";
      AddOutputChainData( hello );

      PacketBase_TestOnly* test = new PacketBase_TestOnly();
      test->testNo = 30;
      test->testString = "what the heck is this long string doing in our network protocol";
      AddOutputChainData( test );

      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool  KhaanGateway::IsWhiteListedIn( const BasePacket* packet ) const
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
      {
         if( packet->packetSubType == PacketLogin::LoginType_ListOfAggregatePurchases )
         {
            return true;// breakpoint
         }
         return true;
      }
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
   case PacketType_Notification:
      return true;
   case PacketType_Invitation:
      return true;
   case PacketType_UserStats:
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

bool  KhaanGateway::TrackInwardPacketType( const BasePacket* packet )
{
   switch( packet->packetType )
   {
      case PacketType_Gameplay:
         m_gameId = packet->gameProductId;
         //m_gateway->TrackCountStats( StatTrackingConnections::StatTracking_GamePacketsSentToGame, 1, packet->gameProductId );
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool  KhaanGateway::TrackOutwardPacketType( const BasePacket* packet )
{
   switch( packet->packetType )
   {
      case PacketType_Login:
      {
         if( packet->packetSubType == PacketLogin::LoginType_Logout )
         {
            m_logoutPacketSent = true;
         }
      }
      break;
      case PacketType_Gameplay:
         //m_gateway->TrackCountStats( StatTrackingConnections::StatTracking_GamePacketsSentToClient, 1, packet->gameProductId );
         //m_gateway->TrackStats( const string& serverName, int ServerId, const string& statName, U16 stat, float value, PacketStat::StatType type )
      return true;
   }
   return false;
}

//-----------------------------------------------------------------------------------------

bool  KhaanGateway::HasPermission( const BasePacket* packet ) const
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

