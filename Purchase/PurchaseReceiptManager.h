#pragma once

#include <deque>
using namespace std;

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"

#include "PurchaseMainThread.h"

class DiplodocusPurchase;
class SalesManager;
class PacketPurchase_ValidatePurchaseReceipt;

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

class PurchaseReceiptTracking
{
public:
   PurchaseReceiptTracking();
   PurchaseReceiptTracking& operator = ( PurchaseReceiptParser::row  row );

   U32         index;
   U32         user_id;
   string      user_uuid;
   string      transaction_id;
   string      date_received;
   string      receipt;
   U64         receipt_hash;
   U32         platform_id;
   string      product_purchased;
   U32         product_purchased_count;
   U32         num_attempts_to_validate;
   string      date_last_validation_attempt;
   U8          validated_result;
};

//------------------------------------------------------------


///////////////////////////////////////////////////////////////////////////////////////////

class PurchaseReceiptManager : public QueryHandler< DiplodocusPurchase* >
{
public: 
   typedef QueryHandler< DiplodocusPurchase* > ParentType;

public:
   PurchaseReceiptManager( U32 id, ParentQueryerPtr parent, string& query, bool runQueryImmediately );
   bool     HandleResult( const PacketDbQueryResult* dbResult );

   void     SetSalesManager( SalesManager* manager ) { m_salesManager = manager; }

   bool     WriteReceipt( const PacketPurchase_ValidatePurchaseReceipt* receipt, U32 userId, const string& userUuid );

   void     Update( time_t currentTime );

   void     SetValidationEndpoint( const string& http ) { m_endpointValidation = http; }

private:

   void     ValidateReceipts();

   void     AddProductToUserInventory( const string& productUuid, int quantity, const string& userUuid );

   bool                                m_isServicingReceipts;
   bool                                m_isInitializing, m_hasSendProductRequest;
   deque< PurchaseReceiptTracking >    m_receiptsToValidate;
   set< string >                       m_usersBeingServiced;
   string                              m_endpointValidation;

   SalesManager*                       m_salesManager;
};

///////////////////////////////////////////////////////////////////////////////////////////
