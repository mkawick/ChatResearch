// PurchasePacket.h
#pragma once

#include "BasePacket.h"


struct PurchaseInfo
{
   UuidString        exchangeUuid;
   BoundedString80   exchangeTitle;
   BoundedString140  exchangeDescription;

   UuidString        productSourceUuid;
   BoundedString80   productSourceName;
   int               quantityRequiredSource;

   UuidString        productDestUuid;
   BoundedString80   productDestName;
   int               quantityGivenDest;

   BoundedString80   advertisementAssetSource;
   BoundedString80   iconHashSource;
   BoundedString80   iconHashDest;
   TimeString        beginDate, endDate;

 /*  U32      junk1;
   string   junk2;*/

   PurchaseInfo()
   {
      Clear();
   }
   void  Clear()
   {
      exchangeUuid.clear();
      exchangeTitle.clear();
      exchangeDescription.clear();
      productSourceUuid.clear();
      productSourceName.clear();
      quantityRequiredSource = 0;

      productDestUuid.clear();
      productDestName.clear();
      quantityGivenDest = 0;

      advertisementAssetSource.clear();
      iconHashSource.clear();
      iconHashDest.clear();
      beginDate.clear();
      endDate.clear();

   /*   junk1 = 3;
      junk2.clear();*/
   }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////////

struct PurchaseServerDebitItem
{
   PurchaseServerDebitItem() : numToDebit( 0 ){}
   PurchaseServerDebitItem( const PurchaseServerDebitItem& rhs ):numToDebit( rhs.numToDebit ), productUuidToSpend( rhs.productUuidToSpend ) {}
   PurchaseServerDebitItem( const string& uuid, int num ):numToDebit( num ), productUuidToSpend( uuid ) {}
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   int         numToDebit;
   UuidString  productUuidToSpend;
};

///////////////////////////////////////////////////////////////////


int   GetItemNumToDebit( const SerializedVector< PurchaseServerDebitItem >& itemsToSpend, const string& uuid );

///////////////////////////////////////////////////////////////////

class PacketPurchase : public BasePacket 
{
public:
   enum PurchaseType
   {
      PurchaseType_Base,
      PurchaseType_TestNotification,
      PurchaseType_EchoToServer,
      PurchaseType_EchoToClient,

      PurchaseType_Buy,
      PurchaseType_BuyResponse,
      PurchaseType_RequestListOfSales,
      PurchaseType_RequestListOfSalesResponse,

      PurchaseType_ValidatePurchaseReceipt,
      PurchaseType_ValidatePurchaseReceiptResponse
   };

public:
   PacketPurchase( int packet_type = PacketType_Purchase, int packet_sub_type = PurchaseType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketPurchase_TestNotification : public PacketPurchase
{
public:
   PacketPurchase_TestNotification() : PacketPurchase( PacketType_Purchase, PurchaseType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString140  message;
   int               type;
};

///////////////////////////////////////////////////////////////

class PacketPurchase_EchoToServer : public BasePacket
{
public:
   PacketPurchase_EchoToServer(): BasePacket( PacketType_Purchase, PacketPurchase::PurchaseType_EchoToServer ) {}
};

///////////////////////////////////////////////////////////////

class PacketPurchase_EchoToClient : public BasePacket
{
public:
   PacketPurchase_EchoToClient(): BasePacket( PacketType_Purchase, PacketPurchase::PurchaseType_EchoToClient ) {}
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketPurchase_Buy : public PacketPurchase
{
public:
   PacketPurchase_Buy() : PacketPurchase( PacketType_Purchase, PurchaseType_Buy ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  purchaseUuid;
};

///////////////////////////////////////////////////////////////

class PacketPurchase_BuyResponse: public PacketPurchase
{
public:
   PacketPurchase_BuyResponse() : PacketPurchase( PacketType_Purchase, PurchaseType_BuyResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   UuidString  purchaseUuid;
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketPurchase_RequestListOfSales : public PacketPurchase
{
public:
   PacketPurchase_RequestListOfSales() : PacketPurchase( PacketType_Purchase, PurchaseType_RequestListOfSales ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////

class PacketPurchase_RequestListOfSalesResponse : public PacketPurchase
{
public:
   PacketPurchase_RequestListOfSalesResponse() : PacketPurchase( PacketType_Purchase, PurchaseType_RequestListOfSalesResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   SerializedKeyValueVector< PurchaseInfo >   thingsForSale;
};

///////////////////////////////////////////////////////////////

class PacketPurchase_ValidatePurchaseReceipt : public PacketPurchase
{
public:
   PacketPurchase_ValidatePurchaseReceipt() : PacketPurchase( PacketType_Purchase, PurchaseType_ValidatePurchaseReceipt ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   BoundedString80            purchaseItemId;
   int                        quantity;
   BoundedString80            transactionId;
   BoundedLengthString<4600>  receipt;
   int      platformId;// = Platform_ios
};

///////////////////////////////////////////////////////////////

class PacketPurchase_ValidatePurchaseReceiptResponse : public PacketPurchase
{
public:
   PacketPurchase_ValidatePurchaseReceiptResponse() : PacketPurchase( PacketType_Purchase, PurchaseType_ValidatePurchaseReceiptResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   //SerializedKeyValueVector< PurchaseInfo >   thingsForSale;

   int                        errorCode;// 0 = none or success
   BoundedString80            transactionId;
};
      
///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

