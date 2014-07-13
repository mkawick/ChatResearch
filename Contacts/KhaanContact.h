#pragma once
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

class KhaanContact : public KhaanServerToServer
{
public:
   KhaanContact() : KhaanServerToServer() {}
   KhaanContact( int id, bufferevent* be ) : KhaanServerToServer( id, be ) {}
   const char* GetClassName() const { return "KhaanContact"; }

protected:
};