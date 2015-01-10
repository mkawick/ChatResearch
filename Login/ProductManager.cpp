
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/StringUtils.h"
#include "../NetworkCommon/ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Logging/server_log.h"
#include "../NetworkCommon/Packets/LoginPacket.h"

#include "../NetworkCommon/Database/StringLookup.h"

#include <boost/lexical_cast.hpp>
#include "ProductManager.h"
#include "LoginMainThread.h"

/////////////////////////////////////////////////////////////////////////////////

ProductManager::ProductManager() : m_mainThread( NULL ),
                                 m_printFunctionNames( false ),
                                 m_isInitialized( false )
{
   time_t currentTime;
   time( &currentTime );
   m_initializingProductListTimeStamp = currentTime;
}

/////////////////////////////////////////////////////////////////////////////////

ProductManager::~ProductManager()
{
}

/////////////////////////////////////////////////////////////////////////////////

void     ProductManager:: Init()
{
   if( m_productList.size() == 0 )
   {
      time_t currentTime;
      time(& currentTime );
      // try every 15 seconds
      if( difftime( currentTime, m_initializingProductListTimeStamp ) > 15 )
      {
         m_initializingProductListTimeStamp = currentTime;
         if( m_printFunctionNames == true )
         {
            cout << "fn: " << __FUNCTION__ << endl;
         }

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id =           0;
         dbQuery->lookup =       LoginMainThread::QueryType_Products;
         dbQuery->meta =         "";
         dbQuery->serverLookup = QueryType_LoadProductInfo;

         // add in 0's last
         dbQuery->query = "SELECT * FROM product ORDER BY product_id DESC"; // WHERE product_id > 0 

         m_mainThread->AddQueryToOutput( dbQuery );
      }
   }
}

/////////////////////////////////////////////////////////////////////////////////

bool     ProductManager:: HandleQueryResult( const PacketDbQueryResult* dbResult )
{
   bool wasHandled = false;

   switch( dbResult->serverLookup )
   {
   case QueryType_LoadProductInfo:
      {
         if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )
         {
            string str = "Initialization failed: table does not exist ";
            wasHandled = false;
         }
         else
         {
            StoreAllProducts( dbResult );
            wasHandled = true;
         }
      }
      break;
   case QueryType_GetSingleProductInfo:
      {
         if( dbResult->successfulQuery == false ||
            dbResult->GetBucket().size() == 0 )
         {
            string str = "Product not found ";
            str += dbResult->meta;
            LogMessage( LOG_PRIO_ERR, str.c_str() );
            wasHandled = false;
         }
         else
         {
            StoreSingleProduct( dbResult );
            wasHandled = true;
         }
      }
      break;
   }
   return wasHandled;
}

/////////////////////////////////////////////////////////////////////////////////


