#pragma once
// Enigmasaurus.h

#include "../Packets/BasePacket.h"

////////////////////////////////////////////////////////////////////////
// this is for reference only. You will need to create your own.
class TabelInterfaceStub 
{
public:
   enum Columns
   {
      Column_none
   };
};

////////////////////////////////////////////////////////////////////////

template< typename TabelInterface >
class Enigmosaurus
{
private:
   typedef typename TabelInterface  tabletype;
   typedef vector< string >         DataRow;
   typedef list< DataRow >          DataSet;

public:
   typedef DataSet::const_iterator  iterator;
   typedef const DataRow&           row;

public:
   Enigmosaurus( const DynamicDataBucket& bucket ) : m_bucket( bucket.bucket ) {}
   ~Enigmosaurus() {}

   iterator    begin() { return m_bucket.begin(); }
   iterator    end() { return m_bucket.end(); }
   iterator    operator[] ( enum TabelInterface::Columns a ) { return m_bucket.begin(); }
   
public:
   const DynamicDataBucket::DataSet& m_bucket;
};

////////////////////////////////////////////////////////////////////////

/*
// use case.....

   class TableUser
   {
   public:
      enum Columns
      {
         Column_id,
         Column_name,
         Column_uuid,
         Column_birth_date,
         Column_last_login_time,
         Column_last_logout_time,
         Column_end
      };
      static const char* const column_names[];
   };

   ...

   Enigmosaurus <TableUser> enigma2( bucket );
   Enigmosaurus <TableUser>::iterator it2 = enigma2.begin();
   while( it2 != enigma2.end() )
   {
      Enigmosaurus <TableUser>::row row = *it2++;
      cout << "username: " << row[ TableUser::Column_name ] << endl;
   }
   Enigmosaurus <TableUser>::iterator var = enigma2[ TableUser::Column_name ];
*/

//*********************************************************
//#include "Enigmosaurus.inl"
//*********************************************************
