// ProductInfo.h

#pragma once

#include <string>
using namespace std; 

//-----------------------------------------------------------------------------------------

struct ProductInfo
{
   ProductInfo() : productId( 0 ), 
                  productType( 0 ), 
                  parentId( 0 ),
                  convertsToProductId( 0 ),
                  convertsToQuantity( 0 ){}
   int      productId;
   string   uuid;
   string   name;
   string   vendorUuid;
   string   Begindate;
   string   lookupName;
   string   iconName;
   int      productType;
   int      parentId;

   double   price;

   int      convertsToProductId;
   int      convertsToQuantity;
   bool     isHidden;
   //float    quantity;
};

//-----------------------------------------------------------------------------------------

struct ProductBrief
{
   ProductBrief() : productDbId( 0 ), quantity( 0 ) {}
   int      productDbId;
   string   uuid;
   string   vendorUuid;
   string   localizedName;
   float    quantity;
};

//-----------------------------------------------------------------------------------------
