// SalesManager.cpp


#include <time.h>
#include <iostream>
#include <string>
#include <set>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "SalesManager.h"

#include "../NetworkCommon/Utils/StringUtils.h"
#include "../NetworkCommon/Database/StringLookup.h"
#include "../NetworkCommon/Packets/PurchasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"

///////////////////////////////////////////////////////////////////////////////////////////

void  ThreadSafeSetWrapper::Add( const string& uuid )
{
   m_mutex.lock();
   m_set.insert( uuid );
   m_mutex.unlock();
}

void  ThreadSafeSetWrapper::Remove( const string& uuid )
{
   m_mutex.lock();
   m_set.erase( uuid );
   m_mutex.unlock();
}

bool  ThreadSafeSetWrapper::Find( const string& uuid )
{
   bool isFound = false;
   m_mutex.lock();
   if( m_set.find( uuid ) != m_set.end() )
   {
      isFound = true;
   }
   m_mutex.unlock();
   return isFound;
}



///////////////////////////////////////////////////////////////////////////////////////////

SalesManager::SalesManager( U32 id, ParentQueryerPtr parent, string& query, bool runQueryImmediately ) : ParentType( id, 20, parent, runQueryImmediately ), 
                              m_isServicingExchangeRates( false ),
                              m_isInitializing( true ),
                              m_hasSendProductRequest( false )
{
   SetQuery( query );
}

void     SalesManager::Update( time_t currentTime )
{
   if( m_isInitializing )
   {
      if( m_hasSendProductRequest == false )
      {
         m_hasSendProductRequest = true;
         RequestAllProducts();
      }
      else
      {
         if( m_lastTimeStamp == 0 &&
               difftime( currentTime, m_lastTimeStamp ) > 15 )/// bad initialization... retry.
         {
            m_lastTimeStamp = currentTime;
            m_hasSendProductRequest = false;
         }
      }
   }
   else
   {
      // request strings first for all of the sales.
      ParentType::Update( currentTime, m_isServicingExchangeRates );
   }
}

//---------------------------------------------------------------

string OpenMultipleInserts()
{
   return string( "INSERT INTO user_join_product VALUES");
};
string CloseMultipleInserts()
{
   return string(";");
}

string   FormatInsertIntoUserJoinProduct( const string& userUuid, const string& productUuid, int count, const string& note, const string& exchangeUuid, int index )
{
   string query;
   if( index >0 )
      query = ",";
   query += "( DEFAULT, '";
   query += userUuid;
   query += "', '";
   query += productUuid;
   query += "',DEFAULT, 0, 0, ";
   query += boost::lexical_cast< string > ( count );    // << notice the negative value here
   query += ", '1', '";
   query += note;
   query += "', '0', '";
   query += exchangeUuid;
   query += "')";
   

   return query;
}

string OpenMultipleInsertsSimple()
{
   return string( "INSERT INTO user_join_product (user_uuid, product_id, num_purchased) VALUES");
};

string   FormatInsertIntoUserJoinProductSimple( const string& userUuid, const string& productUuid, int count, int index = 0 )
{
   string query;
   if( index >0 )
      query = ",";
   query += "('";
   query += userUuid;
   query += "', '";
   query += productUuid;
   query += "',";
   query += boost::lexical_cast< string > ( count );    // << notice the negative value here
   query += ")";
   
   return query;
}

void     SalesManager::UserLoggedOut( const string& uuid )
{
   m_usersBeingServiced.Remove( uuid );
}

//---------------------------------------------------------------

