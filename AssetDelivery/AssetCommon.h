#pragma once
#include "../NetworkCommon/DataTypes.h"
#include <vector>
using namespace std;

struct Manifest
{
   vector< string > fileManifest;
};


class UserTicket // *consider making this a packet*
{
public:
   U32      userId;
   string   uuid;
   string   username;// just for debugging purposes
   U32      connectionId;/// only after this person has logged in
   string   userTicket;
   Manifest manifest;
   string   dateOfLastRequest;// once the client tells us what this is
   U8       gameProductId;
};