// PurchaseMainThread.h

#pragma once


#include "../NetworkCommon/NetworkIn/Diplodocus.h"
#include "../NetworkCommon/Database/QueryHandler.h"

#include "SalesManager.h"
#include "PurchaseReceiptManager.h"
#include "UserAccountPurchase.h"
#include "KhaanPurchase.h"


#include <map>
using namespace std;
class PurchasePacket;
class PurchasePacket_Buy;
class PurchasePacket_GetListOfSaleItems;
class PacketPrepareForUserLogin;
class PacketPrepareForUserLogout;
class PacketListOfUserProductsS2S;
class PacketLoginExpireUser;
class SalesManager;
class StringLookup;
class PurchaseReceiptManager;
class PacketTournament_PurchaseTournamentEntry;

///////////////////////////////////////////////////////////////////

class DiplodocusPurchase : public Queryer, public Diplodocus< KhaanPurchase > 
{
public:
   enum QueryType
   {
      QueryType_ExchangeRateLookup,
      QueryType_LoadAllProducts,
      QueryType_VerifyThatUserHasEnoughMoney1,
      QueryType_VerifyThatUserHasEnoughMoney2,
      QueryType_PerformPurchase,
      QueryType_PerformInventoryAddition,
      QueryType_PerformPurchase2,
      QueryType_ProductLookup,
      QueryType_SalesLookup,
      QueryType_ReceiptInsert,
      QueryType_ReceiptLookup,
      QueryType_PurchaseAdminSettings,
      QueryType_Count
   };

public:
   DiplodocusPurchase( const string& serverName, U32 serverId );
   ~DiplodocusPurchase();
   const char*             GetClassName() const { return "DiplodocusPurchase"; }

   void                    ServerWasIdentified( IChainedInterface* khaan );// callback really
   void                    InputRemovalInProgress( IChainedInterface * chainedInput );

   bool                    AddInputChainData( BasePacket* packet, U32 connectionId );
   bool                    AddOutputChainData( BasePacket* packet, U32 connectionId );
   bool                    SendPacketToLoginServer( BasePacket* packet, U32 connectionId );

   bool                    AddQueryToOutput( PacketDbQuery* packet );

   const SalesManager*     GetSalesOrganizer() const { return m_salesManager; }
   const StringLookup*     GetStringLookup() const { return m_stringLookup; }
   const PurchaseReceiptManager* GetReceiptManager() const { return m_purchaseReceiptManager; }

   bool                    GetUser( const string& uuid, UserAccountPurchase*& user );
   const string            GetUuid( U32 connectionId ) const;

   //-----------------------------------------------------------

private:
   void                    RequestAdminSettings();
   void                    HandleAdminSettings( const PacketDbQueryResult* dbResult );

   bool                    HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId );
   bool                    ConnectUser( const PacketPrepareForUserLogin* loginPacket );
   bool                    DisconnectUser( const PacketPrepareForUserLogout* loginPacket );
   bool                    StoreUserProductsOwned( PacketListOfUserProductsS2S* productNamesPacket );
   bool                    ExpireUser( const PacketLoginExpireUser* actualPacket );
   bool                    DeleteAllUsers();

   bool                    HandlePurchaseRequest( const PacketTournament_PurchaseTournamentEntry* packet, U32 connectionId );
   bool                    HandlePurchaseRefund( const PacketTournament_PurchaseTournamentEntryRefund* packet, U32 connectionId );

   void                    UpdateDbResults();
   bool                    HandleDbResult( const PacketDbQueryResult* dbResult );

   int                     CallbackFunction();
   bool                    ProcessInboundPacket( PacketStorage& storage );

   void                    AddServerNeedingUpdate( U32 serverId );

   typedef map< stringhash, UserAccountPurchase >  UAADMap;
   typedef pair< stringhash, UserAccountPurchase > UAADPair;
   typedef UAADMap::iterator                       UAADMapIterator;

   UAADMapIterator         GetUserByConnectionId( U32 connectionId );

   UAADMap                          m_userTickets;
   SalesManager*                    m_salesManager;
   PurchaseReceiptManager*          m_purchaseReceiptManager;
   StringLookup*                    m_stringLookup;
   bool                             m_isInitializing;
   bool                             m_isWaitingForAdminSettings;

   time_t                           m_initializingAdminSettingsTimeStamp;
};

///////////////////////////////////////////////////////////////////