bool     SalesManager::HandleResult( const PacketDbQueryResult* dbResult )
{
   int lookupType = dbResult->lookup;
   if( lookupType == static_cast<int>( m_queryType ) )
   {
      SetValueOnExit< bool >           setter( m_isServicingExchangeRates, false );// due to multiple exit points...

      ExchangeRateParser            enigma( dbResult->bucket );
      ExchangeRateParser::iterator  it = enigma.begin();
      if( enigma.m_bucket.size() > 0 )
      {
         //cout << " Successful query = " << m_queryString );
         exchangeRates.clear();
      }  
      while( it != enigma.end() )
      {
         ExchangeEntry entry;
         entry = *it++;
         exchangeRates.push_back( entry );
      }
      return true;
   }

   if( lookupType == DiplodocusPurchase::QueryType_LoadAllProducts )
   {
      LogMessage( LOG_PRIO_INFO, "product list loaded" );
      ProductTable         enigma( dbResult->bucket );
      ProductTable::iterator  it = enigma.begin();
      while( it != enigma.end() )
      {
         Product p;
         p = *it++;
         m_productMapByUuid.insert( ProductUuidPair( p.uuid, p ) );
      }
      m_isInitializing = false;
      LogMessage( LOG_PRIO_INFO, "Final product list size was: %d", m_productMapByUuid.size() );
      return true;
   }

   if( lookupType == DiplodocusPurchase::QueryType_VerifyThatUserHasEnoughMoney1 )
   {
      PurchaseTracking* purchaseTracking = static_cast< PurchaseTracking* >( dbResult->customData );
      bool doesPlayerHaveEnoughMoney = false;
      ExchangeEntry ee;
      if( FindItem( purchaseTracking->exchangeUuid, ee ) == true )
      {
         IndexTableParser            enigma( dbResult->bucket );
         IndexTableParser::iterator  it = enigma.begin();
         if( enigma.m_bucket.size() > 0 )
         {
            IndexTableParser::row row = *it;

            string count = row[ TableIndexOnly::Column_index ];
            int num = 0;
            if( count != "null" )
            {
               num = boost::lexical_cast< int >( count );
            }

            if( num >= ee.sourceCount )
            {
               doesPlayerHaveEnoughMoney = true;
            }
         }
      }
      else
      {
         m_parent->SendErrorToClient( purchaseTracking->connectionId, purchaseTracking->gatewayId, PacketErrorReport::ErrorType_Purchase_ItemNotAvailable );
      }
      if( doesPlayerHaveEnoughMoney == true )
      {
         PacketDbQuery* dbQuery = new PacketDbQuery;
         dbQuery->id = 0;
         dbQuery->lookup = DiplodocusPurchase::QueryType_PerformPurchase;

         string queryOpen = OpenMultipleInserts();
         string query1 = FormatInsertIntoUserJoinProduct( purchaseTracking->userUuid, ee.sourceUuid, -ee.sourceCount, "user purchase", ee.exchangeUuid, 0 ); // << notice the negative value here
         string query2 = FormatInsertIntoUserJoinProduct( purchaseTracking->userUuid, ee.destUuid, ee.destCount, "user purchase", ee.exchangeUuid, 1 );
         string queryClose = CloseMultipleInserts();
         string query = queryOpen + query1 + query2 + queryClose;

         dbQuery->customData = purchaseTracking; // do not delete this data
         dbQuery->query = query;
         m_parent->AddQueryToOutput( dbQuery );
      }
      else
      {
         m_parent->SendErrorToClient( purchaseTracking->connectionId, purchaseTracking->gatewayId, PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade );
         SendTournamentPurchaseResultBackToServer( purchaseTracking->fromOtherServerId, purchaseTracking->fromOtherServerTransactionId, PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade );
         m_usersBeingServiced.Remove( purchaseTracking->userUuid );
         delete purchaseTracking;
      }
      return true;
   }

   if( lookupType == DiplodocusPurchase::QueryType_PerformPurchase ) // purchase completion... report back to the client that all went well.
   {
      PurchaseTracking* purchaseTracking = static_cast< PurchaseTracking* >( dbResult->customData );
      PacketPurchase_BuyResponse* packet = new PacketPurchase_BuyResponse;
      packet->purchaseUuid = purchaseTracking->exchangeUuid;

      if( dbResult->successfulQuery == true )
      {
         m_parent->SendErrorToClient( purchaseTracking->connectionId, purchaseTracking->gatewayId, PacketErrorReport::ErrorType_Purchase_Success );
         SendTournamentPurchaseResultBackToServer( purchaseTracking->fromOtherServerId, purchaseTracking->fromOtherServerTransactionId, PacketErrorReport::ErrorType_Purchase_Success );
         packet->success = true;
      }
      else
      {
         packet->success = false;
      }

      m_parent->SendPacketToGateway( packet, purchaseTracking->connectionId, purchaseTracking->gatewayId );

      NotifyLoginToReloadUserInventory( purchaseTracking->userUuid, purchaseTracking->connectionId );

      m_usersBeingServiced.Remove( purchaseTracking->userUuid );
      delete purchaseTracking;

      return true;
   }

   if( lookupType == DiplodocusPurchase::QueryType_VerifyThatUserHasEnoughMoney2 ) // purchase completion... report back to the client that all went well.
   {
      PurchaseTracking* purchaseTracking = static_cast< PurchaseTracking* >( dbResult->customData );
   
      VerifyThatUserHasEnoughMoneyForEntry2( dbResult );
      return true;
   }

   if( lookupType ==DiplodocusPurchase:: QueryType_PerformInventoryAddition )
   {
      LogMessage( LOG_PRIO_INFO, "SalesManager handle query result: QueryType_PerformInventoryAddition " );
      LogMessage( LOG_PRIO_INFO, " Successful addition: %s", ConvertToTrueFalseString( dbResult->successfulQuery ) );
      const string& userUuid = dbResult->meta;
      m_usersBeingServiced.Remove( userUuid );
      if( dbResult->successfulQuery )
      {
         UserAccountPurchase* user = NULL;
         if( m_parent->GetUser( userUuid, user ) == true )
         {
            if( user->IsConnected() == true )
            {
               NotifyLoginToReloadUserInventory( userUuid, user->GetFirstConnectedId() );
            }
         }
      }
      return true;
   }

   return false;
}

