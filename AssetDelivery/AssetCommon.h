#pragma once
#include "../NetworkCommon/DataTypes.h"
#include <string>
#include <vector>
using namespace std;


#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>


#include "../NetworkCommon/Version.h"
#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Daemon/Daemonizer.h"
#include "../NetworkCommon/Utils/CommandLineParser.h"

struct Manifest
{
   vector< string > fileManifest;
};


class UserTicket // *consider making this a packet*
{
public:
   U32      userId;
   string   uuid;
   string   userName;// just for debugging purposes
   U32      connectionId;/// only after this person has logged in
   U32      gatewayId;
   string   userTicket;
   Manifest manifest;
   string   dateOfLastRequest;// once the client tells us what this is
   U8       gameProductId;
};