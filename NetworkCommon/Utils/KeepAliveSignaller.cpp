// KeepAliveSignaller.cpp

#include "../Packets/BasePacket.h"
#include "../Packets/ServerToServerPacket.h"
#include "./Utils.h"
#include "../ChainedArchitecture/ChainedInterface.h"
#include "../ServerConstants.h"
#include "../NetworkUtils.h"
#include "../Logging/server_log.h"

#include "KeepAliveSignaller.h"

#include <time.h>

/////////////////////////////////////////////////////////////////////////////////

KeepAliveSignaller::KeepAliveSignaller() : 
         m_isValid( true ),
         m_isEnabled( false ),
         m_requiresKeepAliveSignal( false ),
         m_isServer( true ),
         m_enableLogging( false ),
         m_timeLastSignalSent( 0 ),
         m_timeLastSignalReceived( 0 ),
         m_timeoutPeriod( 10 ),
         m_packetCounter( 0 ),
         m_isAwaitingKeepAliveReturn( false ),
         m_packetHandler( NULL )
   { }

void  KeepAliveSignaller::Enable( bool enable ) 
{ 
   m_isEnabled = enable; 
   if( enable == true )
   {
      LogMessage( LOG_PRIO_INFO, "KeepAliveSignaller::enabled - send" );
   }
}

void  KeepAliveSignaller::FunctionsAsServer( bool isServer )
{
   m_isServer = isServer;
}

void  KeepAliveSignaller::ResetAfterDisconnect()
{
   m_requiresKeepAliveSignal = false;
   m_isAwaitingKeepAliveReturn = false;
   m_timeLastSignalSent = 0;
   m_timeLastSignalReceived = 0;
}

bool  KeepAliveSignaller::HasKeepAliveExpired() const
{
   if( m_packetHandler == NULL )
   {
      if( m_enableLogging )
      {
         LogMessage( LOG_PRIO_INFO, "Error in config for KeepAliveSignaller::HasKeepAliveExpired" );
      }
      return false;
   }

   if( CanDoAnyWork() == false )
      return false;

   if( m_isServer )
   {
      if( m_isAwaitingKeepAliveReturn == true )
      {
         time_t currentTime;
         time( &currentTime );
         if( HasTimeWindowExpired( currentTime, m_timeLastSignalSent, m_timeoutPeriod ) )
         {
            if( m_enableLogging )
            {
               LogMessage( LOG_PRIO_INFO, ">>> Has Keep Alive Expired: %s", GetDateInUTC( currentTime ).c_str() );         
            }
            return true;
         }
      }
   }
   else
   {
      if( m_requiresKeepAliveSignal == true && m_timeLastSignalReceived != 0 )
      {
         time_t currentTime;
         time( &currentTime );
         if( HasTimeWindowExpired( currentTime, m_timeLastSignalReceived, m_timeoutPeriod ) )
         {
            if( m_enableLogging )
            {
               LogMessage( LOG_PRIO_INFO, ">>> Has Keep Alive Expired: %s", GetDateInUTC( currentTime ).c_str() );
            }
            return true;
         }
      }
      return false;
   }

   return false;
}

/////////////////////////////////////////////////////////////////////////////////

void  KeepAliveSignaller::Update()
{
   if( CanDoAnyWork() == false )
      return;

   if( m_isServer == true )
   {
      if( m_isValid == false )
         return;

      time_t currentTime;
      time( &currentTime );

      if( m_timeLastSignalSent == 0 )// first connect, we do not send keep alive immediately.
      {
         m_timeLastSignalSent = currentTime;
         return;
      }

      if( HasTimeWindowExpired( currentTime, m_timeLastSignalSent, m_timeoutPeriod ) )
      {
         if( m_enableLogging )
         {
            LogMessage( LOG_PRIO_INFO, ">>> Send keep alive:        %s,    packet no: %d", GetDateInUTC( currentTime ).c_str(), m_packetCounter );
         }
         PacketServerConnectionInfo_KeepAlive* packet = new PacketServerConnectionInfo_KeepAlive;
         packet->gameInstanceId = 0;
         packet->gameProductId = 0;
         packet->packetNo = m_packetCounter++;
         m_packetHandler->AddOutputChainDataNoLock( packet );
         m_isAwaitingKeepAliveReturn = true;
         m_timeLastSignalSent = currentTime;
      }
   }
   else
   {
      if( m_requiresKeepAliveSignal ) {}
   }
}

/////////////////////////////////////////////////////////////////////////////////

bool  KeepAliveSignaller::HandlePacket( const BasePacket* packetIn )
{
   if( CanDoAnyWork() == false )
      return false;

   if( packetIn->packetType == PacketType_ServerToServerWrapper )
   {
       const PacketServerToServerWrapper* wrapper = static_cast< const PacketServerToServerWrapper* >( packetIn );
       packetIn = wrapper->pPacket;
   }
   if( packetIn->packetType == PacketType_ServerInformation &&
      packetIn->packetSubType == PacketServerConnectionInfo::PacketServerIdentifier_KeepAlive )
   {
      if( m_isServer == true )
      {
         time( &m_timeLastSignalSent );
         m_timeLastSignalReceived = m_timeLastSignalSent;
         //m_timeLastSignalSent -= (m_secondsBeforeTimeout >> 1);// so that it sends soon
         m_isAwaitingKeepAliveReturn = false;
         return true;
      }
      else
      {
         m_requiresKeepAliveSignal = true;// this is not required unless the server says so. 
         //  For a sender (Khaan).. this flag doesn't mean anything.
         // For a Receiver, this triggers the requirement that we try to reconnect
         if( m_enableLogging )
         {
            LogMessage( LOG_PRIO_INFO, "KeepAliveSignaller::enabled - receive" );
         }
         
         PacketServerConnectionInfo_KeepAlive* returnPacket = new PacketServerConnectionInfo_KeepAlive;
         returnPacket->gameInstanceId = 0;
         returnPacket->gameProductId = 0;
         m_packetCounter = static_cast< const PacketServerConnectionInfo_KeepAlive* >( packetIn )->packetNo;
         returnPacket->packetNo = m_packetCounter;

         m_packetHandler->AddOutputChainDataNoLock( returnPacket );
         time( &m_timeLastSignalReceived );
         if( m_enableLogging )
         {
            LogMessage( LOG_PRIO_INFO, ">>> Send keep alive:        %s,    packet no: %d", GetDateInUTC( m_timeLastSignalReceived ).c_str(), m_packetCounter );
         }
         return true;
      }
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////

bool  KeepAliveSignaller::CanDoAnyWork() const
{
   if( m_packetHandler == NULL || m_isEnabled == false || m_timeoutPeriod == 0 )
      return false;

   return true;
}

/////////////////////////////////////////////////////////////////////////////////
