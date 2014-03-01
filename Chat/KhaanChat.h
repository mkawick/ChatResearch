#pragma once
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

class PacketServerIdentifier;

/////////////////////////////////////////////////////////////////////

class KhaanChat : public KhaanServerToServer
{
public:
   KhaanChat() : KhaanServerToServer( 0, NULL ) {}
   KhaanChat( int id, bufferevent* be ) : KhaanServerToServer( id, be ) {}

   void  PreStart();
   void  PreCleanup();
};

/////////////////////////////////////////////////////////////////////
