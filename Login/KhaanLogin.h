#pragma once
#include "../NetworkCommon/NetworkIn/khaan.h"

class KhaanLogin : public Khaan
{
public:
   KhaanLogin() : Khaan( 0, NULL ) {}
   KhaanLogin( int id, bufferevent* be ) : Khaan( id, be ) {}

   bool	OnDataReceived( unsigned char* data, int length );

   U32   GetServerId() const { return m_serverId; }

protected:
   void  PreCleanup();

   string      m_serverName;
   U32         m_serverId;
   bool        m_isGameServer;
   bool        m_isController;
};
