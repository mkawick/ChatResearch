#pragma once

#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

class KhaanAsset : public KhaanServerToServer
{
public:
   KhaanAsset() : KhaanServerToServer() {}
   KhaanAsset( int id, bufferevent* be ) : KhaanServerToServer( id, be ) {}
   const char*  GetClassName() const { return "KhaanAsset"; }

private:
   //void	UpdateInwardPacketList();
   //void	UpdateOutwardPacketList();

};
