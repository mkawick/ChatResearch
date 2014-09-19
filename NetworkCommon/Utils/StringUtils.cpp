#include "StringUtils.h"

using namespace std;

string BufferToHexString( const U8* buffer, int numBytes )
{
   string str;
   str.reserve( 3*numBytes + 2 );
   for( int j=0; j< numBytes; j++ )
   {
      str += ToHexString( *buffer ) + " ";
      buffer ++;
   }
   return str;
}