void     ProductManager:: AddProduct( const PurchaseEntryExtended& product )
{
   assert( m_mainThread );

   std::string lowercase_productUuidname = product.productUuid; 
   std::transform( lowercase_productUuidname.begin(), lowercase_productUuidname.end(), lowercase_productUuidname.begin(), ::tolower );
   

   if( FindProductByVendorUuid( lowercase_productUuidname ) != ProductNotFound )
   {
      LogMessage( LOG_PRIO_ERR, "Duplicate product found by vendor uuid" );
      LogMessage( LOG_PRIO_ERR, "Invalid product: %d, name='%s', uuid='%s', filter='%s'", product.name.c_str(), lowercase_productUuidname.c_str(), lowercase_productUuidname.c_str() );
      return;
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       LoginMainThread::QueryType_Products;
   dbQuery->meta =         product.name.c_str();
   dbQuery->serverLookup = QueryType_AddProductInfo;
   dbQuery->isFireAndForget = true;

   U32 hash = static_cast<U32>( GenerateUniqueHash( product.name ) );
   string newUuid = GenerateUUID( hash );   

   LogMessage( LOG_PRIO_INFO, "New product entry: " );
   LogMessage( LOG_PRIO_INFO, " name: %s", product.name.c_str() );
   LogMessage( LOG_PRIO_INFO, " UUID: %s", newUuid.c_str() );

   dbQuery->query = "INSERT INTO product (product_id, uuid, name, filter_name, first_available) ";
   dbQuery->query += "VALUES( 0, '";// new products haven an id of 0
   dbQuery->query += newUuid;
   dbQuery->query += "', '%s', '%s', UTC_TIMESTAMP())";

   dbQuery->escapedStrings.insert( product.name );
   dbQuery->escapedStrings.insert( lowercase_productUuidname );

   m_mainThread->AddQueryToOutput( dbQuery );

   //--------------------------------------------------

   // pull back the results so that we have the index.
   dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       LoginMainThread::QueryType_Products;
   dbQuery->meta =         product.name.c_str();
   dbQuery->serverLookup = QueryType_GetSingleProductInfo;

   dbQuery->query = "SELECT * FROM product WHERE uuid = '%s'";
   dbQuery->escapedStrings.insert( newUuid );

   m_mainThread->AddQueryToOutput( dbQuery );
}

/////////////////////////////////////////////////////////////////////////////////
////////////////////////////////     utilities  /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

int         ProductManager:: FindProductByName( const string& name )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   std::string lowercase_productUUID = name; 
   std::transform( lowercase_productUUID.begin(), lowercase_productUUID.end(), lowercase_productUUID.begin(), ::tolower );

   ProductList::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->vendorUuid == name )   // NOTE: these names may vary
      {
	      int index = static_cast< int >( it - m_productList.begin() );
         return index;
      }
      it++;
   }
   return ProductManager::ProductNotFound;
}

/////////////////////////////////////////////////////////////////////////////////

bool        ProductManager:: FindProductByLookupName( const string& lookupName, ProductInfo& productDefn )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   ProductList::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->lookupName == lookupName )   // NOTE: these names may vary
      {
	      productDefn = *it;
         return true;
      }
      it++;
   }
   return false;
}

/////////////////////////////////////////////////////////////////////////////////

bool        ProductManager:: FindProductByUuid( const string& uuid, ProductInfo& returnPi  )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   vector< ProductInfo >::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->uuid == uuid )
      {
         returnPi = *it;
         return true;
      }
      it++;
   }

   return false;
}

/////////////////////////////////////////////////////////////////////////////////

int        ProductManager:: FindProductByVendorUuid( const string& vendorUuid )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   std::string lowercase_productUUID = vendorUuid; 
   std::transform( lowercase_productUUID.begin(), lowercase_productUUID.end(), lowercase_productUUID.begin(), ::tolower );

   vector< ProductInfo >::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->vendorUuid == lowercase_productUUID )
      {
         int index = static_cast< int >( it - m_productList.begin() );
         return index;
      }
      it++;
   }

   return ProductManager::ProductNotFound;
}

/////////////////////////////////////////////////////////////////////////////////

bool        ProductManager:: GetProductByIndex( int index, ProductInfo& returnPi )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   if( index > (int) m_productList.size() )
   {
      return false;
   }
   returnPi = m_productList[ index] ;
   return true;
}

/////////////////////////////////////////////////////////////////////////////////

bool        ProductManager:: GetProductByProductId( int productId, ProductInfo& returnPi  )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   vector< ProductInfo >::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      if( it->productId == productId )
      {
         returnPi = *it;
         return true;
      }
      it++;
   }

   return false;
}

/////////////////////////////////////////////////////////////////////////////////