void     SalesManager::NotifyLoginToReloadUserInventory( const string& userUuid, U32 connectionId )
{
   LogMessage( LOG_PRIO_INFO, "SalesManager::NotifyLoginToReloadUserInventory" );
   LogMessage( LOG_PRIO_INFO, "UserUuid: %s", userUuid.c_str() );
   LogMessage( LOG_PRIO_INFO, "connectionId: %d", connectionId );

   PacketListOfUserPurchasesUpdated* purchasesUpdate = new PacketListOfUserPurchasesUpdated;
   purchasesUpdate->userUuid = userUuid;
   purchasesUpdate->userConnectionId = connectionId;
   m_parent->SendPacketToLoginServer( purchasesUpdate, connectionId );
}

//---------------------------------------------------------------

void     SalesManager::VerifyThatUserHasEnoughMoneyForEntry2( const PacketDbQueryResult* dbResult )
{
   LogMessage( LOG_PRIO_INFO, "SalesManager::VerifyThatUserHasEnoughMoneyForEntry2" );

   PurchaseTracking* purchaseTracking = static_cast< PurchaseTracking* >( dbResult->customData );
   
   if( dbResult->successfulQuery == false )
   {
      m_parent->SendErrorToClient( purchaseTracking->connectionId, purchaseTracking->gatewayId, PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade );
      SendTournamentPurchaseResultBackToServer( purchaseTracking->fromOtherServerId, purchaseTracking->fromOtherServerTransactionId, PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade );
      delete purchaseTracking;
      m_usersBeingServiced.Remove( purchaseTracking->userUuid );
      return;
   }

   // verify that the user has enough of these items.
   bool doesPlayerHaveEnoughMoney = false;

   KeyValueParser            enigma( dbResult->bucket );
   KeyValueParser::iterator  it = enigma.begin();
   if( it != enigma.end() )
   {
      doesPlayerHaveEnoughMoney = true;
      while( it != enigma.end() )
      {
         KeyValueParser::row row = *it++;

         string productId = row[ TableKeyValue::Column_key ];
         string count = row[ TableKeyValue::Column_value ];
         int num = 0;
         if( count != "null" )
         {
            num = boost::lexical_cast< int >( count );
         }

         if( num < GetItemNumToDebit( purchaseTracking->itemsToSpend, productId ) )
         {
            doesPlayerHaveEnoughMoney = false;
            break;
         }
      }
   }
   else
   {
      doesPlayerHaveEnoughMoney = false;
   }


   if( doesPlayerHaveEnoughMoney == true )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = DiplodocusPurchase::QueryType_PerformPurchase;

      string queryOpen = OpenMultipleInsertsSimple();
      string middle;

      int numItems = purchaseTracking->itemsToSpend.size();
      for( int i=0; i< numItems; i++ )
      {
         const PurchaseServerDebitItem& item = purchaseTracking->itemsToSpend[i];
         
         middle += FormatInsertIntoUserJoinProductSimple( purchaseTracking->userUuid, item.productUuidToSpend, -( item.numToDebit ), i );
      }
      string queryClose = CloseMultipleInserts();
      string query = queryOpen + middle + queryClose;

      dbQuery->customData = purchaseTracking; // do not delete this data
      dbQuery->query = query;
      m_parent->AddQueryToOutput( dbQuery );
   }
   else
   {
      m_parent->SendErrorToClient( purchaseTracking->connectionId, purchaseTracking->gatewayId, PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade );
      SendTournamentPurchaseResultBackToServer( purchaseTracking->fromOtherServerId, purchaseTracking->fromOtherServerTransactionId, PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade );
      m_usersBeingServiced.Remove( purchaseTracking->userUuid );
      delete purchaseTracking;
   }

  /* PurchaseTracking* purchaseTracking = static_cast< PurchaseTracking* >( dbResult->customData );
   PacketPurchase_BuyResponse* packet = new PacketPurchase_BuyResponse;
   packet->purchaseUuid = purchaseTracking->exchangeUuid;*/
}

