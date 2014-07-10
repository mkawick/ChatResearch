#pragma once

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"

#include "PurchaseMainThread.h"

class DiplodocusPurchase;

//------------------------------------------------------------

class TablePurchaseReceipts
{
public:
   enum Columns
   {
      Column_index,
      Column_user_id,
      Column_user_uuid,
      Column_transaction_id,
      Column_date_received,
      Column_receipt,
      Column_receipt_hash,
      Column_platform_id,
      Column_product_purchased,
      Column_product_purchased_count,
      Column_num_attempts_to_validate,
      Column_date_last_validation_attempt,
      Column_validated_result,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TablePurchaseReceipts> PurchaseReceiptParser;


//------------------------------------------------------------

enum ValidatedResults
{
   ValidatedResults_no_result,
   ValidatedResults_invalid,
   ValidatedResults_valid
};
//------------------------------------------------------------
/*
class PurchaseTracking
{
public:
   string   userUuid;
   string   exchangeUuid;
   
   U32      connectionId;

   U32      fromOtherServerId;
   string   fromOtherServerTransactionId;

   SerializedVector< PurchaseServerDebitItem > itemsToSpend;
};*/

//------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////////////////

class PurchaseReceiptManager : public QueryHandler< DiplodocusPurchase* >
{
public: 
   typedef QueryHandler< DiplodocusPurchase* > ParentType;

public:
   PurchaseReceiptManager( U32 id, ParentQueryerPtr parent, string& query, bool runQueryImmediately );
   bool     HandleResult( const PacketDbQueryResult* dbResult );

 /*  bool     GetListOfItemsForSale( PacketPurchase_RequestListOfSalesResponse* packet, int userLanguageIndex = 1 );// english
   bool     PerformSale( const string& purchaseUuid, const UserTicket& userPurchasing, U32 serverIdentifier = 0, string serverTransactionUuid = "" );
   bool     PerformSale( const SerializedVector< PurchaseServerDebitItem >& itemsToSpend, const UserTicket& userPurchasing, U32 serverIdentifier, string serverTransactionUuid = "" );

   int      GetProductType( const string& uuid );*/

   void     Update( time_t currentTime );

private:

  /* bool     FindItem( const string& exchangeUuid, ExchangeEntry& ee );
   string   LookupString( const string& name, int languageId );

   bool     RequestAllProducts();
   bool     SendTournamentPurchaseResultBackToServer( U32 serverIdentifier, string serverTransactionUuid, int result );
   void     VerifyThatUserHasEnoughMoneyForEntry2( const PacketDbQueryResult* dbResult );*/

   bool     m_isServicingReceipts;
   bool     m_isInitializing, m_hasSendProductRequest;
//   vector< ExchangeEntry > exchangeRates;
   set< string >           m_usersBeingServiced;

   //map< string, Product >  m_productMapByUuid;
   //typedef pair< string, Product >  ProductUuidPair;
};

///////////////////////////////////////////////////////////////////////////////////////////
