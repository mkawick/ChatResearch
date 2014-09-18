
#include <memory.h>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

#include <assert.h>

#include "FruitadensLogin.h"

//-----------------------------------------------------------------------------------------

FruitadensLogin::FruitadensLogin( const char* name ) : FruitadensServer( name )
{
   SetSleepTime( 30 );// Sleeping frees up CPU
}

//-----------------------------------------------------------------------------------------

/*void     FruitadensLogin::InitalConnectionCallback()
{
   //NotifyEndpointOfIdentification( serverName, serverAddress, serverId, serverPort, 0, false, false, true, false  );
}*/

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------

bool FruitadensLogin::FilterOutwardPacket( BasePacket* packet ) const
{
   // we should only pass login successful packets.. not even failure packets

   assert( m_serverId != 0 );

   //PacketType type = static_cast< PacketType > ( wrapper->pPacket->packetType );
   U32 packetType = static_cast< U32 >( packet->packetType );
   U32 packetSubType = static_cast< U32 >( packet->packetSubType );

   if( packetType == PacketType_ServerToServerWrapper )
   {
      return true;
   }

   if( packetType == PacketType_Login ) 
   {
      if( packetSubType == PacketLogin::LoginType_PrepareForUserLogin ||
         packetSubType == PacketLogin::LoginType_PrepareForUserLogout)
      {
         return true;
      }
      
      if( packetSubType == PacketLogin::LoginType_ListOfProductsS2S )
      {
         if( m_serverType == ServerType_Asset )
            return true;
         return false;
      }

      if( packetSubType == PacketLogin::LoginType_UserUpdateProfile )
      {
         if( m_serverType == ServerType_Contact )
            return true;
         return false;
      }
   }

   if( packetType == PacketType_Gameplay ) 
   {
      if( packetSubType == PacketGameToServer::GamePacketType_ListOfGames )
      {
         return true;
      }
   }
   if( m_serverType == ServerType_Analytics )
   {
      if( packetType == PacketType_Analytics )
      {
         return true;
      }
   }
   if( m_serverType == ServerType_Notification )
   {
      if( packetType == PacketType_Notification )
      {
         return true;
      }
   }

   return false;
}
/*
int       FruitadensLogin::CallbackFunction()
{
   ProcessEvents();

   m_inputChainListMutex.lock();
   LogMessage( LOG_PRIO_INFO, "FruitadensLogin: MainLoop_InputProcessing enter" );
   MainLoop_InputProcessing();
   LogMessage( LOG_PRIO_INFO, "FruitadensLogin: MainLoop_InputProcessing exit" );
   m_inputChainListMutex.unlock();

   //-------------------------------

   m_outputChainListMutex.lock();
   LogMessage( LOG_PRIO_INFO, "FruitadensLogin: MainLoop_OutputProcessing enter" );
   MainLoop_OutputProcessing();
   LogMessage( LOG_PRIO_INFO, "FruitadensLogin: MainLoop_OutputProcessing exit" );
   m_outputChainListMutex.unlock();

   return 0;
}*/

//-----------------------------------------------------------------------------------------

int  FruitadensLogin::MainLoop_OutputProcessing()
{
   if( m_extensiveLogging )
   {
      LogMessage( LOG_PRIO_INFO, "FruitadensLogin: OutputLoop enter" );
   }
   if( m_isConnected == false )
   {
      return 0;
   }

   //m_mutex.lock();
   PacketQueue tempQueue = m_packetsReadyToSend;
   m_packetsReadyToSend.clear();
   //m_mutex.unlock();

   if( tempQueue.size() > 0 )
   {
      if( m_extensiveLogging )
      {
         LogMessage( LOG_PRIO_INFO, "FruitadensLogin:sending packets: %u ", tempQueue.size() );
      }
      PacketFactory factory;      
      while( tempQueue.size() )
      {
         BasePacket* packet = tempQueue.front();
         tempQueue.pop_front();
         
         if( packet->packetType == PacketType_ServerToServerWrapper )         
         {
            SerializePacketOut( packet );
         }
         else
         {
            PacketServerToServerWrapper wrapper;
            wrapper.serverId = m_serverId;
            wrapper.pPacket = packet;
            SerializePacketOut( &wrapper );
         }

         factory.CleanupPacket( packet );
      }
   }

   if( m_extensiveLogging )
   {
      LogMessage( LOG_PRIO_INFO, "FruitadensLogin: OutputLoop exit" );
   }
   return 0;
}

//-----------------------------------------------------------------------------------------
