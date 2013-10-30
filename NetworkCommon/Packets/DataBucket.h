#pragma once

#include "../ServerType.h"
#include "../DataTypes.h"
#include "../ServerConstants.h"
#include "../Serialize.h"
#include <string>
#include <vector>
#include <list>

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class StringBucket
{
public:
   typedef list< string >  DataSet;

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   void  operator = ( const list< string >& copyData );
   void  insert( const string& str ) { bucket.push_back( str ); }
   DataSet  bucket;   
};

///////////////////////////////////////////////////////////////

class DataRow : public vector< string >  
{
public:
   const_reference operator[](size_type _Pos) const{
      const_reference ref = vector< string >::operator []( _Pos );
      if( ref == "NULL")
         return empty;
      else 
         return ref;
   }
   reference operator[](size_type _Pos){
      reference ref = vector< string >::operator []( _Pos );
      if( ref == "NULL")
         return empty;
      else 
         return ref;
   };
   string empty;
};


class DynamicDataBucket
{
public:
#ifdef _MEMLEAK_TESTING_
   ~DynamicDataBucket();
#endif
   typedef list< DataRow >  DataSet;

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   void  operator = ( const list< DataRow >& copyData );
   void  operator = ( const list< list<string> >& copyData );
   DataSet  bucket;   
};

///////////////////////////////////////////////////////////////

template < typename type = string >
class KeyValueSerializer
{
public:
   KeyValueSerializer(){}
   KeyValueSerializer( string _key, type _value ): key( _key ), value( _value ){}

   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   string   key;
   type     value;
};

typedef KeyValueSerializer< string >            KeyValueString;
typedef vector< KeyValueString >                KeyValueVector;
typedef KeyValueVector::iterator                KeyValueVectorIterator;
typedef KeyValueVector::const_iterator          KeyValueConstIterator;

///////////////////////////////////////////////////////////////

template < typename type = string >
class SerializedKeyValueVector
{
public:
   typedef  KeyValueSerializer< type >           KeyValue;
   typedef  vector< KeyValue >                   KeyValueVector;
   typedef typename KeyValueVector::iterator             KeyValueVectorIterator;
   typedef typename KeyValueVector::iterator             KVIterator;
   typedef typename KeyValueVector::const_iterator       const_KVIterator;

   SerializedKeyValueVector() {}
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   const KeyValueVector&   GetData() const { return dataList; }
   void  clear() { dataList.clear(); }
   
   // helper functions
   void  insert( const string& key, const type& obj ) { dataList.push_back( KeyValue( key, obj ) ); }

   int                  size() const { return static_cast< int >( dataList.size() ); }
   const_KVIterator     begin() const { return dataList.begin(); }
   KVIterator           begin() { return dataList.begin(); }
   const_KVIterator     end() const { return dataList.end(); }
   KVIterator           end() { return dataList.end(); }

   //KVIterator           rbegin() { return dataList.rbegin(); }
   type&                lastValue() { return (dataList.rbegin()->value); }
   type                 find( const string& key ) const;

   bool operator = (const KeyValueSerializer< type >& src );
   bool operator = (const KeyValueVector& src );

protected:
   KeyValueVector    dataList;
};

///////////////////////////////////////////////////////////////

template < typename type >
class SerializedVector
{
public:
   bool  SerializeIn( const U8* data, int& bufferOffset );
   bool  SerializeOut( U8* data, int& bufferOffset ) const;

   // helper functions
   void           push_back( type value ) { m_data.push_back( value ); }
   int            size() const { return static_cast< int >( m_data.size() ); }
   const type&    operator[]( int index ) { return m_data[ index ]; }
   const type&    operator[]( int index ) const { return m_data[ index ]; }

protected:
   vector< type >    m_data;
};

///////////////////////////////////////////////////////////////