// BasePacket.inl

#include <assert.h>

template < typename type >
bool  SerializedVector< type >::SerializeIn( const U8* data, int& bufferOffset )
{
   int num = static_cast< int >( m_data.size() );
   Serialize::In( data, bufferOffset, num );
   Serialize::In( data, bufferOffset, listIndex );
   Serialize::In( data, bufferOffset, listCount );

   for( int i=0; i<num; i++ )
   {
      type temp;
      Serialize::In( data, bufferOffset, temp );
      m_data.push_back( temp );
   }

   return true;
}

template < typename type >
bool  SerializedVector< type >::SerializeOut( U8* data, int& bufferOffset ) const
{
   int num = static_cast< int >( m_data.size() );
   Serialize::Out( data, bufferOffset, num );
   Serialize::Out( data, bufferOffset, listIndex );
   Serialize::Out( data, bufferOffset, listCount );

   typename std::vector< type > :: const_iterator  it = m_data.begin();
   while( it != m_data.end() )
   {
      Serialize::Out( data, bufferOffset, *it++ );
   }
   return true;
}


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

template < typename type >
bool  KeyValueSerializer< type >::SerializeIn( const U8* data, int& bufferOffset )
{
   Serialize::In( data, bufferOffset, key );
   Serialize::In( data, bufferOffset, value );
   return true;
}

template < typename type >
bool  KeyValueSerializer< type >::SerializeOut( U8* data, int& bufferOffset ) const
{
   Serialize::Out( data, bufferOffset, key );
   Serialize::Out( data, bufferOffset, value );
   return true;
}

///////////////////////////////////////////////////////////////

template < typename type >
bool  SerializedKeyValueVector< type > ::SerializeIn( const U8* data, int& bufferOffset )
{
   int num = 0;
   Serialize::In( data, bufferOffset, num );
   Serialize::In( data, bufferOffset, listIndex );
   Serialize::In( data, bufferOffset, listCount );

   for( int i=0; i< num; i++ )
   {
      KeyValueSerializer<type> kvs;
      kvs.SerializeIn( data, bufferOffset );
      dataList.push_back( kvs );
   }
   return true;
}

template < typename type >
bool  SerializedKeyValueVector< type > ::SerializeOut( U8* data, int& bufferOffset ) const
{
   int num = static_cast< int >( dataList.size() );
   Serialize::Out( data, bufferOffset, num );
   Serialize::Out( data, bufferOffset, listIndex );
   Serialize::Out( data, bufferOffset, listCount );

   typename KeyValueVector::const_iterator it = dataList.begin();
   while( it != dataList.end() )
   {
      const KeyValueSerializer<type>& kvs = *it++ ;
      kvs.SerializeOut( data, bufferOffset );
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