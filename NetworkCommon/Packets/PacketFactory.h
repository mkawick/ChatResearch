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

private:

   bool     ParseBasePacket( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseAsset( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseChat( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseCheat( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseContact( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseDbQuery( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseGame( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseInvitation( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   
   bool     ParseLogin( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseNotification( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParsePurchase( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseAnalytics( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseTournament( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseUserInfo( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;

   bool     ParseUserStats( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   
   bool     ParseServerToServerWrapper( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseServerInfo( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
   bool     ParseGatewayWrapper( const U8* bufferIn, int& bufferOffset, const BasePacket* firstPassParse, BasePacket** packetOut, int networkMinorVersion ) const;
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