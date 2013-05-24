#pragma once
#include "khaan.h"

class PacketServerIdentifier;

//////////////////////////////////////////////////////////////////////////////////

class KhaanServerToServer : public Khaan
{
public:
   KhaanServerToServer() : Khaan( 0, NULL ), m_serverId( 0 ), m_isGameServer( false ), m_isController( false ), m_isGateway( false ) {}
   KhaanServerToServer( int id, bufferevent* be ) : Khaan( id, be ), m_serverId( 0 ), m_isGameServer( false ), m_isController( false ), m_isGateway( false )  {}

   bool	   OnDataReceived( unsigned char* data, int length );

   U32      GetServerId() const { return m_serverId; }

protected:
   void  PreCleanup();

   virtual bool   PassPacketOn( BasePacket* serverId, U32 connectionId );// this is good to override
   void           SaveOffServerIdentification( const PacketServerIdentifier* serverId );

   string      m_serverName;
   U32         m_serverId;
   bool        m_isGameServer;
   bool        m_isController;
   bool        m_isGateway;
};

//////////////////////////////////////////////////////////////////////////////////