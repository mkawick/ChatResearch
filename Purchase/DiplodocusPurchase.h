#pragma once


#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Database/QueryHandler.h"

#include "SalesManager.h"
#include "UserAccountPurchase.h"
#include "KhaanPurchase.h"


#include <map>
#include <deque>
using namespace std;
class PurchasePacket;
class PurchasePacket_Buy;
class PurchasePacket_GetListOfSaleItems;
class PacketPrepareForUserLogin;
class PacketPrepareForUserLogout;
class PacketListOfUserProductsS2S;
class SalesManager;
class StringLookup;
class PacketTournament_PurchaseTournamentEntry;

///////////////////////////////////////////////////////////////////

class DiplodocusPurchase : public Queryer, public Diplodocus< KhaanPurchase > 
{
public: 
   typedef Diplodocus< KhaanPurchase > ChainedType;

public:
   enum QueryType
   {
      QueryType_ExchangeRateLookup,
      QueryType_LoadAllProducts,
      QueryType_VerifyThatUserHasEnoughMoney,
      QueryType_PerformPurchase,
      QueryType_ProductLookup,
      QueryType_SalesLookup,
      QueryType_Count
   };

public:
   DiplodocusPurchase( const string& serverName, U32 serverId );
   ~DiplodocusPurchase();

   void                    ServerWasIdentified( IChainedInterface* khaan );// callback really

   bool                    AddInputChainData( BasePacket* packet, U32 connectionId );
   bool                    AddOutputChainData( BasePacket* packet, U32 connectionId );

   bool                    AddQueryToOutput( PacketDbQuery* packet );

   const SalesManager*     GetSalesOrganizer() const { return m_salesManager; }
   const StringLookup*     GetStringLookup() const { return m_stringLookup; }

private:
   bool                    HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool                    ConnectUser( PacketPrepareForUserLogin* loginPacket );
   bool                    DisconnectUser( PacketPrepareForUserLogout* loginPacket );
   bool                    StoreUserProductsOwned( PacketListOfUserProductsS2S* productNamesPacket );
   bool                    HandlePurchaseRequest( const PacketTournament_PurchaseTournamentEntry* packet, U32 connectionId );

   int                     CallbackFunction();

   void                    AddServerNeedingUpdate( U32 serverId );

   typedef map< U64, UserAccountPurchase >      UAADMap;
   typedef pair< U64, UserAccountPurchase >     UAADPair;
   typedef UAADMap::iterator                    UAADMapIterator;

   deque< U32 >                     m_serversNeedingUpdate;
   UAADMap                          m_userTickets;
   SalesManager*                    m_salesManager;
   StringLookup*                    m_stringLookup;
};

///////////////////////////////////////////////////////////////////