void     ProductManager:: StoreAllProducts( const PacketDbQueryResult* dbResult )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   ProductTable            enigma( dbResult->bucket );

   ProductTable::iterator  it = enigma.begin();
   //if( m_printPacketTypes == true )
   {
      LogMessage( LOG_PRIO_INFO, "products loaded: enigma size=%d", enigma.size() );
   }
  // int numProducts = static_cast< int >( dbResult->bucket.size() );          
   while( it != enigma.end() )
   {
      ProductTable::row       row = *it++;

      ProductInfo productDefn;
      productDefn.uuid =                  row[ TableProduct::Column_uuid ];
      productDefn.name =                  row[ TableProduct::Column_name ];
      int id =                            boost::lexical_cast< int >( row[ TableProduct::Column_id ] );
      productDefn.vendorUuid =            row[ TableProduct::Column_vendor_uuid ];
      if( row[ TableProduct::Column_is_hidden ] == "1" )
         productDefn.isHidden =           true;
      else 
         productDefn.isHidden =           false;

      string productId =                  row[ TableProduct::Column_product_id ];
      if( productId == "NULL" || productId.size() == 0 || productId == "0" )
      {
         if( productDefn.vendorUuid.size() == 0 )
         {
            LogMessage( LOG_PRIO_ERR, "Invalid product: %d, name='%s', uuid='%s', filter='%s'", id, productDefn.name.c_str(), productDefn.uuid.c_str(), productDefn.vendorUuid.c_str() );
            continue;// invalid product
         }
         // the filter is still useful for preventing further additions
         LogMessage( LOG_PRIO_ERR, "Invalid product: %d, name='%s', uuid='%s', filter='%s'", id, productDefn.name.c_str(), productDefn.uuid.c_str(), productDefn.vendorUuid.c_str() );
      }

      if( FindProductByVendorUuid( productDefn.vendorUuid ) != ProductNotFound )
      {
         LogMessage( LOG_PRIO_ERR, "Duplicate product found by vendor uuid" );
         LogMessage( LOG_PRIO_ERR, "Invalid product: %d, name='%s', uuid='%s', filter='%s'", id, productDefn.name.c_str(), productDefn.uuid.c_str(), productDefn.vendorUuid.c_str() );
      }

      std::string lowercase_productUUID = productDefn.vendorUuid; 
      std::transform( lowercase_productUUID.begin(), lowercase_productUUID.end(), lowercase_productUUID.begin(), ::tolower );
      productDefn.vendorUuid = lowercase_productUUID;

      productDefn.productId  =            boost::lexical_cast< int >( productId );
      
      
      productDefn.Begindate =             row[ TableProduct::Column_begin_date ];
      productDefn.lookupName =            row[ TableProduct::Column_name_string ];
      string temp = row[ TableProduct::Column_product_type ];
      if( temp == "" )
         temp = "0";

      productDefn.parentId =              boost::lexical_cast< int >( row[ TableProduct::Column_parent_id ] );

      productDefn.productType  =          boost::lexical_cast< int >( temp );
      
      productDefn.iconName =              row[ TableProduct::Column_icon_lookup ];
      productDefn.convertsToProductId =   boost::lexical_cast< int >( row[ TableProduct::Column_converts_to_product_id ] );
      productDefn.convertsToQuantity =    boost::lexical_cast< int >( row[ TableProduct::Column_converts_to_quantity ] );

      m_productList.push_back( productDefn );
   }

   //if( m_printPacketTypes == true )
   {
      LogMessage( LOG_PRIO_ERR, "products loaded: size=%d", m_productList.size() );
   }

   m_isInitialized = true;
}

/////////////////////////////////////////////////////////////////////////////////