//---------------------------------------------------------------

bool     SalesManager::RequestAllProducts()
{
   LogMessage( LOG_PRIO_INFO, "Loading initial product list" );
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = DiplodocusPurchase::QueryType_LoadAllProducts;

   dbQuery->query = "SELECT * FROM product;";
   m_parent->AddQueryToOutput( dbQuery );

   return true;
}

//---------------------------------------------------------------

string   SalesManager::LookupString( const string& name, int languageId )
{
   assert( m_parent->GetStringLookup()->IsReady() == true );

   
   return m_parent->GetStringLookup()->GetString( name, languageId );
}

//---------------------------------------------------------------

bool     SalesManager::GetListOfItemsForSale( PacketPurchase_RequestListOfSalesResponse* packet, int userLanguageIndex )
{
   vector< ExchangeEntry >::iterator it = exchangeRates.begin();
   while( it != exchangeRates.end() )
   {
      const ExchangeEntry& ee = *it++;
      packet->thingsForSale.insert( ee.exchangeUuid, PurchaseInfo() );
      //KeyValue
      PurchaseInfo& pi = packet->thingsForSale.lastValue();
      pi.exchangeUuid =          ee.exchangeUuid;
      pi.beginDate =             ee.beginDate;
      pi.endDate =               ee.endDate;
      pi.exchangeTitle =         LookupString( ee.titleStringId, userLanguageIndex );
      pi.exchangeDescription =   LookupString( ee.descriptionStringId, userLanguageIndex );

      pi.productSourceUuid =     ee.sourceUuid;
      pi.productSourceName =     LookupString( ee.sourceNameStringId, userLanguageIndex );
      pi.quantityRequiredSource =ee.sourceCount;
      pi.iconHashSource =        ee.sourceIcon;

      pi.productDestUuid =       ee.sourceUuid;
      pi.productDestName =       LookupString( ee.destNameStringId, userLanguageIndex );
      pi.quantityGivenDest =     ee.sourceCount;
      pi.iconHashDest =          ee.destIcon;
   }

   if( packet->thingsForSale.size() > 0 )
      return true;

   return false;
}

//---------------------------------------------------------------

bool     SalesManager::FindItem( const string& exchangeUuid, ExchangeEntry& ee )
{
   //bool found = false;
   vector< ExchangeEntry >::iterator it = exchangeRates.begin();
   while( it != exchangeRates.end() )
   {
      if( it->exchangeUuid == exchangeUuid )
      {
         ee = *it;
         return true;
      }
      it++;
   }

   return false;
}

//---------------------------------------------------------------

