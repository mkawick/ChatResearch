#pragma once
// KhaanGame.h

#include "../NetworkCommon/NetworkIn/Khaan.h"

///////////////////////////////////////////////////////////////

class KhaanGame : public Khaan
{
public:
   KhaanGame() : Khaan( 0, NULL ), m_isGameServer( false ), m_isController( 0 )  {}
   KhaanGame( int id, bufferevent* be ) : Khaan( id, be ), m_isGameServer( false ), m_isController( 0 ) {}

   bool	OnDataReceived( const U8* data, int length );

private:
   string      m_serverName;
   U32         m_serverId;
   bool        m_isGameServer;
   bool        m_isController;
};

///////////////////////////////////////////////////////////////
