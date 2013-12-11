#pragma once

#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

class KhaanPurchase : public KhaanServerToServer
{
public:
   KhaanPurchase() : KhaanServerToServer() {}
   KhaanPurchase( int id, bufferevent* be ) : KhaanServerToServer( id, be ) {}

private:
   //void	UpdateInwardPacketList();
   //void	UpdateOutwardPacketList();
};
