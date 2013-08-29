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
class AssetOrganizer;

///////////////////////////////////////////////////////////////////

class DiplodocusAsset : public Diplodocus< KhaanAsset >
{
public:
   DiplodocusAsset( const string& serverName, U32 serverId );
   ~DiplodocusAsset();

   void                    ServerWasIdentified( KhaanAsset* khaan );// callback really
   bool                    SetIniFilePath( const string& assetPath, const string& assetDictionary );

   bool                    AddInputChainData( BasePacket* packet, U32 connectionId );
   bool                    AddOutputChainData( BasePacket* packet, U32 connectionId );

   const AssetOrganizer*   GetAssetOrganizer() const { return m_assets; }

private:
   bool                    HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool                    ConnectUser( PacketPrepareForUserLogin* loginPacket );
   bool                    DisconnectUser( PacketPrepareForUserLogout* loginPacket );

   int                     CallbackFunction();

   typedef map< U64, UserAccountAssetDelivery >      UAADMap;
   typedef pair< U64, UserAccountAssetDelivery >     UAADPair;
   typedef UAADMap::iterator                       UAADMapIterator;

   deque< U32 >                     m_serversNeedingUpdate;
   UAADMap                          m_userTickets;
   AssetOrganizer*                  m_assets;
};

///////////////////////////////////////////////////////////////////