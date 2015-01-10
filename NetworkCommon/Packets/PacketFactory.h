// PacketFactory.h

#pragma once

#include "../DataTypes.h"

class BasePacket;
///////////////////////////////////////////////////////////////

// The following class should be derived from and provide the uniqueness for each server product. The definition of which packets
// can be parsed, what they should have in them, etc. This should be a behavior-only class with minimal data needed for packing
// the various packets. This is a policy-class as recommended by Alexandrescu.

class PacketFactory
{
public:
	PacketFactory();
   bool     Parse( const U8* bufferIn, int& bufferOffset, BasePacket** packetOut, int networkMinorVersion ) const;// be sure to check the return value
   void     CleanupPacket( BasePacket*& packetOut );

   bool     SafeParse( const U8* bufferIn, int& bufferOffset, BasePacket& packetOut, int networkMinorVersion ) const;// only ever returns an instance of basepacket

   bool     Create( int packetType, int packetSubType, BasePacket** packetOut ) const;
private:
   bool     CreateBasePacket( int packetSubType, BasePacket** packetOut ) const;
   
   //bool     CreateBasePacket( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateAsset( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateChat( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateCheat( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateContact( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateDbQuery( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateGame( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateInvitation( int packetSubType, BasePacket** packetOut ) const;
   
   bool     CreateLogin( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateNotification( int packetSubType, BasePacket** packetOut ) const;
   bool     CreatePurchase( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateAnalytics( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateTournament( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateUserInfo( int packetSubType, BasePacket** packetOut ) const;

   bool     CreateUserStats( int packetSubType, BasePacket** packetOut ) const;
   
   bool     CreateServerToServerWrapper( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateServerInfo( int packetSubType, BasePacket** packetOut ) const;
   bool     CreateGatewayWrapper( int packetSubType, BasePacket** packetOut ) const;
};


class PacketCleaner
{
public:
   PacketCleaner( BasePacket* packet ): m_packet( packet ){}
   PacketCleaner( const BasePacket* packet ): m_packet( const_cast<BasePacket*>( packet ) ){}
   ~PacketCleaner()
   {
      if( m_packet )
      {
         PacketFactory factory;
         factory.CleanupPacket( m_packet );
      }
   }
   void Clear() { m_packet = NULL; }
private:
   PacketCleaner();

   BasePacket* m_packet;
};

///////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////