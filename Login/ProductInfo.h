// ProductInfo.h

#pragma once

#include <string>
using namespace std; 

//-----------------------------------------------------------------------------------------

struct ProductInfo
{
   ProductInfo() : productId( 0 ), productType( 0 ) {}
   int      productId;
   string   uuid;
   string   name;
   string   filterName;
   string   Begindate;
   int      productType;

   double   price;
};

//-----------------------------------------------------------------------------------------
