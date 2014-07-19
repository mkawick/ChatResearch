// PurchaseReceiptManager.cpp

#include <time.h>
#include <iostream>
#include <string>
#include <set>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>


#include "PurchaseReceiptManager.h"

#include "../NetworkCommon/Database/StringLookup.h"
#include "../NetworkCommon/Packets/PurchasePacket.h"

struct ReceiptTempData: public PacketPurchase_ValidatePurchaseReceipt
{
   ReceiptTempData& operator = ( const PacketPurchase_ValidatePurchaseReceipt* receipt )
   {
      purchaseItemId =  receipt->purchaseItemId;
      quantity =        receipt->quantity;
      transactionId =   receipt->transactionId;
      //receipt; // do not copy
      platformId =      receipt->platformId;
      return *this;
   }
};

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

PurchaseReceiptManager::PurchaseReceiptManager( U32 id, ParentQueryerPtr parent, string& query, bool runQueryImmediately ) : ParentType( id, 20, parent, runQueryImmediately ),
                              m_isServicingReceipts( false ),
                              m_isInitializing( true ),
                              m_salesManager( NULL )
                              /*,
                              m_hasSendProductRequest( false )*/
{
   SetQuery( query );
}

/////////////////////////////////////////////////////////////////////////////////////////

void     PurchaseReceiptManager::Update( time_t currentTime )
{
   if( m_isInitializing )
   {
     /* if( m_hasSendProductRequest == false )
      {
         m_hasSendProductRequest = true;
         RequestAllProducts();
      }*/
      m_isInitializing = false;
   }
   else
   {
      // request strings first for all of the sales.
      ParentType::Update( currentTime, m_isServicingReceipts );
      if( m_isServicingReceipts )
         ValidateReceipts();
   }
}

/////////////////////////////////////////////////////////////////////////////////////////

void  PurchaseReceiptManager::ValidateReceipts()
{
   if( m_receiptsToValidate.size() != 0 )
   {
      while( m_receiptsToValidate.size() )
      {
         const PurchaseReceiptTracking& prt = m_receiptsToValidate.front();

         // todo, send http request

         m_receiptsToValidate.pop_front();
      }
      
      return;
   }
   else
   {
      m_isServicingReceipts = false;
   }
}

/////////////////////////////////////////////////////////////////////////////////////////

bool     PurchaseReceiptManager::HandleResult( const PacketDbQueryResult* dbResult )
{
   //DiplodocusPurchase::QueryType_ReceiptInsert,
   //DiplodocusPurchase::QueryType_ReceiptLookup, 
   int lookupType = dbResult->lookup;
   if( lookupType == static_cast<int>( m_queryType ) )
   {
      //SetValueOnExit< bool >           setter( m_isServicingReceipts, false );// due to multiple exit points...

      if( m_endpointValidation.size() != 0 )
      {
         PurchaseReceiptParser            enigma( dbResult->bucket );
         PurchaseReceiptParser::iterator  it = enigma.begin();
         if( enigma.m_bucket.size() > 0 )
         {
            
        /*    cout << "PurchaseReceiptManager:" << endl;
            cout << " Successful query = " << m_queryString << endl;
            cout << "No receipts to service " << endl;*/
         }  
         while( it != enigma.end() )
         {
            PurchaseReceiptTracking prt;
            prt = *it++;
            // send HTTP requests to apple for each of these.
            m_receiptsToValidate.push_back( prt );
         }
      }
      return true;
   }
   if( lookupType == DiplodocusPurchase::QueryType_ReceiptInsert )
   {
      ReceiptTempData* data = static_cast< ReceiptTempData* >( dbResult->customData );
      if( dbResult->successfulQuery == true )
      {
         cout << "successful receipt additions" << endl;
      }
      else
      {
         cout << "receipt additions failed Time:" << GetDateInUTC() << endl;
      }
      delete data;
      return true;
   }

   // m_salesManager
   return false;
}

/*void  RequestAllProducts()
{
   string query = "SELECT * FROM procuct";
}*/

/////////////////////////////////////////////////////////////////////////////////////////