void     ProductManager:: StoreSingleProduct( const PacketDbQueryResult* dbResult )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   ProductTable            enigma( dbResult->bucket );

   string vendorUuid;
   string filterUuid;

   
   ProductTable::row row = *enigma.begin();
   //if( it != enigma.end() )
   {
      ProductInfo productDefn;
      productDefn.productId  =            boost::lexical_cast< int >( row[ TableProduct::Column_product_id ] );
      productDefn.uuid =                  row[ TableProduct::Column_uuid ];
      productDefn.name =                  row[ TableProduct::Column_name ];
      productDefn.vendorUuid =            row[ TableProduct::Column_vendor_uuid ];

      if( row[ TableProduct::Column_is_hidden ] == "1" )
         productDefn.isHidden =           true;
      else 
         productDefn.isHidden =           false;

      std::string lowercase_productUUID = productDefn.vendorUuid; 
      std::transform( lowercase_productUUID.begin(), lowercase_productUUID.end(), lowercase_productUUID.begin(), ::tolower );
      productDefn.vendorUuid = lowercase_productUUID;

      productDefn.Begindate =             row[ TableProduct::Column_begin_date ];
      productDefn.lookupName =            row[ TableProduct::Column_name_string ];

      string temp =                       row[ TableProduct::Column_product_type ];
      if( temp == "" )
         temp = "0";
      productDefn.productType  =          boost::lexical_cast< int >( temp );
      
      vendorUuid = productDefn.vendorUuid;
      filterUuid = productDefn.uuid;

      if( FindProductByVendorUuid( vendorUuid ) != ProductNotFound )
      {
         LogMessage( LOG_PRIO_ERR, "StoreSingleProduct failed" );
         LogMessage( LOG_PRIO_ERR, "New product already stored" );
         return;
      }

      m_productList.push_back( productDefn );
   }

   AddProductToAllUsersWaitingForIt( vendorUuid );
   
}

/////////////////////////////////////////////////////////////////////////////////

bool     ProductManager:: CanProductBePurchasedMultipleTimes( const ProductInfo& productInfo )
{
   // todo: verify that the product cannot be bought multiple times. 
   // Some IAP/DLC can be purchased multiple times. e.g. Tournament tickets, consumables
   return false;
}

/////////////////////////////////////////////////////////////////////////////////

int      ProductManager:: CountNumOfAvailableProducts()
{
   int count = 0;
   ProductList::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      const ProductInfo& pi = *it ++;
      if( pi.isHidden == true )
         continue;

      count ++;
   }
   return count;
}

/////////////////////////////////////////////////////////////////////////////////

void     ProductManager:: PackageProductToSendToClient( const ProductInfo& pi, ProductBriefPacketed& brief, U32 languageId )
{
   StringLookup*  stringLookup = m_mainThread->GetStringLookup();
   string name = pi.name;
   if( pi.lookupName.size() != 0 )
   {
      name = stringLookup->GetString( pi.lookupName, languageId );
   }
   brief.uuid =            pi.uuid;
   brief.vendorUuid =      pi.vendorUuid;
   brief.localizedName =   name;

   /* if( pi.productType != GameProductType_Game && 
   pi.productType != GameProductType_Dlc && 
   pi.productType != GameProductType_Consumable )
   continue;*/

   if( pi.parentId )
   {
      ProductInfo parent;
      GetProductByProductId( pi.parentId, parent );// reuse this local variable
      brief.parentUuid = parent.uuid;
   }
   brief.iconName = pi.iconName;
   brief.productType = pi.productType;
}

/////////////////////////////////////////////////////////////////////////////////

bool     ProductManager:: SendListOfAvailableProducts( U32 connectionId, U32 gatewayId, U32 platformId, U32 languageId )
{
   PacketRequestListOfProductsResponse* response = new PacketRequestListOfProductsResponse();
   response->platformId = platformId;
   
   int totalCount = CountNumOfAvailableProducts();

   response->products.SetIndexParams( 0, totalCount );
   int   numPacketsSent = 0;
   const int numProductsPerPacket = 15;

   ProductList::iterator it = m_productList.begin();
   while( it != m_productList.end() )
   {
      const ProductInfo& pi = *it ++;
      if( pi.isHidden == true )
         continue;

      ProductBriefPacketed    brief;
      PackageProductToSendToClient( pi, brief, languageId );

      response->products.push_back( brief );
      if( response->products.size() == numProductsPerPacket )
      {
         numPacketsSent++;
         m_mainThread->SendPacketToGateway( response, connectionId, gatewayId );
         if( numPacketsSent* numProductsPerPacket < totalCount )
         {
            response = new PacketRequestListOfProductsResponse();
            response->platformId = platformId;
            response->products.SetIndexParams( numPacketsSent* numProductsPerPacket, totalCount );
         }
         else
         {
            response = NULL;
            break;
         }
      }
   }

   if( response != NULL )
   {
      m_mainThread->SendPacketToGateway( response, connectionId, gatewayId );
   }
   return true;
}

