// ProductEntryCreateBasedOnPlayHistory.h

#pragma once

#include "../NetworkCommon/Database/QueryHandler.h"

struct QueryPerProduct
{
   int      productId;
   string   userTableName;
   string   gameTableName;
   string   playerTableName;
   string   queryToRun;
   string   productUuid;
};


struct  UserWhoMayNeedUpdate
{
   U32      id;
   string   uuid;
   string   firstKnownPlayDate;
   string   productUuid;
   int      productId;
   bool     awaitingQueryResult;

   UserWhoMayNeedUpdate() : id( -1 ), productId( -1 ), awaitingQueryResult( false ) {}
};

class ProductEntryCreateBasedOnPlayHistory : public QueryHandler
{

public:
   //BlankUserProfileHandler( U32 id, Queryer* parent, string& query );
   ProductEntryCreateBasedOnPlayHistory( U32 id, Queryer* parent );

   void     SetProductIdStart( int productId );
   void     SetUserIdStart( int userId ) { m_currentUserIndex = userId; }

   bool     HandleResult( const PacketDbQueryResult* dbResult );
   void     Update( time_t currentTime );

private:
   ProductEntryCreateBasedOnPlayHistory();

   void     CreatePurchaseRecord( string user_uuid, string product_uuid, string approximate_data_of_purchase );
   void     UpdateProductIndex();
   void     SubmitQuery();
   void     RequestProducts();
   void     WriteAdminValuesToDb();

   bool     StoreUsersWhoMayNeedAnUpdate( const PacketDbQueryResult* dbResult );
   void     RunUserQueries();
   void     SubmitRequestForNextUser();
   void     ErasePendingUserLookup( const string& userUuid );
   const UserWhoMayNeedUpdate*    FindPendingUser( const string& userUuid );

   void     AddQueryPerProduct( int productId, const string& productUuid );
   void     AddGenericProductEntry( const string& gameUuid, const string& userUuid, const string& date );

   bool     FindQueryPerProduct( int gameId, QueryPerProduct& qpg );
   bool     FindQueryPerProduct( const string& productUuid, QueryPerProduct& qpg );

   bool     m_hasLoadedAllProducts;
   bool     m_hasRequestedAllProducts;
   bool     m_hasPendingDbResult;
   bool     m_hasCompletedTryingEveryProduct;
   int      m_currentProductIndex;
   int      m_currentUserIndex;
   int      m_startingProductId;

   vector <QueryPerProduct> m_listOfQueries;
   list <UserWhoMayNeedUpdate> m_listOfUsersQueryingUpdate;
   list <UserWhoMayNeedUpdate> m_listOfUsersAwaitingQueryResult;
};
