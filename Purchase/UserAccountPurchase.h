// UserAccountPurchase.h

#pragma once

#include <time.h>
#include <set>
#include <vector>
#include <string>
using namespace std;

#include "../NetworkCommon/DataTypes.h"
#include "../NetworkCommon/UserAccount/UserAccountCommon.h"


class DiplodocusPurchase;
class PacketPurchase;
class PacketPurchase_Buy;
class PacketPurchase_RequestListOfSales;
class StringBucket;
class SalesManager;
class PurchaseReceiptManager;
class PacketTournament_PurchaseTournamentEntry;
class PacketTournament_PurchaseTournamentEntryRefund;
class PacketPurchase_ValidatePurchaseReceipt;

///////////////////////////////////////////////////////////////////

class UserAccountPurchase : public UserLoginBase
{
   enum Status
   {
      Status_initial_login,
      Status_query_assets_for_product,
      Status_awating_user_login,
      Status_assemble_assets_for_delivery,
      Status_send_files_to_client,
      Status_awaiting_cleanup
   };

public:
   
   //---------------------------------------

   UserAccountPurchase();
   ~UserAccountPurchase();

   //---------------------------------------

   void              SetServer( DiplodocusPurchase* assetManager ) { m_purchaseManager = assetManager; }
   void              SetSalesManager( SalesManager* manager ) { m_salesManager = manager; }
   void              SetReceiptManager( PurchaseReceiptManager* manager ) { m_purchaseReceiptManager = manager; }
   
   bool              HandleRequestFromClient( const PacketPurchase* packet, U32 connectionId ); 
   bool              MakePurchase( const PacketTournament_PurchaseTournamentEntry* packet, U32 connectionId );
   bool              MakeRefund( const PacketTournament_PurchaseTournamentEntryRefund* packet, U32 connectionId );

   void              Update();

   void              StoreProducts( const StringBucket& bucket );

   //---------------------------------------

private:

   bool              MakePurchase( const PacketPurchase_Buy* packet, U32 connectionId );   
   bool              GetListOfItemsForSale( const PacketPurchase_RequestListOfSales* packet, U32 connectionId );
   bool              EchoHandler( U32 connectionId );
   bool              HandleReceipt( const PacketPurchase_ValidatePurchaseReceipt* packet, U32 connectionId );

   bool              PerformTournamentPurchase();

   Status                     m_status;
   bool                       m_readyForCleanup;
   DiplodocusPurchase*        m_purchaseManager;
   SalesManager*              m_salesManager;
   PurchaseReceiptManager*    m_purchaseReceiptManager;

   set< string >              m_productFilterNames;
};

///////////////////////////////////////////////////////////////////
