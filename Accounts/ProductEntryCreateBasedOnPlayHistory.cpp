
#include "ProductEntryCreateBasedOnPlayHistory.h"
#include "StatusUpdate.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include <boost/lexical_cast.hpp>
#include <iostream>
using namespace std;

struct UserJoinProduct
{
   UserJoinProduct( U32 id, const string& user_uuid, const string& date, const string& product_uuid ) : user_id( id ), userUuid( user_uuid ), userCreationDate( date ), productUuid( product_uuid ) {}
   U32      user_id;
   string   userUuid;
   string   userCreationDate;
   string   productUuid;
};

ProductEntryCreateBasedOnPlayHistory::ProductEntryCreateBasedOnPlayHistory( U32 id, Queryer* parent ): 
                                       QueryHandler( id, 45, parent ), 
                                       m_hasLoadedAllProducts( false ), 
                                       m_hasRequestedAllProducts( false ), 
                                       m_hasPendingDbResult( false ), 
                                       m_hasCompletedTryingEveryProduct( false ),
                                       m_lastQueryIndexRun( -1 )
{
   /*
   SELECT users.user_id, users.uuid from users_ascension inner join users 
   on users_ascension.user_id=users.user_id 
   where users_ascension.user_id not in 

   (SELECT users.user_id from users 
   inner join user_join_product on 
   users.uuid=user_join_product.user_uuid 
   where user_join_product.product_id=1) LIMIT 5";
   */

   
   
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::Update( time_t currentTime )
{
   QueryHandler::Update( currentTime, m_hasPendingDbResult );
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::SubmitQuery()
{
   if( m_hasLoadedAllProducts == false )
   {
      if( m_hasRequestedAllProducts == false )
      {
         RequestProducts();
         m_hasRequestedAllProducts = true;
      }
      return;
   }

   if( m_listOfQueries.size() < 1 )// no queries
      return;

   if( m_lastQueryIndexRun < 0 || m_lastQueryIndexRun >= (int)m_listOfQueries.size() )
   {
      if( m_lastQueryIndexRun >= (int)m_listOfQueries.size() )
      {
         m_hasCompletedTryingEveryProduct = true;
         SetRunSlower( true );
      }
      m_lastQueryIndexRun = 0;
   }

   do{// find a valid one
      if( m_listOfQueries[m_lastQueryIndexRun].productUuid == "" )
      {
         m_lastQueryIndexRun++;
      }
      else
      {
         break;
      }
   }
   while( m_lastQueryIndexRun < (int)m_listOfQueries.size() );

   if( m_lastQueryIndexRun >= (int)m_listOfQueries.size() )
   {
      cout << "ProductEntryCreateBasedOnPlayHistory:: m_lastQueryIndexRun too large for the array it's indexing" << endl;
      assert( m_lastQueryIndexRun < (int)m_listOfQueries.size() );
   }

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = m_queryType;
   dbQuery->meta = boost::lexical_cast< string >( m_lastQueryIndexRun );

   dbQuery->query = m_listOfQueries[m_lastQueryIndexRun].queryToRun;

   m_parent->AddQueryToOutput( dbQuery );

   cout << "ProductEntryCreateBasedOnPlayHistory: looking for users who have played but have no purchase record" << endl;
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::RequestProducts()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = StatusUpdate::QueryType_LoadProductIds;
   dbQuery->query = "SELECT product_id, uuid FROM product";

   m_parent->AddQueryToOutput( dbQuery );
   m_hasPendingDbResult = true;
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory:: SetupQueryForUserBoughtProductBasedOnFirstDatePlayed( U32 user_id, const string& userUuid, const string& userCreationDate, const string& productUuid )
{
   QueryPerProduct qpg;
   bool result = FindQueryPerProduct( productUuid, qpg );
   if( result == false )
   {
      cout << "ProductEntryCreateBasedOnPlayHistory:: could not find product... FindQueryPerProduct " << endl;
      assert( 0 );
   }

   string productName = FindProductName( qpg.productId );
   string playersTable = "players_";
   playersTable += productName;
   string gamesTable = "games_";
   gamesTable += productName;

   string query = "SELECT users.uuid, min(creation_time) FROM ";
   query += playersTable;
   query += " JOIN ";
   query += gamesTable;
   query += " ON ";
   query += playersTable;
   query += ".game_id=";
   query += gamesTable;
   query += ".game_id JOIN users ON users.user_id=";
   query += playersTable;
   query += ".user_id WHERE users.user_id=";
   query += boost::lexical_cast< string >( user_id );
   query += " GROUP BY users.user_id";


   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = StatusUpdate::QueryType_FindEarliestPlayDateForProduct;
   dbQuery->meta = productUuid;
   dbQuery->query = query;
   dbQuery->customData = new UserJoinProduct( user_id, userUuid, userCreationDate, productUuid );

   m_parent->AddQueryToOutput( dbQuery );
   m_hasPendingDbResult = true;
}


//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::AddQueryPerProduct( int productId, const string& productUuid )
{
   if( productId >= GameProductId_ASCENSION &&
      productId < GameProductId_NUM_GAMES )
   {
      if( productUuid.size() == 0 )
      {
         cout << "product ID is invalid == \"\"" << endl;
         assert( 0 );
      }

      if( productId == GameProductId_DOMINION || // to support a sparse array, we need to eliminate games that we do not support.
          productId == GameProductId_THUNDERSTONE ||
          productId == GameProductId_WOWCMG || 
          productId == GameProductId_INFINITECITY ||
          productId == GameProductId_SMASHUP ||
          productId == GameProductId_MONKEYS_FROM_MARS )
             return;

      string desiredTable = "users_";
      desiredTable += FindProductName( productId );

      string query = "SELECT users.user_id, users.uuid, users.user_creation_date FROM ";
      query += desiredTable;
      query += " INNER JOIN users ON ";
      query += desiredTable;
      query += ".user_id=users.user_id";
      query += " WHERE users.user_id NOT IN (SELECT users.user_id FROM users INNER JOIN user_join_product ON users.uuid=user_join_product.user_uuid";
      query += " WHERE user_join_product.product_id='";
      query += productUuid;
      query += "') LIMIT 5"; // never more than 5 updates at a time.

      // for test only
      //cout << "Product query:" << query << endl;
      QueryPerProduct qpg;
      qpg.productId = productId;
      qpg.queryToRun = query;
      qpg.userTableName = desiredTable;
      qpg.productUuid = productUuid;
      m_listOfQueries.push_back( qpg );
   }
}

//---------------------------------------------------------------

bool     ProductEntryCreateBasedOnPlayHistory::HandleResult( const PacketDbQueryResult* dbResult )
{
   U32 queryType = dbResult->lookup;
   if( queryType != m_queryType &&
      queryType != StatusUpdate::QueryType_LoadProductIds &&
      queryType != StatusUpdate::QueryType_FindEarliestPlayDateForProduct )
   {
      return false;
   }

   SetValueOnExit< bool >           setter( m_hasPendingDbResult, false );// due to multiple exit points...

   if( queryType == m_queryType )
   {
      int whichProductIndex = boost::lexical_cast< int >( dbResult->meta );
      if( whichProductIndex >= (int) m_listOfQueries.size() )
      {
         cout << " ProductEntryCreateBasedOnPlayHistory:: Query result.. whichProductIndex is too large for array it indexes" << endl;
         assert( 0 );
      }
      const QueryPerProduct& qpg = m_listOfQueries[whichProductIndex];

      User_IdUUidDateParser    enigma( dbResult->bucket );
      User_IdUUidDateParser::iterator    it = enigma.begin();
      if( enigma.m_bucket.size() > 0 )
      {
         cout << " Successful query: found users who have played " << qpg.productUuid << " and do not have purchase records" << endl;

         while (it != enigma.end() )
         {
            User_IdUUidDateParser::row      row = *it++;

            
            

            string productUuid = qpg.productUuid;

            U32      userId =    boost::lexical_cast<int>( row[ TableUser_IdUUidDate::Column_id ] );
            string   userUuid =      row[ TableUser_IdUUidDate::Column_uuid ];
            string   date =          row[ TableUser_IdUUidDate::Column_date ];

            SetupQueryForUserBoughtProductBasedOnFirstDatePlayed( userId, userUuid, date, productUuid );
         }
      }
      else
      {
         cout << " Failed query: no users who have played " << qpg.productUuid << " and do not have purchase records" << endl;
         m_lastQueryIndexRun ++;// rotate to the next game title.
      }

      time( &m_lastTimeStamp );// restart timer
      return true;
   }

   if( queryType == StatusUpdate::QueryType_LoadProductIds )
   {
      KeyValueParser                enigma( dbResult->bucket );
      KeyValueParser::iterator      it = enigma.begin();
      while( it != enigma.end() )
      {
         KeyValueParser::row        row = *it++;
         int product_id =  boost::lexical_cast< int >( row[ TableKeyValue::Column_key ] );
         string uuid =              row[ TableKeyValue::Column_value ];

         AddQueryPerProduct( product_id, uuid );
      }

      m_hasLoadedAllProducts = true;
      time( &m_lastTimeStamp );// restart timer
      return true;
   }

   if( queryType == StatusUpdate::QueryType_FindEarliestPlayDateForProduct )
   {
      // a decision here allows us to look for the first instance of that product being played as our purchase date. If not, we use the user creation date.
      KeyValueParser                enigma( dbResult->bucket );
      KeyValueParser::iterator      it = enigma.begin();
      if( enigma.m_bucket.size() > 0 )
      {
         KeyValueParser::row        row = *it;
         string userUuid =                   row[ TableKeyValue::Column_key ];
         string earliestDate =               row[ TableKeyValue::Column_value ];
         string productUuid =                dbResult->meta;

         AddGenericProductEntry( productUuid, userUuid, earliestDate );
         delete static_cast<UserJoinProduct*> ( dbResult->customData );
      }
      else
      {
         UserJoinProduct* productInfo = static_cast<UserJoinProduct*> ( dbResult->customData );
         AddGenericProductEntry( productInfo->productUuid, productInfo->userUuid, productInfo->userCreationDate );
         delete productInfo;
      }
   }

   return false;
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::AddGenericProductEntry( const string& productUuid, const string& userUuid, const string& date )
{
   if( productUuid.size()==0 || userUuid.size()==0 || date.size()==0 )
   {
      cout << "ProductEntryCreateBasedOnPlayHistory::params to AddGenericProductEntry are invalid" << endl;
   }
   assert( productUuid.size() > 0 );
   assert( userUuid.size() > 0 );
   assert( date.size() > 0 );

   string query = "INSERT INTO user_join_product values( DEFAULT,'";
   query += userUuid;
   query += "','";
   query += productUuid;         
   query += "','";
   query += date;
   query += "',0,1,1,0, 'auto create from login history', null)";

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = StatusUpdate::QueryType_ProductEntryCreateBasedOnPlayHistory;
   dbQuery->query = query;
   dbQuery->isFireAndForget = true;

   m_parent->AddQueryToOutput( dbQuery );

   cout << "Adding product entry for user:" << query << endl;
}

//---------------------------------------------------------------

bool     ProductEntryCreateBasedOnPlayHistory::FindQueryPerProduct( int productId, QueryPerProduct& qpg )
{
   vector <QueryPerProduct>::iterator lookupIt = m_listOfQueries.begin();
   while( lookupIt != m_listOfQueries.end() )
   {
      if( lookupIt->productId == productId )
      {
         qpg = *lookupIt;
         return true;
      }
      lookupIt ++;
   }
   return false;
}

//---------------------------------------------------------------

bool     ProductEntryCreateBasedOnPlayHistory::FindQueryPerProduct( const string& productUuid, QueryPerProduct& qpg )
{
   vector <QueryPerProduct>::iterator lookupIt = m_listOfQueries.begin();
   while( lookupIt != m_listOfQueries.end() )
   {
      if( lookupIt->productUuid == productUuid )
      {
         qpg = *lookupIt;
         return true;
      }
      lookupIt ++;
   }
   return false;
}

//---------------------------------------------------------------

//---------------------------------------------------------------
