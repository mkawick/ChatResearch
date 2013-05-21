#pragma once
#include "khaan.h"

//////////////////////////////////////////////////////////////////////////////////

class KhaanServerToServer : public Khaan
{
public:
   KhaanServerToServer() : Khaan( 0, NULL ), m_serverId( 0 ), m_isGameServer( false ), m_isController( false ) {}
   KhaanServerToServer( int id, bufferevent* be ) : Khaan( id, be ), m_serverId( 0 ), m_isGameServer( false ), m_isController( false ) {}

   bool	   OnDataReceived( unsigned char* data, int length );

   U32      GetServerId() const { return m_serverId; }

protected:
   void  PreCleanup();

   void        SaveOffServerIdentification( const BasePacket* serverId );

   string      m_serverName;
   U32         m_serverId;
   bool        m_isGameServer;
   bool        m_isController;
};

//////////////////////////////////////////////////////////////////////////////////