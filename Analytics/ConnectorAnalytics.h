// ConnectorAnalytics.h

#pragma once
#include "../NetworkCommon/NetworkIn/KhaanServerToServer.h"

class KhaanStat : public KhaanServerToServer
{
public:
public:
   KhaanStat() : KhaanServerToServer() {}
   KhaanStat( int id, bufferevent* be ) : KhaanServerToServer( id, be ) {}

};
