// ProductManager.h

#pragma once

#include "ProductInfo.h"

class LoginMainThread;
/////////////////////////////////////////////////////////////////////////////////

class ProductManager
{
public:
   enum 
   {
      ProductNotFound = -1
   };
public:
   ProductManager();
   ~ProductManager();

   void  SetLoginMainThread( LoginMainThread* mainThread ) { m_mainThread = mainThread; }

   void  Init();
   void  AddProduct( const PurchaseEntryExtended& product );
   bool  HandleQueryResult( const PacketDbQueryResult* query );
   bool  IsFinishedInitializing() const { return m_isInitialized; }
   bool  CanProductBePurchasedMultipleTimes( const ProductInfo& productInfo );

   bool  SendListOfAvailableProducts( U32 connectionId, U32 gatewayId, U32 platformId, U32 languageId );

   //------------------------------------

   void  PrintFunctionNames( bool printingOn ) { m_printFunctionNames = printingOn; }

   // utils ... return ProductNotFound if product does not exist.
   int         FindProductByName( const string& name );
   bool        FindProductByLookupName( const string& lookupName, ProductInfo& productDefn );
   bool        FindProductByUuid( const string& uuid, ProductInfo& returnPi  );
   int         FindProductByVendorUuid( const string& vendorUuid );
   bool        GetProductByIndex( int index, ProductInfo& returnPi );
   bool        GetProductByProductId( int productId, ProductInfo& returnPi );

private:
   enum QueryType 
   {
      QueryType_LoadProductInfo,
      QueryType_AddProductInfo,
      QueryType_GetSingleProductInfo,
      QueryType_ProductStringLookup
   };

private:
   void     StoreAllProducts( const PacketDbQueryResult* dbResult );
   void     StoreSingleProduct( const PacketDbQueryResult* dbResult );
   void     AddProductToAllUsersWaitingForIt( const string& vendorUuid );
   int      CountNumOfAvailableProducts();
   void     PackageProductToSendToClient( const ProductInfo& pi, ProductBriefPacketed& brief, U32 languageId );

private:
   typedef vector< ProductInfo > ProductList;
   ProductList                   m_productList;

   time_t                        m_initializingProductListTimeStamp;
   LoginMainThread*              m_mainThread;
   bool                          m_printFunctionNames;
   bool                          m_isInitialized;
};

/////////////////////////////////////////////////////////////////////////////////

