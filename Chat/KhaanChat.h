#pragma once
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

class PacketServerIdentifier;

/////////////////////////////////////////////////////////////////////

class KhaanChat : public KhaanServerToServer
{
public:
   KhaanChat() : KhaanServerToServer( 0, NULL ) {}
   KhaanChat( int id, bufferevent* be ) : KhaanServerToServer( id, be ) {}
   const char* GetClassName() const { return "KhaanChat"; }

   void  PreStart();
   void  PreCleanup();
};

/////////////////////////////////////////////////////////////////////
