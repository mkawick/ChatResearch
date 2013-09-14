// ServerConstants.cpp

#include "ServerConstants.h"

#include <string.h>
#if PLATFORM == PLATFORM_WINDOWS
   #include <windows.h>
   #include <mmsystem.h>
#pragma warning (disable:4996)
#endif

const char* productNames [] = {
   "",
   "ascension",
   "dominion",
   "thunderstone",
   "wowcmg",
   "summonwar",
   "foodfight",
   "nightfall",
   "pennyarcade",
   "infinitecity",
   "agricola",
   "fluxx",
   "smashup"
};

const char* platformStrings[] = {
   "",
   "ios",
   "iphone",
   "android",
   "androidtablet",
   "pc",
   "mac",
   "vita",
   "xbox",
   "blackberry",
   "wii"
};


//////////////////////////////////////////////////////////////////////////

int   FindProductId( const char* value )
{
   int numProductNames = sizeof( productNames ) / sizeof( productNames[0] );
   for( int i=0; i< numProductNames; i++ )
   {
      if( stricmp(value, productNames[i] ) == 0 )
      {
         return i;
      }
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////

int   FindPlatformId( const char* value )
{
   int numPlatformNames = sizeof( platformStrings ) / sizeof( platformStrings[0] );
   for( int i=0; i< numPlatformNames; i++ )
   {
      if( stricmp(value, platformStrings[i] ) == 0 )
      {
         return i;
      }
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////

const char*   FindProductName( int productId )
{
   if( productId < 0 || productId >= sizeof(productNames)/sizeof( productNames[0]) )
      return NULL;
   return productNames[ productId ];
}

//////////////////////////////////////////////////////////////////////////

const char*   FindPlatformName( int platformId )
{
   if( platformId < 0 || platformId >= sizeof(platformStrings)/sizeof( platformStrings[0]) )
      return NULL;
   return platformStrings[ platformId ];
}


//////////////////////////////////////////////////////////////////////////
