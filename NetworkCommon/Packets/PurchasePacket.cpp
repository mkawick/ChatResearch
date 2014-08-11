#include "PurchasePacket.h"

///////////////////////////////////////////////////////////////

bool  PacketPurchase::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   BasePacket::SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketPurchase::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}


bool  PurchaseInfo::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, exchangeUuid, minorVersion );
   Serialize::In( data, bufferOffset, exchangeTitle, minorVersion );
   Serialize::In( data, bufferOffset, exchangeDescription, minorVersion );

   Serialize::In( data, bufferOffset, productSourceUuid, minorVersion );
   Serialize::In( data, bufferOffset, productSourceName, minorVersion );
   Serialize::In( data, bufferOffset, quantityRequiredSource, minorVersion );

   Serialize::In( data, bufferOffset, productDestUuid, minorVersion );
   Serialize::In( data, bufferOffset, productDestName, minorVersion );
   Serialize::In( data, bufferOffset, quantityGivenDest, minorVersion );

   Serialize::In( data, bufferOffset, advertisementAssetSource, minorVersion );
   Serialize::In( data, bufferOffset, iconHashSource, minorVersion );
   Serialize::In( data, bufferOffset, iconHashDest, minorVersion );
   Serialize::In( data, bufferOffset, beginDate, minorVersion );
   Serialize::In( data, bufferOffset, endDate, minorVersion );

   if( minorVersion > 1 )
   {
      Serialize::In( data, bufferOffset, junk1, minorVersion );
      Serialize::In( data, bufferOffset, junk2, minorVersion );
   }


   return true;
}

bool  PurchaseInfo::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, exchangeUuid, minorVersion );
   Serialize::Out( data, bufferOffset, exchangeTitle, minorVersion );
   Serialize::Out( data, bufferOffset, exchangeDescription, minorVersion );

   Serialize::Out( data, bufferOffset, productSourceUuid, minorVersion );
   Serialize::Out( data, bufferOffset, productSourceName, minorVersion );
   Serialize::Out( data, bufferOffset, quantityRequiredSource, minorVersion );

   Serialize::Out( data, bufferOffset, productDestUuid, minorVersion );
   Serialize::Out( data, bufferOffset, productDestName, minorVersion );
   Serialize::Out( data, bufferOffset, quantityGivenDest, minorVersion );

   Serialize::Out( data, bufferOffset, advertisementAssetSource, minorVersion );
   Serialize::Out( data, bufferOffset, iconHashSource, minorVersion );
   Serialize::Out( data, bufferOffset, iconHashDest, minorVersion );
   Serialize::Out( data, bufferOffset, beginDate, minorVersion );
   Serialize::Out( data, bufferOffset, endDate, minorVersion );

   if( minorVersion > 1 )
   {
      Serialize::Out( data, bufferOffset, junk1, minorVersion );
      Serialize::Out( data, bufferOffset, junk2, minorVersion );
   }

   return true;
}

///////////////////////////////////////////////////////////////

bool  PurchaseServerDebitItem::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, numToDebit, minorVersion );
   Serialize::In( data, bufferOffset, productUuidToSpend, minorVersion );

   return true;
}

bool  PurchaseServerDebitItem::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, numToDebit, minorVersion );
   Serialize::Out( data, bufferOffset, productUuidToSpend, minorVersion );

   return true;
}


int   GetItemNumToDebit( const SerializedVector< PurchaseServerDebitItem >& itemsToSpend, const string& uuid )
{
   int num = itemsToSpend.size();
   for( int i=0; i<num; i++ )
   {
      if( itemsToSpend[i].productUuidToSpend == uuid )
         return itemsToSpend[i].numToDebit;
   }

   return 0;
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_TestNotification::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, message, minorVersion );
   Serialize::In( data, bufferOffset, type, minorVersion );

   return true; 
}

bool  PacketPurchase_TestNotification::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, message, minorVersion );
   Serialize::Out( data, bufferOffset, type, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_Buy::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, purchaseUuid, minorVersion );

   return true; 
}

bool  PacketPurchase_Buy::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, purchaseUuid, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_BuyResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, purchaseUuid, minorVersion );
   Serialize::In( data, bufferOffset, success, minorVersion );

   return true; 
}

bool  PacketPurchase_BuyResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, purchaseUuid, minorVersion );
   Serialize::Out( data, bufferOffset, success, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_RequestListOfSales::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset, minorVersion );

   return true; 
}

bool  PacketPurchase_RequestListOfSales::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_RequestListOfSalesResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, thingsForSale, minorVersion );

   return true; 
}

bool  PacketPurchase_RequestListOfSalesResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, thingsForSale, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_ValidatePurchaseReceipt::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, purchaseItemId, minorVersion );
   Serialize::In( data, bufferOffset, quantity, minorVersion );
   Serialize::In( data, bufferOffset, transactionId, minorVersion );
   Serialize::In( data, bufferOffset, receipt, minorVersion );
   Serialize::In( data, bufferOffset, platformId, minorVersion );

   return true; 
}

bool  PacketPurchase_ValidatePurchaseReceipt::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, purchaseItemId, minorVersion );
   Serialize::Out( data, bufferOffset, quantity, minorVersion );
   Serialize::Out( data, bufferOffset, transactionId, minorVersion );
   Serialize::Out( data, bufferOffset, receipt, minorVersion );
   Serialize::Out( data, bufferOffset, platformId, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_ValidatePurchaseReceiptResponse::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset, minorVersion );
   Serialize::In( data, bufferOffset, errorCode, minorVersion );
   Serialize::In( data, bufferOffset, transactionId, minorVersion );

   return true; 
}

bool  PacketPurchase_ValidatePurchaseReceiptResponse::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset, minorVersion );
   Serialize::Out( data, bufferOffset, errorCode, minorVersion );
   Serialize::Out( data, bufferOffset, transactionId, minorVersion );

   return true; 
}

///////////////////////////////////////////////////////////////