bool     SalesManager::SendTournamentPurchaseResultBackToServer( U32 serverIdentifier, string serverTransactionUuid, int result )
{
   LogMessage( LOG_PRIO_INFO, "SalesManager::SendTournamentPurchaseResultBackToServer" );
   if( serverIdentifier == 0 )
      return false;
   assert( serverTransactionUuid.size() > 0 );

   PacketTournament_PurchaseTournamentEntryResponse* response = new PacketTournament_PurchaseTournamentEntryResponse;
   response->uniqueTransactionId = serverTransactionUuid;
   response->result = result;

   // PacketType_ServerJobWrapper
   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper;
   wrapper->serverId = serverIdentifier;
   wrapper->pPacket = response;


   if( m_parent->AddOutputChainData( wrapper, serverIdentifier ) == false )
   {
      PacketFactory factory;
      BasePacket* tempPack = static_cast< BasePacket* >( wrapper );
      factory.CleanupPacket( tempPack );
   }

   return true;
}

//---------------------------------------------------------------

bool     SalesManager::PerformSale( const string& purchaseUuid, const string& userUuid, 
                                   const UserConnectionList& connectionList, U32 userConnectionId, U32 userGatewayId,
                                   U32 serverIdentifier, string serverTransactionUuid )
{
   LogMessage( LOG_PRIO_INFO, "SalesManager::PerformSale2" );
   assert( connectionList.size() != 0 );

   ExchangeEntry ee;
   if( FindItem( purchaseUuid, ee ) == false )
   {
      m_parent->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Purchase_BadPurchaseId );
      SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_BadPurchaseId );
      return false;
   }

   //const string& userUuid = m_parent->GetUuid( userConnectionId );
   assert( userUuid.size() );

   if( m_usersBeingServiced.Find( userUuid ) == true )
   {
      m_parent->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Purchase_StoreBusy );
      SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_StoreBusy );
      return false;
   }

   if( ee.beginDate.size() > 1 )
   {
      int secondsUntil = GetDiffTimeFromRightNow( ee.beginDate.c_str() );
      if( secondsUntil < 0 )
      {
         m_parent->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Purchase_TimePeriodHasNotBegunYet );
         SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_TimePeriodHasNotBegunYet );
         return false;
      }
   }
   if( ee.endDate.size() > 1 )
   {
      int secondsUntil = GetDiffTimeFromRightNow( ee.endDate.c_str() );
      if( secondsUntil > 0 )
      {
         m_parent->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Purchase_TimePeriodHasExpired );
         SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_TimePeriodHasExpired );
         return false;
      }
   }

   m_usersBeingServiced.Add( userUuid );

   PurchaseTracking* purchaseLookup = new PurchaseTracking;
   purchaseLookup->userUuid = userUuid;
   purchaseLookup->exchangeUuid = purchaseUuid;
   purchaseLookup->connectionId = userConnectionId;
   purchaseLookup->gatewayId = userGatewayId;
   purchaseLookup->fromOtherServerId = serverIdentifier;
   purchaseLookup->fromOtherServerTransactionId = serverTransactionUuid;
   

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = DiplodocusPurchase::QueryType_VerifyThatUserHasEnoughMoney1;

   dbQuery->query = "SELECT SUM( num_purchased ) FROM user_join_product WHERE user_uuid='";
   dbQuery->query += userUuid;
   dbQuery->query += "' AND product_id='";
   dbQuery->query += ee.sourceUuid;
   dbQuery->query += "';";
   dbQuery->customData = purchaseLookup;
   m_parent->AddQueryToOutput( dbQuery );

   return true;
}

//---------------------------------------------------------------

