// ConnectionToUser.cpp

#include "ConnectionToUser.h"


//////////////////////////////////////////////////////////////////////////

ConnectionToUser:: ConnectionToUser( const string& name, const string& pword, const string& key ) : 
                     username( name ), 
                     passwordHash( pword ), 
                     loginKey( key ), 
                     status( LoginStatus_Pending ), 
                     active( true ), 
                     loggedOutTime( 0 ),
                     adminLevel( 0 ),
                     showWinLossRecord( true ),
                     marketingOptOut( false ),
                     showGenderProfile( false )
                     {}


void  ConnectionToUser::AddProductFilterName( const string& text )
{
   bool found = false;
   vector< string >::iterator searchIt = productFilterNames.begin();
   while( searchIt != productFilterNames.end() )
   {
      if( *searchIt == text )
      {
         found = true;
         break;
      }
      searchIt++;
   }
   if( found == false )
   {
      productFilterNames.push_back( text );
   }
}

int   ConnectionToUser::FindProductFilterName( const string& text )
{
   vector< string >::iterator searchIt = productFilterNames.begin();
   while( searchIt != productFilterNames.end() )
   {
      if( *searchIt == text )
      {
         return ( searchIt - productFilterNames.begin() );
      }
      searchIt++;
   }
   return -1;
}
                     
//////////////////////////////////////////////////////////////////////////