bool  PurchaseReceiptManager::WriteReceipt( const PacketPurchase_ValidatePurchaseReceipt* receipt, U32 userId, const string& userUuid )
{
   /*
   INSERT INTO playdek.purchase_receipts (user_id, user_uuid, transaction_id, receipt, receipt_hash, platform_id, product_purchased, product_purchased_count ) 
   VALUES( 1, "38cabbad2461e678", "AABBCCDDEEFFGGHH", "a long receipt which is filled with garbage", 12234456, 1, "fe78c73d96db90cf", 1 );
   */

   if( userId == 0 )
      return false;
   if( userUuid.size() < 2 )
      return false;

   string productUuid = receipt->purchaseItemId;

   stringhash lookupHash = GenerateUniqueHash( receipt->receipt );

   string insertQuery = "INSERT INTO playdek.purchase_receipts (user_id, user_uuid, transaction_id, receipt, receipt_hash, platform_id, product_purchased_uuid, product_purchased_count ) VALUES( ";

   insertQuery += boost::lexical_cast< string >( userId );
   insertQuery += ", '%s', '%s', '%s', ";
   insertQuery += boost::lexical_cast< string >( lookupHash );
   insertQuery += ", ";
   insertQuery += boost::lexical_cast< string >( receipt->platformId );
   insertQuery += ", '%s', "; // product id
   insertQuery += boost::lexical_cast< string >( receipt->quantity );
   insertQuery += ");";
   //1, "38cabbad2461e678", "AABBCCDDEEFFGGHH", "a long receipt which is filled with garbage", 12234456, 1, "fe78c73d96db90cf", 1


   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =     0;
   dbQuery->lookup = DiplodocusPurchase::QueryType_ReceiptInsert;

   ReceiptTempData* data = new ReceiptTempData;
   (*data) = ( receipt );
   dbQuery->customData = data;
   //dbQuery->isFireAndForget = true;

   dbQuery->escapedStrings.insert( userUuid );
   dbQuery->escapedStrings.insert( receipt->transactionId );
   dbQuery->escapedStrings.insert( receipt->receipt );
   dbQuery->escapedStrings.insert( receipt->purchaseItemId );
   dbQuery->query = insertQuery;

   m_parent->AddQueryToOutput( dbQuery );

   if( m_salesManager )
   {
      m_salesManager->PerformSimpleInventoryAddition( userUuid, productUuid, receipt->quantity, true );
   }

   return true;
}

/////////////////////////////////////////////////////////////////////////////////////////

void  PurchaseReceiptManager::AddProductToUserInventory( const string& productUuid, int quantity, const string& userUuid )
{
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

PurchaseReceiptTracking::PurchaseReceiptTracking()
{
}

/////////////////////////////////////////////////////////////////////////////////////////

PurchaseReceiptTracking& PurchaseReceiptTracking::operator = ( PurchaseReceiptParser::row  row )
{

   index =                          boost::lexical_cast< int > ( row[ TablePurchaseReceipts::Column_index ] );
   user_id =                        boost::lexical_cast< int > ( row[ TablePurchaseReceipts::Column_user_id ] );
   user_uuid =                      row[ TablePurchaseReceipts::Column_user_uuid ];
   transaction_id =                 row[ TablePurchaseReceipts::Column_transaction_id ];
   date_received =                  row[ TablePurchaseReceipts::Column_date_received ];

   receipt =                        row[ TablePurchaseReceipts::Column_receipt ];
   receipt_hash =                   boost::lexical_cast< U64 > ( row[ TablePurchaseReceipts::Column_receipt_hash ] );
   platform_id =                    boost::lexical_cast< int > ( row[ TablePurchaseReceipts::Column_platform_id ] );
   product_purchased =              row[ TablePurchaseReceipts::Column_product_purchased ];
   product_purchased_count =        boost::lexical_cast< int > ( row[ TablePurchaseReceipts::Column_product_purchased_count ] );
   num_attempts_to_validate =       boost::lexical_cast< int > ( row[ TablePurchaseReceipts::Column_num_attempts_to_validate ] );
   date_last_validation_attempt =   row[ TablePurchaseReceipts::Column_date_last_validation_attempt ];
   validated_result =               boost::lexical_cast< int > ( row[ TablePurchaseReceipts::Column_validated_result ] );

   return *this;
}

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////
