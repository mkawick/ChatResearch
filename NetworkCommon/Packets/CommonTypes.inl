// FixedLengthString.inl

////////////////////////////////////////////////////////

template< int length >
FixedLengthString <length>::FixedLengthString()
{
   memset( buffer, 0, length+1 );
}

////////////////////////////////////////////////////////

template< int length >
FixedLengthString <length>::FixedLengthString ( const char* text )
{
   int len = strlen( text );
   if( len >= length )
   {
      strncpy( buffer, text, length );
   }
   else
   {
      strcpy( buffer, text );
   }

   buffer[length] = 0;
}

////////////////////////////////////////////////////////

template< int length >
FixedLengthString <length>::FixedLengthString( const FixedLengthString <length>& rhs )
{
   memcpy( buffer, rhs.buffer, length );
   buffer[length] = 0;
}

////////////////////////////////////////////////////////

template< int length >
FixedLengthString <length>& FixedLengthString <length> ::operator = ( const FixedLengthString <length>& rhs )
{
   memcpy( buffer, rhs.buffer, length );
   return *this;
}

template< int length >
FixedLengthString <length>& FixedLengthString <length> ::operator = ( const char* text )
{
   int len = strlen( text );
   if( len >= length )
   {
      strncpy( buffer, text, length );
   }
   else
   {
      strcpy( buffer, text );
   }
   buffer[length] = 0;
   return *this;
}

////////////////////////////////////////////////////////

template< int length >
FixedLengthString <length> :: operator const char* ()
{
   return buffer;
}

////////////////////////////////////////////////////////

template< int length >
bool FixedLengthString <length> ::operator == ( const FixedLengthString <length>& rhs ) const
{
   if( memcmp( buffer, rhs.buffer, length ) == 0 )
      return true;
   return false;
}

template< int length >
bool FixedLengthString <length> ::operator == ( const char* text ) const
{
   int len = strlen( text );
   if( len >= length )
   {
      return false;
   }
   else
   {
      if( strcmp( buffer, text ) == 0 )
         return true;
   }
   return false;
}

////////////////////////////////////////////////////////
