#include "DataBucket.h"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  StringBucket::SerializeIn( const U8* data, int& bufferOffset )
{ 
   int numStrings = 0;
   Serialize::In( data, bufferOffset, numStrings );

   for( int i=0; i<numStrings; i++ )
   {
      string temp;
      Serialize::In( data, bufferOffset, temp );
      bucket.push_back( temp );
   }

   return true; 
}

bool  StringBucket::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   int numStrings = static_cast< int >( bucket.size() );
   Serialize::Out( data, bufferOffset, numStrings );

   
   DataSet::const_iterator it = bucket.begin();
   while( it != bucket.end() )
   {
      const string& value = *it++;

      Serialize::Out( data, bufferOffset, value );
   }

   return true; 
}

///////////////////////////////////////////////////////////////


#ifdef _MEMLEAK_TESTING_
DynamicDataBucket::~DynamicDataBucket()
{
   bucket.clear();
}
#endif

bool  DynamicDataBucket::SerializeIn( const U8* data, int& bufferOffset )
{ 
   int numRows = 0, numColumns = 0;
   Serialize::In( data, bufferOffset, numRows );
   Serialize::In( data, bufferOffset, numColumns );
   bucket.clear();

   for( int i=0; i<numRows; i++ )
   {
      bucket.push_back( DataRow() );
      DataRow& newRow = *bucket.rbegin();
      for (int j=0; j<numColumns; j++ )
      {
         string temp;
         Serialize::In( data, bufferOffset, temp );
         newRow.push_back( temp );
      }
   }

   return true; 
}

bool  DynamicDataBucket::SerializeOut( U8* data, int& bufferOffset ) const 
{ 
   int numRows = static_cast< int >( bucket.size() );
   Serialize::Out( data, bufferOffset, numRows );

   DataSet::const_iterator it = bucket.begin();
   int numColumns = static_cast< int >( (*it).size() );
   Serialize::Out( data, bufferOffset, numColumns );

   while( it != bucket.end() )
   {
      const DataRow& newRow = *it++;

      DataRow::const_iterator    rowIt = newRow.begin();
      while( rowIt != newRow.end() )
      {
         const string& value = *rowIt;

         Serialize::Out( data, bufferOffset, value );

         ++ rowIt;
      }
   }

   return true; 
}


void  DynamicDataBucket::operator = ( const list< DataRow >& copydata )
{
   bucket.clear(); 
   list< DataRow >::const_iterator    it = copydata.begin();
   while( it != copydata.end() )
   {
      const DataRow& listref = *it++;
      DataRow::const_iterator rowit = listref.begin();

      bucket.push_back( DataRow() );
      DataRow& newrow = *bucket.rbegin();
      while( rowit != listref.end() )
      {
         newrow.push_back( *rowit++ );
      }
   }
}

void  DynamicDataBucket::operator = ( const list< list< string > >& copyData )
{
   bucket.clear(); 
   list< list< string > >::const_iterator    it = copyData.begin();
   while( it != copyData.end() )
   {
      const list< string >& listRef = *it++;
      list< string >::const_iterator rowIt = listRef.begin();

      bucket.push_back( DataRow() );
      DataRow& newRow = *bucket.rbegin();
      while( rowIt != listRef.end() )
      {
         newRow.push_back( *rowIt++ );
      }
   }
}

