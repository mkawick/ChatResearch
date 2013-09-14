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

   void                    ServerWasIdentified( ChainedInterface* khaan );// callback really
   bool                    SetIniFilePath( const string& assetPath, const string& assetDictionary, const string& dynamicAssetDictionary );

   bool                    AddInputChainData( BasePacket* packet, U32 connectionId );
   bool                    AddOutputChainData( BasePacket* packet, U32 connectionId );

   const AssetOrganizer*   GetStaticAssetOrganizer() const { return m_staticAssets; }
   const AssetOrganizer*   GetDynamicAssetOrganizer() const { return m_dynamicAssets; }

private:
   bool                    HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool                    ConnectUser( PacketPrepareForUserLogin* loginPacket );
   bool                    DisconnectUser( PacketPrepareForUserLogout* loginPacket );
   bool                    StoreUserProductsOwned( PacketListOfUserProductsS2S* productNamesPacket );

   int                     CallbackFunction();

   void                    AddServerNeedingUpdate( U32 serverId );

   typedef map< U64, UserAccountAssetDelivery >      UAADMap;
   typedef pair< U64, UserAccountAssetDelivery >     UAADPair;
   typedef UAADMap::iterator                       UAADMapIterator;

   deque< U32 >                     m_serversNeedingUpdate;
   UAADMap                          m_userTickets;
   AssetOrganizer*                  m_dynamicAssets;
   AssetOrganizer*                  m_staticAssets;
};

///////////////////////////////////////////////////////////////////