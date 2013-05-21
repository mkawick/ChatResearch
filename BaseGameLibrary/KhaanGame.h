#pragma once
// KhaanGame.h

#include "../NetworkCommon/NetworkIn/Khaan.h"

class PacketServerIdentifier;

///////////////////////////////////////////////////////////////

class KhaanGame : public Khaan
{
public:
   KhaanGame() : Khaan( 0, NULL ), m_isGameServer( false ), m_isController( 0 )  {}
   KhaanGame( int id, bufferevent* be ) : Khaan( id, be ), m_isGameServer( false ), m_isController( 0 ) {}

   bool	OnDataReceived( unsigned char* data, int length );

   U32   GetServerId() const { return m_serverId; }

private:
   void  SaveOffServerIdentification( const PacketServerIdentifier* packet );

   string      m_serverName;
   U32         m_serverId;
   bool        m_isGameServer;
   bool        m_isController;
};

///////////////////////////////////////////////////////////////
