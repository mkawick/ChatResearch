// KhaanChat.h

#pragma once

#include "../NetworkCommon/NetworkIn/Khaan.h"

class PacketServerIdentifier;

/////////////////////////////////////////////////////////////////////

class KhaanChat : public Khaan
{
public:
   KhaanChat() : Khaan( 0, NULL ) {}
   KhaanChat( int id, bufferevent* be ) : Khaan( id, be ) {}

   void  PreStart();
   void  PreCleanup();

   bool	OnDataReceived( unsigned char* data, int length );

   U32   GetServerId() const { return m_serverId; }
   void  SaveOffServerIdentification( const PacketServerIdentifier* packet );

   string      m_serverName;
   U32         m_serverId;
   bool        m_isGameServer;
   bool        m_isController;
   bool        m_isGateway;
};

/////////////////////////////////////////////////////////////////////
