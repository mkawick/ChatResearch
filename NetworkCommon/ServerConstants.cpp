// ServerConstants.cpp

#include "ServerConstants.h"

#include <stdio.h>
#include <string.h>
#if PLATFORM == PLATFORM_WINDOWS
   #include <windows.h>
   #include <mmsystem.h>
#pragma warning (disable:4996)

#else

#define stricmp strcasecmp

#endif

const char* DefaultLoadBalancerAddress = "mber.pub.playdekgames.com";

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
   "smashup", 
   "tantocuore",
   "waterdeep",
   "twilight_struggle",
   "unsung_story",
   "mber"
};

const char* platformStrings[] = {
   "",
   "ios",
   //"iphone",
   "android",
   //"androidtablet",
   "pc",
   "mac",
   "vita",
   "sony",
   "xbox",
   "blackberry",
   "wii",
   "phone",
   "all"
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

int   GetIndexOfPlatformAll()
{
   int numPlatformNames = sizeof( platformStrings ) / sizeof( platformStrings[0] );
   for( int i=0; i< numPlatformNames; i++ )
   {
      if( stricmp( "all", platformStrings[i] ) == 0 )
      {
         return i;
      }
   }
   return 0;
}

int   GetIndexOfPlatformMber()
{
   int numProductNames = sizeof( productNames ) / sizeof( productNames[0] );
   for( int i=0; i< numProductNames; i++ )
   {
      if( stricmp("mber", productNames[i] ) == 0 )
      {
         return i;
      }
   }
   return 0;
}

//////////////////////////////////////////////////////////////////////////

const char*   FindProductName( int productId )
{
   const int numItems = static_cast< int >( sizeof(productNames)/sizeof( productNames[0]) );
   if( productId < 0 || productId >= numItems )
      return NULL;
   return productNames[ productId ];
}

//////////////////////////////////////////////////////////////////////////

const char*   FindPlatformName( int platformId )
{
   const int numItems = static_cast< int >( sizeof(platformStrings)/sizeof( platformStrings[0]) );
   if( platformId < 0 || platformId >= numItems )
      return NULL;
   return platformStrings[ platformId ];
}


//////////////////////////////////////////////////////////////////////////
