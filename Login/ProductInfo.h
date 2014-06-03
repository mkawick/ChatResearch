// ProductInfo.h

#pragma once

#include <string>
using namespace std; 

//-----------------------------------------------------------------------------------------

struct ProductInfo
{
   ProductInfo() : productId( 0 ), productType( 0 ), quantity( 0 ), parentId( 0 ) {}
   int      productId;
   string   uuid;
   string   name;
   string   filterName;
   string   Begindate;
   string   lookupName;
   int      productType;
   int      parentId;

   double   price;
   float    quantity;
};

//-----------------------------------------------------------------------------------------

struct ProductBrief
{
   ProductBrief() : productDbId( 0 ), quantity( 0 ) {}
   int      productDbId;
   string   uuid;
   string   filterName;
   float    quantity;
};

//-----------------------------------------------------------------------------------------
