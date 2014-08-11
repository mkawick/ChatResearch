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