bool     SalesManager::PerformSale( const SerializedVector< PurchaseServerDebitItem >& itemsToSpend, const string& userUuid, 
                                   const UserConnectionList& connectionList, U32 userConnectionId, U32 userGatewayId,
                                   U32 serverIdentifier, string serverTransactionUuid )
{
   LogMessage( LOG_PRIO_INFO, "SalesManager::PerformSale1" );
   assert( connectionList.size() != 0 );

   if( itemsToSpend.size() == 0 )
   {
      m_parent->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Purchase_NoTradeItemsSpecified );
      SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_NoTradeItemsSpecified );
      return false;
   }

   //const string& userUuid = m_parent->GetUuid( userConnectionId );
   assert( userUuid.size() );

   if( m_usersBeingServiced.Find( userUuid ) == true )
   {
      m_parent->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Purchase_StoreBusy );
      SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_StoreBusy );
      return false;
   }

   //---------------------------------
   set< string > validationList;
   int num = itemsToSpend.size();
   for( int i=0; i<num; i++ )
   {
      if( validationList.find( itemsToSpend[i].productUuidToSpend ) != validationList.end() )
      {
         m_parent->SendErrorToClient( connectionList, PacketErrorReport::ErrorType_Purchase_DuplicateItemsForPayment );
         SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_DuplicateItemsForPayment );
         return false;
      }
      validationList.insert( itemsToSpend[i].productUuidToSpend );
   }
   
   //---------------------------------

   m_usersBeingServiced.Add( userUuid );

   PurchaseTracking* purchaseLookup = new PurchaseTracking;
   purchaseLookup->userUuid = userUuid;
   purchaseLookup->connectionId = userConnectionId;
   purchaseLookup->gatewayId = userGatewayId;
   purchaseLookup->fromOtherServerId = serverIdentifier;
   purchaseLookup->fromOtherServerTransactionId = serverTransactionUuid;
   purchaseLookup->itemsToSpend = itemsToSpend;

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = DiplodocusPurchase::QueryType_VerifyThatUserHasEnoughMoney2;

   dbQuery->query = "SELECT product_id, SUM( num_purchased ) FROM user_join_product WHERE user_uuid='";
   dbQuery->query += userUuid;
   dbQuery->query += "' AND product_id IN (";
   for( int i=0; i<num; i++ )
   {
      dbQuery->query += "'";
      dbQuery->query += itemsToSpend[i].productUuidToSpend.c_str();
      if( i < num-1 ) // not at the end of the list
         dbQuery->query += "',";
   }
   dbQuery->query += "');";
   dbQuery->customData = purchaseLookup;
   m_parent->AddQueryToOutput( dbQuery );

   return true;
}

//---------------------------------------------------------------

bool     SalesManager::PerformSimpleInventoryAddition( const string& userUuid, string productUuid, int count, bool translateFromIAP )
{
   LogMessage( LOG_PRIO_INFO, "SalesManager::PerformSimpleInventoryAddition" );
   if( m_usersBeingServiced.Find( userUuid ) == true )
   {
      LogMessage( LOG_PRIO_ERR, "Error: SalesManager::PerformSimpleInventoryAddition " );
      LogMessage( LOG_PRIO_ERR, "user already being serviced and cannot add inventory item " );
      LogMessage( LOG_PRIO_ERR, " User uuid: %s", userUuid.c_str() );
      return false;
   }

   if( translateFromIAP == true )
   {
      bool  translated = false;
      ProductMapByUuidIterator it = m_productMapByUuid.find( productUuid );
      if( it != m_productMapByUuid.end() )
      {
         const Product& product = it->second;
         if( product.convertsToQuantity != 0 
               && product.convertsToProductId != 0 )
         {
            it = m_productMapByUuid.begin();
            while( it != m_productMapByUuid.end() )
            {
               if( it->second.productId == product.convertsToProductId )
               {
                  productUuid = it->second.uuid;
                  count *= product.convertsToQuantity; // this probably needs a better solution
                  translated = true;
                  break;
               }
               it++;
            }
         }
      }
      if( translated == false )
      {
         LogMessage( LOG_PRIO_INFO, "SalesManager::PerformSimpleInventoryAddition failed to find IAP" );
         return false;
      }
   }
   m_usersBeingServiced.Add( userUuid ); // << insertion

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = DiplodocusPurchase::QueryType_PerformInventoryAddition;
   dbQuery->meta = userUuid;

   string queryOpen = OpenMultipleInsertsSimple();
   string middle = FormatInsertIntoUserJoinProductSimple( userUuid, productUuid, count );

   string queryClose = CloseMultipleInserts();
   string query = queryOpen + middle + queryClose;

   //dbQuery->customData = purchaseTracking; // do not delete this data
   dbQuery->query = query;
   m_parent->AddQueryToOutput( dbQuery ); 

   return true;
}

