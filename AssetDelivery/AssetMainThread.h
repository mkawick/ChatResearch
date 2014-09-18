#pragma once

#include <map>
#include <deque>
using namespace std;

#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include "AssetCommon.h"
#include "UserAccountAssetDelivery.h"
#include "KhaanAsset.h"
//#include "UserContact.h"

class    AssetOrganizer;
struct   AssetDefinition;
class    PacketPrepareForUserLogin;
class    PacketPrepareForUserLogout;
class    PacketListOfUserProductsS2S;

///////////////////////////////////////////////////////////////////

class AssetMainThread : public Diplodocus< KhaanAsset >
{
public: 
   typedef Diplodocus< KhaanAsset > ChainedType;

public:
   AssetMainThread( const string& serverName, U32 serverId );
   ~AssetMainThread();

   void                    ServerWasIdentified( IChainedInterface* khaan );// callback really
   void                    InputRemovalInProgress( IChainedInterface * );;
   bool                    SetIniFilePath( const string& assetPath, const string& assetOfAssets );

   bool                    AddInputChainData( BasePacket* packet, U32 connectionId );
   bool                    AddOutputChainData( BasePacket* packet, U32 connectionId );

   const AssetOrganizer*   GetAssetOrganizer( const string& AssetCategory ) const;
   bool                    GetListOfAssetCategories( vector<string>& categories ) const;
   const AssetDefinition*  GetAsset( const string& hash ) const;
   //const AssetOrganizer*   GetDynamicAssetOrganizer() const { return m_dynamicAssets; }

private:

   bool                    LoadAllAssetManifests();
   bool                    HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool                    ConnectUser( const PacketPrepareForUserLogin* loginPacket );
   bool                    DisconnectUser( const PacketPrepareForUserLogout* loginPacket );
   bool                    StoreUserProductsOwned( PacketListOfUserProductsS2S* productNamesPacket );

   int                     CallbackFunction();
   void                    ExpireOldConnections();

   void                    AddServerNeedingUpdate( U32 serverId );

   typedef map< U64, UserAccountAssetDelivery >      UAADMap;
   typedef pair< U64, UserAccountAssetDelivery >     UAADPair;
   typedef UAADMap::iterator                       UAADMapIterator;

public:
   typedef map< string, AssetOrganizer >           CategorizedAssetLists;// asset category paired with a list of assets.
   typedef pair< string, AssetOrganizer >          CategorizedAssetPair;

   UserAccountAssetDelivery         m_dummyUser;
   //deque< U32 >                     m_serversNeedingUpdate;
   UAADMap                          m_userTickets;
   CategorizedAssetLists            m_assetsByCategory;

   string                           m_mainAssetFilePath;
   time_t                           m_checkForFileChangeTimestamp;
   time_t                           m_assetOfAssetFileModificationTime;
   static const int                 CheckForFileModificationTimeout = 2 * 60; // two minutes
};

///////////////////////////////////////////////////////////////////