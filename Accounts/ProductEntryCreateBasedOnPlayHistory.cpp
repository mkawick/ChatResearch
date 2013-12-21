
#include "ProductEntryCreateBasedOnPlayHistory.h"
#include "StatusUpdate.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"

#include <boost/type_traits.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <deque>
using namespace std;

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable: 4996 )
#endif

struct UserJoinProduct
{
   UserJoinProduct( U32 id, const string& user_uuid, const string& date, const string& product_uuid ) : user_id( id ), userUuid( user_uuid ), userCreationDate( date ), productUuid( product_uuid ) {}
   U32      user_id;
   string   userUuid;
   string   userCreationDate;
   string   productUuid;
};

ProductEntryCreateBasedOnPlayHistory::ProductEntryCreateBasedOnPlayHistory( U32 id, Queryer* parent ): 
                                       ParentType( id, 45, parent ), 
                                       m_hasLoadedAllProducts( false ), 
                                       m_hasRequestedAllProducts( false ), 
                                       m_hasPendingDbResult( false ), 
                                       m_hasCompletedTryingEveryProduct( false ),
                                       m_currentProductIndex( -1 ),
                                       m_currentUserIndex( -1 ),
                                       m_startingProductId( -1 ),
                                       m_numRecordsToPullAtATime( 400 )
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

   /*

select player_id, users.uuid, creation_time from players_summonwar inner join 
(
select game_id, creation_time from games_summonwar group by creation_time
) games
on
players_summonwar.game_id=games.game_id

inner join users on player_id=users.user_id

where player_id>1000
order by player_id
limit 100

   */
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::SetProductIdStart( int productId )
{
   m_startingProductId = productId;
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::Update( time_t currentTime )
{
   ParentType::Update( currentTime, m_hasPendingDbResult );
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::UpdateProductIndex()
{
   if( m_currentProductIndex < 0 || m_currentProductIndex >= (int)m_listOfQueries.size() )
   {
      if( m_currentProductIndex >= (int)m_listOfQueries.size() )
      {
         m_hasCompletedTryingEveryProduct = true;
         SetRunSlower( true );
         m_numRecordsToPullAtATime = 100;
      }
      m_currentProductIndex = 0;
   }

   do
   {// find a valid one
      if( m_listOfQueries[ m_currentProductIndex ].productUuid == "" )
      {
         m_currentProductIndex++;
      }
      else
      {
         break;
      }
   }
   while( m_currentProductIndex < (int)m_listOfQueries.size() );

   if( m_currentProductIndex >= (int)m_listOfQueries.size() )
   {
      cout << "ProductEntryCreateBasedOnPlayHistory:: m_currentProductIndex too large for the array it's indexing" << endl;
      assert( m_currentProductIndex < (int)m_listOfQueries.size() );
   }
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

   UpdateProductIndex();

   if( m_listOfUsersQueryingUpdate.size() > 0 || 
      m_listOfUsersAwaitingQueryResult.size() > 0 )
   {
      RunUserQueries();
   }
   else
   {
      SubmitRequestForNextUser();
   }
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::SubmitRequestForNextUser()
{
   string numBuffer = itos( m_currentUserIndex );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =        0;
   dbQuery->lookup =    m_queryType;
   dbQuery->meta =      boost::lexical_cast< string >( m_currentProductIndex );
   dbQuery->query =     m_listOfQueries[m_currentProductIndex].queryToRun;
   boost::replace_first( dbQuery->query, "%s", numBuffer );

   m_parent->AddQueryToOutput( dbQuery );

   //cout << "ProductEntryCreateBasedOnPlayHistory: looking for users who have played but have no purchase record" << endl;
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::WriteAdminValuesToDb()
{
   int productId = m_listOfQueries[ m_currentProductIndex ].productId;
   string numBuffer = itos( productId );

   string query = "UPDATE playdek.admin_account SET admin_account.value='%s' WHERE admin_account.key='play_history_product_id'";
   boost::replace_first( query, "%s", numBuffer );

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id =     0;
   dbQuery->lookup = StatusUpdate::QueryType_AccountAdminSettings;
   
   dbQuery->query =  query;
   dbQuery->isFireAndForget = true;

   m_parent->AddQueryToOutput( dbQuery );

   // user
   numBuffer = itos( m_currentUserIndex );
   query = "UPDATE playdek.admin_account SET admin_account.value='%s' WHERE admin_account.key='play_history_user_id'";
   boost::replace_first( query, "%s", numBuffer );

   dbQuery = new PacketDbQuery;
   dbQuery->id =     0;
   dbQuery->lookup = StatusUpdate::QueryType_AccountAdminSettings;
   
   dbQuery->query =  query;
   dbQuery->isFireAndForget = true;  
   m_parent->AddQueryToOutput( dbQuery );

   cout << "Records updated (game index : " << productId << "), (user index: " << m_currentUserIndex << ")" << endl;
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

      assert( productId < GameProductId_NUM_GAMES );

      string usersTable = "users_";
      usersTable += FindProductName( productId );
      string gamesTable = "games_";
      gamesTable += FindProductName( productId );
      string playersTable = "players_";
      playersTable += FindProductName( productId );

      string query = "SELECT player_id, users.uuid, creation_time FROM ";
      query += playersTable;
      query += " INNER JOIN (SELECT game_id, creation_time FROM ";
      query += gamesTable;
      query += " GROUP BY creation_time) games ON ";
      query += playersTable;
      query += ".game_id=games.game_id INNER JOIN users ON ";
      query += playersTable;
      query += ".user_id=users.user_id WHERE player_id>%s ORDER BY player_id LIMIT ";//400"; //<< note the %s userd to help with user indexing
      query += boost::lexical_cast< string >( m_numRecordsToPullAtATime );
      

      // for test only
      //cout << "Product query:" << query << endl;
      QueryPerProduct qpg;
      qpg.productId = productId;
      qpg.queryToRun = query;
      qpg.userTableName = usersTable;
      qpg.gameTableName = gamesTable;
      qpg.playerTableName = playersTable;
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
      queryType != StatusUpdate::QueryType_DoesUserHaveProductPurchase )
   {
      return false;
   }

   SetValueOnExit< bool >           setter( m_hasPendingDbResult, false );// due to multiple exit points...

   if( queryType == m_queryType )
   {
      StoreUsersWhoMayNeedAnUpdate( dbResult );

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

      if( m_startingProductId != -1 )// lookup this product
      {
         int index = 0;
         vector <QueryPerProduct>::iterator lookupIt = m_listOfQueries.begin();
         while( lookupIt != m_listOfQueries.end() )
         {
            if( lookupIt->productId == m_startingProductId )
            {
               m_currentProductIndex = index;
               break;
            }
            lookupIt++;
            index++;
         }
      }

      m_hasLoadedAllProducts = true;
      
      time( &m_lastTimeStamp );// restart timer
      return true;
   }

   if( queryType == StatusUpdate::QueryType_DoesUserHaveProductPurchase )
   {
      string            userUuid = dbResult->meta;
      UserJoinProduct*  productInfo = static_cast<UserJoinProduct*> ( dbResult->customData );

      SimpleUserTable             enigma( dbResult->bucket );
      SimpleUserTable::iterator      it = enigma.begin();
      if( enigma.m_bucket.size() == 0 ) 
      {
         AddGenericProductEntry( productInfo->productUuid, productInfo->userUuid, productInfo->userCreationDate );
      }

      ErasePendingUserLookup( userUuid );
      delete productInfo;

      return true;
   }

   return false;
}

//---------------------------------------------------------------


bool     ProductEntryCreateBasedOnPlayHistory::StoreUsersWhoMayNeedAnUpdate( const PacketDbQueryResult* dbResult )
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
      int highestId = 0;
      while (it != enigma.end() )
      {
         User_IdUUidDateParser::row      row = *it++;

         string productUuid = qpg.productUuid;
         UserWhoMayNeedUpdate  user;

         U32 userId = boost::lexical_cast<U32>( row[ TableUser_IdUUidDate::Column_id ] );
         user.id =                  userId;
         user.uuid =                row[ TableUser_IdUUidDate::Column_uuid ];
         user.firstKnownPlayDate =  row[ TableUser_IdUUidDate::Column_date ];
         user.productId =           whichProductIndex;
         user.productUuid =         productUuid;

         m_listOfUsersQueryingUpdate.push_back( user );

         highestId = userId;
      }

      int numUsersFound = enigma.m_bucket.size();
      numUsersFound = numUsersFound;// compiler warning.
      //cout << " Successful query: found users " << numUsersFound << " who have played " << FindProductName( qpg.productId ) << endl;
      

      m_currentUserIndex = highestId;
      WriteAdminValuesToDb();
   }
   else
   {
      cout << " Failed query: no users who have played " << qpg.productUuid << endl;
      m_currentProductIndex ++;// rotate to the next game title.
      if( m_currentProductIndex < 0 || m_currentProductIndex >= (int)m_listOfQueries.size() )
      {
         if( m_currentProductIndex >= (int)m_listOfQueries.size() )
         {
            m_hasCompletedTryingEveryProduct = true;
            SetRunSlower( true );
         }
         m_currentProductIndex = 0;
      }
      m_currentUserIndex = -1;
      WriteAdminValuesToDb();
   }

   return true;
}

//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::RunUserQueries()
{
   // verify that no user is waiting... if so, simply return.
   if( m_listOfUsersAwaitingQueryResult.size() )
      return;

   // you may be asking why the heck I do this.
   // submitting a query can generate immediate results, and with debugging and the DB thread running
   // I would often get a result before I had removed a previous item from m_listOfUsersQueryingUpdate
   deque< PacketDbQuery* > queriesToSubmit;

   int count = m_numRecordsToPullAtATime / 10;// never send more than 25 at a time.
   if( count > 25 )
      count = 25;
   if( count < 5 )
      count = 5;
   list <UserWhoMayNeedUpdate>:: iterator it = m_listOfUsersQueryingUpdate.begin();
   while( it != m_listOfUsersQueryingUpdate.end() )
   {
      list <UserWhoMayNeedUpdate>::iterator workingIt = it++;
      UserWhoMayNeedUpdate& user = *workingIt;

      m_listOfUsersAwaitingQueryResult.push_back( user );

         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = StatusUpdate::QueryType_DoesUserHaveProductPurchase;
         dbQuery->meta = user.uuid;
         dbQuery->query = "SELECT id FROM user_join_product WHERE user_uuid = '";
         dbQuery->query += user.uuid;
         dbQuery->query += "' AND product_id='";
         dbQuery->query += user.productUuid;
         dbQuery->query += "'";
         dbQuery->customData = new UserJoinProduct( user.id, user.uuid, user.firstKnownPlayDate, user.productUuid );
         queriesToSubmit.push_back( dbQuery );

      m_listOfUsersQueryingUpdate.erase( workingIt );
      count --;
      if( count <= 0 )
         break;
   }

   
   while( queriesToSubmit.size() )
   {
      PacketDbQuery* query = queriesToSubmit.front();
      queriesToSubmit.pop_front();

      m_parent->AddQueryToOutput( query );
   }
   
}
//---------------------------------------------------------------

void     ProductEntryCreateBasedOnPlayHistory::ErasePendingUserLookup( const string& userUuid )
{
   list <UserWhoMayNeedUpdate>:: iterator it = m_listOfUsersAwaitingQueryResult.begin();
   while( it != m_listOfUsersAwaitingQueryResult.end() )
   {
      UserWhoMayNeedUpdate& user = *it;
      if( user.uuid == userUuid )
      {
         m_listOfUsersAwaitingQueryResult.erase( it );
         return;
      }
      it ++;
   }
}

//---------------------------------------------------------------

const UserWhoMayNeedUpdate*     ProductEntryCreateBasedOnPlayHistory::FindPendingUser( const string& userUuid )
{
   list <UserWhoMayNeedUpdate>:: iterator it = m_listOfUsersAwaitingQueryResult.begin();
   while( it != m_listOfUsersAwaitingQueryResult.end() )
   {
      UserWhoMayNeedUpdate& user = *it;
      if( user.uuid == userUuid )
      {
         return &user;
      }
      it ++;
   }
   return NULL;
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
   query += "',0,1,1,0, 'auto create from login history', null, 0)";

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = StatusUpdate::QueryType_ProductEntryCreateBasedOnPlayHistory;
   dbQuery->query = query;
   dbQuery->isFireAndForget = true;

   m_parent->AddQueryToOutput( dbQuery );

   //-----------------

   const UserWhoMayNeedUpdate* user = FindPendingUser( userUuid );
   cout << "Adding product entry for user: ";
   if( user )
   {
      cout << user->id << " product:" << productUuid << " date:" << date << endl;
   }
   else
   {
      cout << query << endl;
   }
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
