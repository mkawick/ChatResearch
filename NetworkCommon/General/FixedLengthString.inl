// FixedLengthString.inl


////////////////////////////////////////////////////////
template < int str_length >
int   FixedLengthString <str_length> :: size() const 
{ 
   return this->length();
}

template < int str_length >
int   FixedLengthString <str_length> :: length() const 
{ 
   if( buffer[0] == 0 )
      return 0; 
   return strlen( buffer ); 
}

template < int str_length >
int   FixedLengthString <str_length>:: find( const char* subString ) const
{
   if( buffer[0] == 0 )
      return maxLength();
   const char* ptr = strstr( buffer, subString );
   if( ptr == NULL )
      return maxLength();
   else
      return ptr - buffer;
}

template < int str_length >
bool  FixedLengthString <str_length>:: operator == ( const char * str ) const
{ return isEqual( str, false ); }

template < int str_length >
bool  FixedLengthString <str_length>:: operator == ( const FixedLengthString& rhs ) const
{
   return this->isEqual( rhs.buffer, false );
}

template < int str_length >
const FixedLengthString <str_length>&		
FixedLengthString <str_length>:: operator = ( const char * str )
{
   copy( str );
   return *this;
}

////////////////////////////////////////////////////////

template < int str_length >
bool  FixedLengthString <str_length> :: SerializeIn( const U8* inputBuffer, int& bufferOffset, int minorVersion )
{
   strncpy( buffer, (const char*)( inputBuffer + bufferOffset ), str_length );
   bufferOffset += str_length;
   buffer[ str_length ] = 0;
   return true;
}

template < int str_length >
bool  FixedLengthString <str_length> :: SerializeOut( U8* outputBuffer, int& bufferOffset, int minorVersion ) const
{
   strncpy( (char*)( outputBuffer + bufferOffset ), buffer, str_length );
   bufferOffset += str_length;
   return true;
}

////////////////////////////////////////////////////////

template < int str_length >
void FixedLengthString <str_length> :: copy( const char* str )
{ 
   if( str == NULL )
   {
      buffer[0] = 0;
      return;
   }
   int len = strlen( str ); 
   if( len > str_length ) 
   {
      len = str_length;
      strncpy( buffer, str, len );
      buffer[ len ] = 0;
   }
   else
   {
      strcpy( buffer, str );
   }
}

template < int str_length >
bool  FixedLengthString <str_length> :: isEqual( const char* str, bool compareCaseless ) const
{ 
   if( str == NULL )
   {
      if( buffer[0] == 0 )
         return true;
      return false;
   }
   int len = strlen( str ); 
   if( len > str_length ) 
   {
      return false;
   }
   if( compareCaseless == false )
   {
      if( strcmp( buffer, str ) == 0 )
         return true;
      return false;
   }
   else
   {
      if( strcasecmp( buffer, str ) == 0 )
         return true;
      return false;
   }
}

////////////////////////////////////////////////////////

template < int str_length >
bool operator==(const string& lhs, const FixedLengthString <str_length>& rhs)
{ 
   return rhs.operator == ( lhs.c_str() );
}

template < int str_length >
bool operator==(const char* lhs, const FixedLengthString <str_length>& rhs)
{ 
   return rhs.operator == ( lhs );
}

template < int str_length >
std::ostream& operator<<(std::ostream& os, const FixedLengthString <str_length>& str )
{
   std::cout << str.c_str();
   return os;
}


////////////////////////////////////////////////////////


////////////////////////////////////////////////////////
template < int str_length >
int   BoundedLengthString <str_length> :: size() const 
{ 
   return this->length();
}

template < int str_length >
int   BoundedLengthString <str_length> :: length() const 
{ 
   if( buffer[0] == 0 )
      return 0; 
   return strlen( buffer ); 
}

template < int str_length >
int   BoundedLengthString <str_length>:: find( const char* subString ) const
{
   if( buffer[0] == 0 )
      return maxLength();
   const char* ptr = strstr( buffer, subString );
   if( ptr == NULL )
      return maxLength();
   else
      return ptr - buffer;
}

template < int str_length >
bool  BoundedLengthString <str_length>:: operator == ( const char * str ) const
{ return isEqual( str, false ); }

template < int str_length >
bool  BoundedLengthString <str_length>:: operator == ( const BoundedLengthString& rhs ) const
{
   return this->isEqual( rhs.buffer, false );
}

template < int str_length >
const BoundedLengthString <str_length>&		
BoundedLengthString <str_length>:: operator = ( const char * str )
{
   copy( str );
   return *this;
}

////////////////////////////////////////////////////////

template < int str_length >
bool  BoundedLengthString <str_length> :: SerializeIn( const U8* inputBuffer, int& bufferOffset, int minorVersion )
{
   U16 size;
   memcpy( &size, (const char*)( inputBuffer + bufferOffset ), sizeof( size ) );
   bufferOffset += sizeof( size );
   m_length = size;
   if( size == 0 )
   {
      buffer[0] = 0;
   }
   else
   {
      //assert( size <= MAX_LENGTH_BOUNDED_STRING );
      if( m_length > str_length )
         m_length = str_length;
      memcpy( buffer, (const char*)( inputBuffer + bufferOffset ), m_length );
      bufferOffset += m_length;
      buffer[ m_length ] = 0;
   }
   return true;
}

template < int str_length >
bool  BoundedLengthString <str_length> :: SerializeOut( U8* outputBuffer, int& bufferOffset, int minorVersion ) const
{
   U16 size = m_length;
   memcpy( (char*)( outputBuffer + bufferOffset ), &size, sizeof( size ) );
   bufferOffset += sizeof( size );

   if( m_length > 0 )
   {
      memcpy( (char*)( outputBuffer + bufferOffset ), buffer, m_length );
      bufferOffset += m_length;
   }
   return true;
}

////////////////////////////////////////////////////////

template < int str_length >
void BoundedLengthString <str_length> :: copy( const char* str )
{ 
   if( str == NULL )
   {
      buffer[0] = 0;
      m_length = 0;
      return;
   }
   m_length = strlen( str ); 
   if( m_length > str_length ) 
   {
      m_length = str_length;
      strncpy( buffer, str, m_length );
   }
   else
   {
      strcpy( buffer, str );
   }
   buffer[ m_length ] = 0;
}

template < int str_length >
bool  BoundedLengthString <str_length> :: isEqual( const char* str, bool compareCaseless ) const
{ 
   if( str == NULL )
   {
      if( buffer[0] == 0 )
         return true;
      return false;
   }
   int len = strlen( str ); 
   if( len > str_length ) 
   {
      return false;
   }
   if( compareCaseless == false )
   {
      if( strcmp( buffer, str ) == 0 )
         return true;
      return false;
   }
   else
   {
      if( strcasecmp( buffer, str ) == 0 )
         return true;
      return false;
   }
}

////////////////////////////////////////////////////////

template < int str_length >
bool operator==(const string& lhs, const BoundedLengthString <str_length>& rhs)
{ 
   return rhs.operator == ( lhs.c_str() );
}

template < int str_length >
bool operator==(const char* lhs, const BoundedLengthString <str_length>& rhs)
{ 
   return rhs.operator == ( lhs );
}

template < int str_length >
std::ostream& operator<<(std::ostream& os, const BoundedLengthString <str_length>& str )
{
   std::cout << str.c_str();
   return os;
}


////////////////////////////////////////////////////////
