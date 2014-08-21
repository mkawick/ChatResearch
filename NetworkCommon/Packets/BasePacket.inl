// BasePacket.inl

#include <assert.h>

template < typename type >
bool  SerializedVector< type >::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   int num = static_cast< int >( m_data.size() );
   Serialize::In( data, bufferOffset, num, minorVersion );
   Serialize::In( data, bufferOffset, listIndex, minorVersion );
   Serialize::In( data, bufferOffset, listCount, minorVersion );

   for( int i=0; i<num; i++ )
   {
      type temp;
      Serialize::In( data, bufferOffset, temp, minorVersion );
      m_data.push_back( temp );
   }

   return true;
}

template < typename type >
bool  SerializedVector< type >::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   int num = static_cast< int >( m_data.size() );
   Serialize::Out( data, bufferOffset, num, minorVersion );
   Serialize::Out( data, bufferOffset, listIndex, minorVersion );
   Serialize::Out( data, bufferOffset, listCount, minorVersion );

   typename std::vector< type > :: const_iterator  it = m_data.begin();
   while( it != m_data.end() )
   {
      Serialize::Out( data, bufferOffset, *it++, minorVersion );
   }
   return true;
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

template < typename type >
bool  KeyValueSerializer< type >::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   Serialize::In( data, bufferOffset, key, minorVersion );
   Serialize::In( data, bufferOffset, value, minorVersion );
   return true;
}

template < typename type >
bool  KeyValueSerializer< type >::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   Serialize::Out( data, bufferOffset, key, minorVersion );
   Serialize::Out( data, bufferOffset, value, minorVersion );
   return true;
}

///////////////////////////////////////////////////////////////

template < typename type >
bool  SerializedKeyValueVector< type > ::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{
   int num = 0;
   Serialize::In( data, bufferOffset, num, minorVersion );
   Serialize::In( data, bufferOffset, listIndex, minorVersion );
   Serialize::In( data, bufferOffset, listCount, minorVersion );

   for( int i=0; i< num; i++ )
   {
      KeyValueSerializer<type> kvs;
      kvs.SerializeIn( data, bufferOffset, minorVersion );
      dataList.push_back( kvs );
   }
   return true;
}

template < typename type >
bool  SerializedKeyValueVector< type > ::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const
{
   int num = static_cast< int >( dataList.size() );
   Serialize::Out( data, bufferOffset, num, minorVersion );
   Serialize::Out( data, bufferOffset, listIndex, minorVersion );
   Serialize::Out( data, bufferOffset, listCount, minorVersion );

   typename KeyValueVector::const_iterator it = dataList.begin();
   while( it != dataList.end() )
   {
      const KeyValueSerializer<type>& kvs = *it++ ;
      kvs.SerializeOut( data, bufferOffset, minorVersion );
   }
   return true;
}

template < typename type >
type  SerializedKeyValueVector< type > ::find( const string& key ) const
{
   typename KeyValueVector::const_iterator it = dataList.begin();
   while( it != dataList.end() )
   {
      const KeyValueSerializer<type>& kvs = *it;
      if( kvs.key == key )
      {
         return kvs.value;
      }
      it++;
   }
   return type();
}

template < typename type >
bool  SerializedKeyValueVector< type > ::erase( const string& key )
{
   typename KeyValueVector::const_iterator it = dataList.begin();
   while( it != dataList.end() )
   {
      const KeyValueSerializer<type>& kvs = *it;
      if( kvs.key == key )
      {
         dataList.erase( it );
         return true;
      }
      it++;
   }
   return false;
}

template < typename type >
void  SerializedKeyValueVector< type > ::reserve( int num )
{
   if( dataList.size() )
      assert( 0 );// disaster
   dataList.reserve( num );
}

template < typename type >
bool SerializedKeyValueVector< type > ::operator = (const KeyValueSerializer< type >& src )
{
   dataList.clear();

   typename KeyValueVector::const_iterator it = src.begin();
   while( it != src.end() )
   {
      dataList.push_back( *it++ );
   }

   return true;
}

template < typename type >
bool SerializedKeyValueVector< type > ::operator = (const vector< KeyValueSerializer< type > > & src )
{
   dataList.clear();

   typename KeyValueVector::const_iterator it = src.begin();
   while( it != src.end() )
   {
      dataList.push_back( *it++ );
   }

   return true;
}

template < typename type >
const KeyValueSerializer<type>&   SerializedKeyValueVector< type > ::operator[] ( const int index )
{
   assert( index >= 0 && index < (int) dataList.size() );

   return dataList[ index ];
}

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////