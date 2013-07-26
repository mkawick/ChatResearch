// FruitadensGateway.cpp

#include "../NetworkCommon/ServerConstants.h"
#include "FruitadensGateway.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/BasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"

#include <iostream>
#include <iomanip>

//-----------------------------------------------------------------------------------------

FruitadensGateway::FruitadensGateway( const char* name ) : Fruitadens( name )
{
   SetSleepTime( 30 );// Sleeping frees up CPU
}

//-----------------------------------------------------------------------------------------

FruitadensGateway::~FruitadensGateway()
{
}

//-----------------------------------------------------------------------------------------

bool FruitadensGateway::FilterOutwardPacket( BasePacket* packet ) const
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );

      PacketType type = static_cast< PacketType > ( wrapper->pPacket->packetType );
      if( m_serverType == ServerType_GameInstance )
      {
         if( type == PacketType_Gameplay )
         {
            // sending packets to the correct server.
            if( m_connectedServerId == wrapper->pPacket->gameInstanceId && 
                  wrapper->pPacket->gameProductId == m_connectedGameProductId )
            {
               return true;
            }
         }
      }
      else if( m_serverType == ServerType_Chat )
      {
         if( type == PacketType_Chat )
            return true;
         if( type == PacketType_UserInfo )// todo, remove me once we have other things in place
            return true;
         if( type == PacketType_GatewayInformation )
            return true;
      }
      else if( m_serverType == ServerType_Login )
      {
         if( type == PacketType_Login )// login is always acceptable... for now. Once we have a login server, we need to remove this exception.
         {
            return true;
         }
      }
      else if( m_serverType == ServerType_Contact )
      {
         if( type == PacketType_UserInfo )
         {
            return true;
         }
         if( type == PacketType_Contact )
         {
            return true;
         }
      }
      else if( m_serverType == ServerType_Asset )
      {
         if( type == PacketType_Asset )
         {
            return true;
         }
      }
   }
   else if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      return true;
   }

   return false;
}

//-----------------------------------------------------------------------------------------

int  FruitadensGateway::ProcessOutputFunction()
{
   if( m_isConnected == false )
   {
      return 0;
   }

   if( m_packetsReadyToSend.size() > 0 )
   {
      U8 buffer[ MaxBufferSize ];
      int dangerZone = MaxBufferSize * 3/4;// 25%
      int offset = 0;

      
      while( m_packetsReadyToSend.size() && offset < dangerZone )
      {
         m_mutex.lock();
         BasePacket* packet = m_packetsReadyToSend.front();
         m_packetsReadyToSend.pop_front();
         m_mutex.unlock();
         packet->SerializeOut( buffer, offset );

         //SerializePacketOut( packet );

         if( packet->packetType == PacketType_GatewayWrapper )
         {
            PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
            delete wrapper->pPacket;
         }
         if( packet->packetType == PacketType_ServerToServerWrapper )
         {
            PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );
            delete wrapper->pPacket;
         }
         delete packet;
      }
      

      SendPacket( buffer, offset );
   }

   return 0;
}

//-----------------------------------------------------------------------------------------
