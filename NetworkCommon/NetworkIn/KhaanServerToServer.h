#pragma once
#include "khaan.h"

//////////////////////////////////////////////////////////////////////////////////

class KhaanServerToServer : public Khaan
{
public:
   KhaanServerToServer() : Khaan( 0, NULL ) {}
   KhaanServerToServer( int id, bufferevent* be ) : Khaan( id, be ) {}

   bool	   OnDataReceived( unsigned char* data, int length );

   U32      GetServerId() const { return m_serverId; }

protected:
   void  PreCleanup();

   string      m_serverName;
   U32         m_serverId;
   bool        m_isGameServer;
   bool        m_isController;
};

//////////////////////////////////////////////////////////////////////////////////