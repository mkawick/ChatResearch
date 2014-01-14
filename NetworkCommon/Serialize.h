// Serialize.h

#pragma once

#include "DataTypes.h"
#include "ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#include <Winsock2.h>

U64 htonll(U64 value); // used for endianness
U64 ntohll(U64 value);

// warning about loss of data for UINT 64 which does not apply
#pragma warning( push )
#pragma warning( disable : 4244 )

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#if defined ANDROID
#include <fcntl.h>
#else
#include <sys/fcntl.h>
#endif
#endif

#include <string>
using namespace std;


////////////////////////////////////////////////////////

namespace Serialize
{
   // Serialization 
   template<typename T>
   struct IsSelfSerializableIn
   {
	   template< typename U, bool (U::*)( const U8*, int&) > struct signature;
	   template< typename U > static char& HasSerialize(signature< U, &U::SerializeIn >*);
      template< typename U > static int& HasSerialize(...);
	   static const bool value = sizeof(char) == sizeof(HasSerialize<T>(0));
   };

   ////////////////////////////////////////////////////////////////
   //                       primitive serialization
   ////////////////////////////////////////////////////////////////
   template< typename T, bool hasSerializer = IsSelfSerializableIn<T>::value >
   struct Serializer
   {
	   static inline void In( const U8* source, int& offset, T& value)
	   {
         int size = sizeof( T );
   #if size == 1
         {
            value = *source;
         }
   #else
      memcpy( &value, source + offset, size );
      #if size == 2 
         {
            value = static_cast< T > ( ntohs( value ) );// using a cast to work around compile error
         }
      #elif size == 4 
         {
            value = static_cast< T > ( ntohl( value ) );
         }
      #elif size == 8 
         {
            value = static_cast< T > ( ntohll( value ) );
         }
      #endif
   #endif
         offset += size;
	   }
      //----------------------------------
      static inline void Out( U8* destination, int& offset, const T& value )
	   {
         int size = sizeof( value );
   #if size == 1
         {
            *destination = &temp;
         }
   #elif size == 2 
         {
            temp = static_cast< T > ( htons( value ) );
         }
   #elif size == 4 
         {
            temp = static_cast< T > ( htonl( value ) );
         }
   #elif size == 8 
         {
            temp = static_cast< T > ( htonll( value ) );
         }
   #else
         memcpy( destination + offset, &value, size );
   #endif
         offset += size;
	   }
   };

   ////////////////////////////////////////////////////////////////
   //                       class serialization
   ////////////////////////////////////////////////////////////////
   template< typename T >
   struct Serializer< T, true >
   {
	   static inline void In( const U8* destination, int& offset, T& value )
	   {
		   value.SerializeIn( destination, offset );
	   }
      static inline void Out( U8* destination, int& offset, const T& value )
	   {
		   value.SerializeOut( destination, offset );
	   }
   };
   ////////////////////////////////////////////////////////////////
   //                       bool serialization
   ////////////////////////////////////////////////////////////////
   template<>
   struct Serializer< bool, false >
   {
	   static inline void In( const U8* source, int& offset, bool& outValue )
	   {
		   U8 tempValue;
         int size = sizeof( tempValue );
         memcpy( &tempValue, source + offset, size );
         outValue = tempValue ? true:false;
         offset += size;
	   }
      static inline void Out( U8* destination, int& offset, const bool& value )
	   {
		   U8 finalValue = value ? 1:0;
         int size = sizeof( finalValue );

         memcpy( destination + offset, &finalValue, size );
         offset += size;
	   }
   };
   ////////////////////////////////////////////////////////////////
   //                       string serialization
   ////////////////////////////////////////////////////////////////
   template<>
   struct Serializer< std::string, false >
   {
	   static inline void In( const U8* source, int& offset, std::string& value )
	   {
		   U16 len = 0;// there should be a maximum size to strings
         int size = sizeof( len );
         memcpy( &len, source + offset, size );
         offset += size;

         if( len == 0 )
         {
            value.clear();
            return;
         }
         char buffer[ MaxBufferSize ];
         memcpy( buffer, source + offset, len );
         buffer[ len ] = 0;
         value = buffer;

         offset += len;
	   }
      static inline void Out( U8* destination, int& offset, const std::string& value )
	   {
		   U16 len = static_cast< U16 >( value.size() );// there should be a maximum size to strings
         int size = sizeof( len );
         memcpy( destination + offset, &len, size );// precede with size info
         offset += size;

         if( len == 0 )
            return;

         memcpy( destination + offset, value.c_str(), len );// now the actual string
         offset += len;
	   }
   };

   ////////////////////////////////////////////////////////////////
   //          global entry points for serialization
   ////////////////////////////////////////////////////////////////
   template<typename T>
   inline void In( const U8* source, int& offset, T& value )
   {
	   Serializer<T>::In( source, offset, value );
   }
   template<typename T>
   inline void Out( U8* dest, int& offset, const T& value )
   {
	   Serializer<T>::Out( dest, offset, value );
   }

};

////////////////////////////////////////////////////////

#if PLATFORM == PLATFORM_WINDOWS 
#pragma warning( pop )// warning about loss of data for UINT 64 which does not apply
#endif