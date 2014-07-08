#include "PurchasePacket.h"

///////////////////////////////////////////////////////////////

bool  PacketPurchase::SerializeIn( const U8* data, int& bufferOffset )
{ 
   BasePacket::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketPurchase::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   BasePacket::SerializeOut( data, bufferOffset );

   return true; 
}


bool  PurchaseInfo::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, exchangeUuid );
   Serialize::In( data, bufferOffset, exchangeTitle );
   Serialize::In( data, bufferOffset, exchangeDescription );

   Serialize::In( data, bufferOffset, productSourceUuid );
   Serialize::In( data, bufferOffset, productSourceName );
   Serialize::In( data, bufferOffset, quantityRequiredSource );

   Serialize::In( data, bufferOffset, productDestUuid );
   Serialize::In( data, bufferOffset, productDestName );
   Serialize::In( data, bufferOffset, quantityGivenDest );

   Serialize::In( data, bufferOffset, advertisementAssetSource );
   Serialize::In( data, bufferOffset, iconHashSource );
   Serialize::In( data, bufferOffset, iconHashDest );
   Serialize::In( data, bufferOffset, beginDate );
   Serialize::In( data, bufferOffset, endDate );

   return true;
}

bool  PurchaseInfo::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, exchangeUuid );
   Serialize::Out( data, bufferOffset, exchangeTitle );
   Serialize::Out( data, bufferOffset, exchangeDescription );

   Serialize::Out( data, bufferOffset, productSourceUuid );
   Serialize::Out( data, bufferOffset, productSourceName );
   Serialize::Out( data, bufferOffset, quantityRequiredSource );

   Serialize::Out( data, bufferOffset, productDestUuid );
   Serialize::Out( data, bufferOffset, productDestName );
   Serialize::Out( data, bufferOffset, quantityGivenDest );

   Serialize::Out( data, bufferOffset, advertisementAssetSource );
   Serialize::Out( data, bufferOffset, iconHashSource );
   Serialize::Out( data, bufferOffset, iconHashDest );
   Serialize::Out( data, bufferOffset, beginDate );
   Serialize::Out( data, bufferOffset, endDate );

   return true;
}

///////////////////////////////////////////////////////////////

bool  PurchaseServerDebitItem::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, numToDebit );
   Serialize::In( data, bufferOffset, productUuidToSpend );

   return true;
}

bool  PurchaseServerDebitItem::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, numToDebit );
   Serialize::Out( data, bufferOffset, productUuidToSpend );

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

bool  PacketPurchase_TestNotification::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, message );
   Serialize::In( data, bufferOffset, type );

   return true; 
}

bool  PacketPurchase_TestNotification::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, message );
   Serialize::Out( data, bufferOffset, type );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_Buy::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, purchaseUuid );

   return true; 
}

bool  PacketPurchase_Buy::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, purchaseUuid );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_BuyResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, purchaseUuid );
   Serialize::In( data, bufferOffset, success );

   return true; 
}

bool  PacketPurchase_BuyResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, purchaseUuid );
   Serialize::Out( data, bufferOffset, success );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_RequestListOfSales::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset );

   return true; 
}

bool  PacketPurchase_RequestListOfSales::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset );

   return true; 
}

///////////////////////////////////////////////////////////////

bool  PacketPurchase_RequestListOfSalesResponse::SerializeIn( const U8* data, int& bufferOffset )
{ 
   PacketPurchase::SerializeIn( data, bufferOffset );
   Serialize::In( data, bufferOffset, thingsForSale );

   return true; 
}

bool  PacketPurchase_RequestListOfSalesResponse::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   PacketPurchase::SerializeOut( data, bufferOffset );
   Serialize::Out( data, bufferOffset, thingsForSale );

   return true; 
}

///////////////////////////////////////////////////////////////

