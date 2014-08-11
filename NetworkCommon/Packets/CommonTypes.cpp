#include "CommonTypes.h"

///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

bool  StringBucket::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   int numStrings = 0;
   Serialize::In( data, bufferOffset, numStrings, minorVersion );

   for( int i=0; i<numStrings; i++ )
   {
      string temp;
      Serialize::In( data, bufferOffset, temp, minorVersion );
      bucket.push_back( temp );
   }

   return true; 
}

bool  StringBucket::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   int numStrings = static_cast< int >( bucket.size() );
   Serialize::Out( data, bufferOffset, numStrings, minorVersion );

   
   DataSet::const_iterator it = bucket.begin();
   while( it != bucket.end() )
   {
      const string& value = *it++;

      Serialize::Out( data, bufferOffset, value, minorVersion );
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

bool  DynamicDataBucket::SerializeIn( const U8* data, int& bufferOffset, int minorVersion )
{ 
   int numRows = 0, numColumns = 0;
   Serialize::In( data, bufferOffset, numRows, minorVersion );
   Serialize::In( data, bufferOffset, numColumns, minorVersion );
   bucket.clear();

   for( int i=0; i<numRows; i++ )
   {
      bucket.push_back( DataRow() );
      DataRow& newRow = *bucket.rbegin();
      for (int j=0; j<numColumns; j++ )
      {
         string temp;
         Serialize::In( data, bufferOffset, temp, minorVersion );
         newRow.push_back( temp );
      }
   }

   return true; 
}

bool  DynamicDataBucket::SerializeOut( U8* data, int& bufferOffset, int minorVersion ) const 
{ 
   int numRows = static_cast< int >( bucket.size() );
   Serialize::Out( data, bufferOffset, numRows, minorVersion );

   DataSet::const_iterator it = bucket.begin();
   int numColumns = static_cast< int >( (*it).size() );
   Serialize::Out( data, bufferOffset, numColumns, minorVersion );

   while( it != bucket.end() )
   {
      const DataRow& newRow = *it++;

      DataRow::const_iterator    rowIt = newRow.begin();
      while( rowIt != newRow.end() )
      {
         const string& value = *rowIt;

         Serialize::Out( data, bufferOffset, value, minorVersion );

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