/////////////////////////////////////////////////////////////////////////////////
/*
void     ProductManager:: AddNewProductToDb( const PurchaseEntryExtended& product )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   std::string lowercase_productUuidname = product.productUuid; 
   std::transform( lowercase_productUuidname.begin(), lowercase_productUuidname.end(), lowercase_productUuidname.begin(), ::tolower );
   

   if( FindProductByVendorUuid( lowercase_productUuidname ) != DiplodocusLogin::ProductNotFound )
   {
      LogMessage( LOG_PRIO_ERR, "Duplicate product found by vendor uuid" );
      LogMessage( LOG_PRIO_ERR, "Invalid product: %d, name='%s', uuid='%s', filter='%s'", product.name.c_str(), lowercase_productUuidname.c_str(), lowercase_productUuidname.c_str() );
      return;
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;

   dbQuery->id =           0;
   dbQuery->lookup =       LoginMainThread::QueryType_Products;
   dbQuery->meta =         product.name.c_str();
   dbQuery->serverLookup = QueryType_AddProductInfo;
   dbQuery->isFireAndForget = true;

   U32 hash = static_cast<U32>( GenerateUniqueHash( product.name ) );
   string newUuid = GenerateUUID( hash );   

   LogMessage( LOG_PRIO_INFO, "New product entry: " );
   LogMessage( LOG_PRIO_INFO, " name: %s", product.name.c_str() );
   LogMessage( LOG_PRIO_INFO, " UUID: %s", newUuid.c_str() );

   dbQuery->query = "INSERT INTO product (product_id, uuid, name, filter_name, first_available) ";
   dbQuery->query += "VALUES( 0, '";// new products haven an id of 0
   dbQuery->query += newUuid;
   dbQuery->query += "', '%s', '%s', UTC_TIMESTAMP())";

   dbQuery->escapedStrings.insert( product.name );
   dbQuery->escapedStrings.insert( lowercase_productUuidname );

   AddQueryToOutput( dbQuery );

   //--------------------------------------------------

   // pull back the results so that we have the index.
   dbQuery = new PacketDbQuery;
   dbQuery->id =           0;
   dbQuery->lookup =       LoginMainThread::QueryType_Products;
   dbQuery->meta =         product.name.c_str();
   dbQuery->serverLookup = QueryType_GetSingleProductInfo;

   dbQuery->query = "SELECT * FROM product WHERE uuid = '%s'";
   dbQuery->escapedStrings.insert( newUuid );

   AddQueryToOutput( dbQuery );
}*/

/////////////////////////////////////////////////////////////////////////////////

void  ProductManager:: AddProductToAllUsersWaitingForIt( const string& vendorUuid )
{
   // now that we've added the new product, see which users needed it.
   //Threading::MutexLock locker( m_inputChainListMutex );
  /* UserConnectionMapIterator userIt = m_userConnectionMap.begin();
   while( userIt != m_userConnectionMap.end() )
   {
      vector< ProductInfo >& userProducts = userIt->second.productsWaitingForInsertionToDb;
      vector< ProductInfo >::iterator productIt = userProducts.begin();
      while( productIt != userProducts.end() )
      {
         if( productIt->vendorUuid == vendorUuid )
         {
            double price = productIt->price;
            userIt->second.WriteProductToUserRecord( filterUuid, price );
            userProducts.erase( productIt );

            if( userProducts.size() == 0 )
            {
               SendListOfUserProductsToAssetServer( userIt->first );
            }
            break;
         }
         productIt ++;
      }

      userIt ++;
   }*/
}

/////////////////////////////////////////////////////////////////////////////////