//---------------------------------------------------------------

int      SalesManager::GetProductType( const string& uuid )
{
   map< string, Product >::iterator it = m_productMapByUuid.find( uuid );
   if( it == m_productMapByUuid.end() )
   {
      return -1;
   }

   return it->second.productType;
}

bool     SalesManager::GetProduct( const string& uuid, Product& product )
{
   map< string, Product >::iterator it = m_productMapByUuid.find( uuid );
   if( it == m_productMapByUuid.end() )
   {
      return false;
   }

   product = it->second;
   return  true;
}

//---------------------------------------------------------------

///////////////////////////////////////////////////////////////////////////////////////////

ExchangeEntry :: ExchangeEntry() : index( 0 )
{
}

ExchangeEntry& ExchangeEntry :: operator = ( ExchangeRateParser::row  row )
{
   //LogMessage( LOG_PRIO_INFO, "ExchangeEntry::operator =" );

   index =              boost::lexical_cast< int > ( row[ TableExchangeRateAggregate::Column_index ] );
   beginDate =          row[ TableExchangeRateAggregate::Column_begin_date ];
   endDate =            row[ TableExchangeRateAggregate::Column_end_date ];
   exchangeUuid =       row[ TableExchangeRateAggregate::Column_exchange_uuid ];
   titleStringId =      row[ TableExchangeRateAggregate::Column_title_id ];
   descriptionStringId =row[ TableExchangeRateAggregate::Column_description_id ];
   customUuid =         row[ TableExchangeRateAggregate::Column_custom_uuid ];

   sourceId =           row[ TableExchangeRateAggregate::Column_source_id ];
   sourceUuid =         row[ TableExchangeRateAggregate::Column_source_uuid ];
   sourceNameStringId = row[ TableExchangeRateAggregate::Column_source_name ];
   sourceCount =        boost::lexical_cast< int > ( row[ TableExchangeRateAggregate::Column_source_count ] );
   sourceIcon =         row[ TableExchangeRateAggregate::Column_source_icon ];
   sourceType =         boost::lexical_cast< int > ( row[ TableExchangeRateAggregate::Column_source_type ] );

   destId =             row[ TableExchangeRateAggregate::Column_dest_id ];
   destUuid =           row[ TableExchangeRateAggregate::Column_dest_uuid ];
   destNameStringId =   row[ TableExchangeRateAggregate::Column_dest_name ];
   destCount =          boost::lexical_cast< int > ( row[ TableExchangeRateAggregate::Column_dest_count ] );
   destIcon =           row[ TableExchangeRateAggregate::Column_dest_icon ];
   destType =           boost::lexical_cast< int > ( row[ TableExchangeRateAggregate::Column_dest_type ] );

   

   return *this;
}

//---------------------------------------------------------------

Product&    Product::operator = ( ProductTable::row  row )
{
   //LogMessage( LOG_PRIO_INFO, "Product::operator =" );
   id =                 boost::lexical_cast< int > ( row[ TableProduct::Column_id ] );
   productId =          boost::lexical_cast< int > ( row[ TableProduct::Column_product_id ] );
   uuid =               row[ TableProduct::Column_uuid ];
   name =               row[ TableProduct::Column_name ];
   vendorUuid =         row[ TableProduct::Column_vendor_uuid ];
   firstAvail =         row[ TableProduct::Column_begin_date ];
   productType =        boost::lexical_cast< int > ( row[ TableProduct::Column_product_type ] );
   nameStringLookup =   row[ TableProduct::Column_name_string ];
   iconLookup =         row[ TableProduct::Column_icon_lookup ];
   parentId =           boost::lexical_cast< int > ( row[ TableProduct::Column_parent_id ] );
   convertsToProductId =boost::lexical_cast< int > ( row[ TableProduct::Column_converts_to_product_id ] );
   convertsToQuantity = boost::lexical_cast< int > ( row[ TableProduct::Column_converts_to_quantity] );

   return *this;
}

///////////////////////////////////////////////////////////////////////////////////////////

