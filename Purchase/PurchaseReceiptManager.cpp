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



PurchaseReceiptManager::PurchaseReceiptManager( U32 id, ParentQueryerPtr parent, string& query, bool runQueryImmediately ) : ParentType( id, 20, parent, runQueryImmediately ),
                              m_isServicingReceipts( false ),
                              m_isInitializing( true )/*,
                              m_hasSendProductRequest( false )*/
{
}
/*
PurchaseReceiptManager::~PurchaseReceiptManager()
{
}*/

void     PurchaseReceiptManager::Update( time_t currentTime )
{
   if( m_isInitializing )
   {
     /* if( m_hasSendProductRequest == false )
      {
         m_hasSendProductRequest = true;
         RequestAllProducts();
      }*/
   }
   else
   {
      // request strings first for all of the sales.
      ParentType::Update( currentTime, m_isServicingReceipts );
   }
}

/*
INSERT INTO playdek.purchase_receipts (user_id, user_uuid, transaction_id, receipt, receipt_hash, platform_id, product_purchased, product_purchased_count ) 
VALUES( 1, "38cabbad2461e678", "AABBCCDDEEFFGGHH", "a long receipt which is filled with garbage", 12234456, 1, "fe78c73d96db90cf", 1 );
*/

bool     PurchaseReceiptManager::HandleResult( const PacketDbQueryResult* dbResult )
{
   //DiplodocusPurchase::QueryType_ReceiptInsert,
   //DiplodocusPurchase::QueryType_ReceiptLookup, 
   int lookupType = dbResult->lookup;
   if( lookupType == static_cast<int>( m_queryType ) )
   {
      SetValueOnExit< bool >           setter( m_isServicingReceipts, false );// due to multiple exit points...

      PurchaseReceiptParser            enigma( dbResult->bucket );
      PurchaseReceiptParser::iterator  it = enigma.begin();
      if( enigma.m_bucket.size() > 0 )
      {
         //cout << " Successful query = " << m_queryString << endl;
         //exchangeRates.clear();
      }  
      while( it != enigma.end() )
      {
       /*  ExchangeEntry entry;
         entry = *it++;
         exchangeRates.push_back( entry );*/
      }
      return true;
   }
   return false;
}