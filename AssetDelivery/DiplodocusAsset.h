#pragma once

#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "../NetworkCommon/Database/QueryHandler.h"
#include "AssetCommon.h"
#include "UserAccountAssetDelivery.h"
#include "KhaanAsset.h"
//#include "UserContact.h"

#include <map>
#include <deque>
using namespace std;

///////////////////////////////////////////////////////////////////

class DiplodocusAsset : public Diplodocus< KhaanAsset >
{
public:
   DiplodocusAsset( const string& serverName, U32 serverId );

   void     ServerWasIdentified( KhaanAsset* khaan );

   bool     AddInputChainData( BasePacket* packet, U32 connectionId );

private:
   bool     HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool     ConnectUser( PacketPrepareForUserLogin* loginPacket );
   bool     DisconnectUser( PacketPrepareForUserLogout* loginPacket );

   typedef map< U64, UserAccountAssetDelivery >      UAADMap;
   typedef pair< U64, UserAccountAssetDelivery >     UAADPair;
   typedef UAADMap::iterator                       UAADMapIterator;

   deque< U32 >                     m_serversNeedingUpdate;
   UAADMap                          m_userTickets;
};

///////////////////////////////////////////////////////////////////