// PurchasePacket.h
#pragma once

#include "BasePacket.h"


struct PurchaseInfo
{
   string   exchangeUuid;
   string   exchangeTitle;
   string   exchangeDescription;

   string   productSourceUuid;
   string   productSourceName;
   int      quantityRequiredSource;

   string   productDestUuid;
   string   productDestName;
   int      quantityGivenDest;

   string   advertisementAssetSource;
   string   iconHashSource;
   string   iconHashDest;
   string   beginDate, endDate;

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
   }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////////

class PacketPurchase : public BasePacket 
{
public:
   enum PurchaseType
   {
      PurchaseType_Base,
      PurchaseType_TestNotification,

      PurchaseType_Buy,
      PurchaseType_BuyResponse,
      PurchaseType_RequestListOfSales,
      PurchaseType_RequestListOfSalesResponse

   };

public:
   PacketPurchase( int packet_type = PacketType_Purchase, int packet_sub_type = PurchaseType_Base ) : BasePacket( packet_type, packet_sub_type ) {}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////

class PacketPurchase_TestNotification : public PacketPurchase
{
public:
   PacketPurchase_TestNotification() : PacketPurchase( PacketType_Purchase, PurchaseType_TestNotification ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   message;
   int      type;
};


///////////////////////////////////////////////////////////////

class PacketPurchase_Buy : public PacketPurchase
{
public:
   PacketPurchase_Buy() : PacketPurchase( PacketType_Purchase, PurchaseType_Buy ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   purchaseUuid;
};

///////////////////////////////////////////////////////////////

class PacketPurchase_BuyResponse: public PacketPurchase
{
public:
   PacketPurchase_BuyResponse() : PacketPurchase( PacketType_Purchase, PurchaseType_BuyResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   purchaseUuid;
   bool     success;
};

///////////////////////////////////////////////////////////////

class PacketPurchase_RequestListOfSales : public PacketPurchase
{
public:
   PacketPurchase_RequestListOfSales() : PacketPurchase( PacketType_Purchase, PurchaseType_RequestListOfSales ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;
};

///////////////////////////////////////////////////////////////

class PacketPurchase_RequestListOfSalesResponse : public PacketPurchase
{
public:
   PacketPurchase_RequestListOfSalesResponse() : PacketPurchase( PacketType_Purchase, PurchaseType_RequestListOfSalesResponse ){  }

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   SerializedKeyValueVector< PurchaseInfo >   thingsForSale;
};

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////