#pragma once
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"


class KhaanLogin : public KhaanServerToServer
{
public:
   KhaanLogin() : KhaanServerToServer() {}
   KhaanLogin( int id, bufferevent* be ) : KhaanServerToServer( id, be ) {}
   const char*  GetClassName() const { return "KhaanLogin"; }

protected:
};
