// ProductEntryCreateBasedOnPlayHistory.h

#pragma once

#include "../NetworkCommon/Database/QueryHandler.h"

struct QueryPerProduct
{
   int      productId;
   string   userTableName;
   string   queryToRun;
   string   productUuid;
};

class ProductEntryCreateBasedOnPlayHistory : public QueryHandler
{

public:
   //BlankUserProfileHandler( U32 id, Queryer* parent, string& query );
   ProductEntryCreateBasedOnPlayHistory( U32 id, Queryer* parent );

   bool     HandleResult( const PacketDbQueryResult* dbResult );
   void     Update( time_t currentTime );

private:
   ProductEntryCreateBasedOnPlayHistory();

   void     CreatePurchaseRecord( string user_uuid, string product_uuid, string approximate_data_of_purchase );
   void     SubmitQuery();
   void     RequestProducts();

   void     SetupQueryForUserBoughtProductBasedOnFirstDatePlayed( U32 user_id, const string& userUuid, const string& date, const string& productUuid );

   void     AddQueryPerProduct( int productId, const string& productUuid );
   void     AddGenericProductEntry( const string& gameUuid, const string& userUuid, const string& date );

   bool     FindQueryPerProduct( int gameId, QueryPerProduct& qpg );
   bool     FindQueryPerProduct( const string& productUuid, QueryPerProduct& qpg );

   bool     m_hasLoadedAllProducts;
   bool     m_hasRequestedAllProducts;
   bool     m_hasPendingDbResult;
   bool     m_hasCompletedTryingEveryProduct;
   int      m_lastQueryIndexRun;

   vector <QueryPerProduct> m_listOfQueries;
};
