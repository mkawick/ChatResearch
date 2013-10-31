
#include <time.h>
#include <iostream>

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "BlankUserProfileHandler.h"
#include <boost/lexical_cast.hpp>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////

BlankUserProfileHandler::BlankUserProfileHandler( U32 id, Queryer* parent, string& query ) : QueryHandler< Queryer* >( id, 20, parent ), m_isServicingBlankUUID( false )
{
   m_queryString = query;
}

void     BlankUserProfileHandler::Update( time_t currentTime )
{
   QueryHandler< Queryer* >::Update( currentTime, m_isServicingBlankUUID );
}

//---------------------------------------------------------------

bool     BlankUserProfileHandler::HandleResult( const PacketDbQueryResult* dbResult )
{
   if( dbResult->lookup == m_queryType )
   {
      SetValueOnExit< bool >           setter( m_isServicingBlankUUID, false );// due to multiple exit points...

      bool addedProfiles = false;
      IndexTableParser              enigma( dbResult->bucket );
      IndexTableParser::iterator    it = enigma.begin();
    /*  if( enigma.m_bucket.size() > 0 )
      {
         cout << " Successful query = " << m_queryString << endl;
      }*/
      while( it != enigma.end() )
      {
         addedProfiles = true;
         IndexTableParser::row      row = *it++;

         string   userId =            row[ TableIndexOnly::Column_index ];

         cout << "User id adding blank profile = " << userId << endl;

         CreateBlankProfile( userId, 0 );
      }

      time( &m_lastTimeStamp );// restart timer
      return true;
   }
   return false;
}

//---------------------------------------------------------------

void     BlankUserProfileHandler::CreateBlankProfile( const string& user_id, int productId )
{
   if( user_id.size() == 0 || user_id == "0" )
   {
      string message = "Accounts::CreateBlankProfile userId is 0\n";
      cout << message << endl;
      return;
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = m_queryType;
   dbQuery->isFireAndForget = true;

   dbQuery->query = "INSERT INTO user_profile VALUES( '";
   dbQuery->query += user_id;
   dbQuery->query += "', DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, DEFAULT, ";
   if( productId == 0 )
   {
      dbQuery->query += "DEFAULT";
   }
   else
   {
      dbQuery->query += boost::lexical_cast< string >( productId );
   }

   dbQuery->query += ")";

   m_parent->AddQueryToOutput( dbQuery );
}


///////////////////////////////////////////////////////////////////////////////////////////
