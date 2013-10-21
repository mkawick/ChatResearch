#pragma once


#include "../NetworkCommon/NetworkIn/Diplodocus.h"

#include "../NetworkCommon/Database/QueryHandler.h"
//#include "AssetCommon.h"
#include "UserAccountPurchase.h"
#include "KhaanPurchase.h"
//#include "UserContact.h"

#include <map>
#include <deque>
using namespace std;
class AssetOrganizer;
class PacketPrepareForUserLogin;
class PacketPrepareForUserLogout;
class PacketListOfUserProductsS2S;

///////////////////////////////////////////////////////////////////

class DiplodocusPurchase : public Diplodocus< KhaanPurchase >
{
public:
   DiplodocusPurchase( const string& serverName, U32 serverId );
   ~DiplodocusPurchase();

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

   typedef map< U64, UserAccountPurchase >      UAADMap;
   typedef pair< U64, UserAccountPurchase >     UAADPair;
   typedef UAADMap::iterator                       UAADMapIterator;

   deque< U32 >                     m_serversNeedingUpdate;
   UAADMap                          m_userTickets;
   AssetOrganizer*                  m_dynamicAssets;
   AssetOrganizer*                  m_staticAssets;
};

///////////////////////////////////////////////////////////////////