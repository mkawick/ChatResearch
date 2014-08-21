#pragma once

#include "../ServerType.h"
#include "../DataTypes.h"
#include "../ServerConstants.h"
#include "../Packets/Serialize.h"
#include <string>
#include <vector>
#include <list>

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

class StringBucket
{
public:
   typedef list< string >  DataSet;

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

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

   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

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
   typedef  KeyValueSerializer< type >                   KeyValue;
   typedef  vector< KeyValue >                           KeyValueVector;
   typedef typename KeyValueVector::iterator             KeyValueVectorIterator;
   typedef typename KeyValueVector::iterator             KVIterator;
   typedef typename KeyValueVector::const_iterator       const_KVIterator;

   SerializedKeyValueVector() { clear(); }
   

   //----------------------------
   const KeyValueVector&   GetData() const { return dataList; }
   void                    clear() { dataList.clear(); listIndex = 0; listCount = 0; }
   bool                    erase( int index );
   bool                    erase( KVIterator iter );

   // helper functions
   void                    insert( const string& key, const type& obj ) { dataList.push_back( KeyValue( key, obj ) ); }

   int                     size() const { return static_cast< int >( dataList.size() ); }
   const_KVIterator        begin() const { return dataList.begin(); }
   KVIterator              begin() { return dataList.begin(); }
   const_KVIterator        end() const { return dataList.end(); }
   KVIterator              end() { return dataList.end(); }

   type&                   lastValue() { return (dataList.rbegin()->value); }
   type                    find( const string& key ) const;
   bool                    erase( const string& key );
   void                    reserve( int num );

   bool              operator = (const KeyValueSerializer< type >& src );
   bool              operator = (const KeyValueVector& src );

   const KeyValueSerializer<type>&  operator[] (const int nIndex);
   
   //------------ tracking variables, mostly for packetization ------------------
   void                    SetIndexParams( U16 firstIndex = 0, U16 totaltCount = 0 ) { listIndex = firstIndex, listCount = totaltCount; }
   U16                     GetFirstIndex() const { return listIndex; }
   U16                     GetTotalCount() const { return listCount; }

protected:
   KeyValueVector    dataList;
   U16   listIndex;// this list will be sent in pieces
   U16   listCount;

public: // I put these down here to stay out of the mental space of the reader. This class is becomming complex
   // so any minor simplifications are helpful
   bool                 SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool                 SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;
};

///////////////////////////////////////////////////////////////

template < typename type >
class SerializedVector
{
public:
   SerializedVector() { clear(); }
   bool  SerializeIn( const U8* data, int& bufferOffset, int minorVersion );
   bool  SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const;

   // helper functions
   void           push_back( type value ) { m_data.push_back( value ); }
   int            size() const { return static_cast< int >( m_data.size() ); }
   void           clear() { m_data.clear(); listIndex = 0, listCount = 0; }
   const type&    operator[]( int index ) { return m_data[ index ]; }
   const type&    operator[]( int index ) const { return m_data[ index ]; }
   bool  remove( U32 index ) { if ( index >= m_data.size() ) return false; m_data.erase( m_data.begin() + index ); return true; }

   //------------ tracking variables, mostly for packetization ------------------
   void                    SetIndexParams( U16 firstIndex = 0, U16 totaltCount = 0 ) { listIndex = firstIndex, listCount = totaltCount; }
   U16                     GetFirstIndex() const { return listIndex; }
   U16                     GetTotalCount() const { return listCount; }
protected:
   vector< type >    m_data;
   U16   listIndex;// this list will be sent in pieces
   U16   listCount;
};

///////////////////////////////////////////////////////////////

template < typename type >
bool  SerializedKeyValueVector<type>::erase( int index ) 
{ 
   if( index < 0 && index >= dataList.size() ) return false; 

   dataList.erase( dataList.begin() + index ); 
   return true; 
}

template < typename type >
bool  SerializedKeyValueVector<type>::erase( KVIterator iter )
{
   dataList.erase( iter ); 
   return true; 
}

////////////////////////////////////////////////////////
/*namespace Serialize
{
   template<>
   inline void In( const U8* source, int& offset, StringBucket& value, int minorVersion )
   {
      value.SerializeIn( source, offset, minorVersion );
   }
   template<>
   inline void Out( U8* dest, int& offset, const StringBucket& value, int minorVersion )
   {
      value.SerializeOut( dest, offset, minorVersion );
   }
   template<>
   inline void In( const U8* source, int& offset, DynamicDataBucket& value, int minorVersion )
   {
      value.SerializeIn( source, offset, minorVersion );
   }
   template<>
   inline void Out( U8* dest, int& offset, const DynamicDataBucket& value, int minorVersion )
   {
      value.SerializeOut( dest, offset, minorVersion );
   }
   template< typename type >
   inline void In( const U8* source, int& offset, KeyValueSerializer<type>& value, int minorVersion )
   {
      value.SerializeIn( source, offset, minorVersion );
   }
   template< typename type >
   inline void Out( U8* dest, int& offset, const KeyValueSerializer<type>& value, int minorVersion )
   {
      value.SerializeOut( dest, offset, minorVersion );
   }
   template< typename type >
   inline void In( const U8* source, int& offset, SerializedKeyValueVector<type>& value, int minorVersion )
   {
      value.SerializeIn( source, offset, minorVersion );
   }
   template< typename type >
   inline void Out( U8* dest, int& offset, const SerializedKeyValueVector<type>& value, int minorVersion )
   {
      value.SerializeOut( dest, offset, minorVersion );
   }
   template< typename type>
   inline void In( const U8* source, int& offset, SerializedVector<type>& value, int minorVersion )
   {
      value.SerializeIn( source, offset, minorVersion );
   }
   template< typename type>
   inline void Out( U8* dest, int& offset, const SerializedVector<type>& value, int minorVersion )
   {
      value.SerializeOut( dest, offset, minorVersion );
   }
}*/
