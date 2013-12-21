#include "SalesManager.h"


#include <time.h>
#include <iostream>

#include "../NetworkCommon/Database/StringLookup.h"
#include "../NetworkCommon/Packets/PurchasePacket.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"
//#include "../NetworkCommon/Utils/TableWrapper.h"
#include <boost/lexical_cast.hpp>

using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////

SalesManager::SalesManager( U32 id, ParentQueryerPtr parent, string& query, bool runQueryImmediately ) : ParentType( id, 20, parent, runQueryImmediately ), 
                              m_isServicingExchangeRates( false ),
                              m_isInitializing( true ),
                              m_hasSendProductRequest( false )
{
   m_queryString = query;
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

//---------------------------------------------------------------

bool     SalesManager::HandleResult( const PacketDbQueryResult* dbResult )
{
   int lookupType = dbResult->lookup;
   if( lookupType == m_queryType )
   {
      SetValueOnExit< bool >           setter( m_isServicingExchangeRates, false );// due to multiple exit points...

      ExchangeRateParser            enigma( dbResult->bucket );
      ExchangeRateParser::iterator  it = enigma.begin();
      if( enigma.m_bucket.size() > 0 )
      {
         //cout << " Successful query = " << m_queryString << endl;
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
      ProductTable         enigma( dbResult->bucket );
      ProductTable::iterator  it = enigma.begin();
      while( it != enigma.end() )
      {
         Product p;
         p = *it++;
         m_productMapByUuid.insert( ProductUuidPair( p.uuid, p ) );
      }
      m_isInitializing = false;
      return true;
   }

   if( lookupType == DiplodocusPurchase::QueryType_VerifyThatUserHasEnoughMoney )
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
         m_parent->SendErrorToClient( purchaseTracking->connectionId, PacketErrorReport::ErrorType_Purchase_ItemNotAvailable );
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
         m_parent->SendErrorToClient( purchaseTracking->connectionId, PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade );
         SendTournamentPurchaseResultBackToServer( purchaseTracking->fromOtherServerId, purchaseTracking->fromOtherServerTransactionId, PacketErrorReport::ErrorType_Purchase_UserDoesNotHaveEnoughToTrade );
         m_usersBeingServiced.erase( purchaseTracking->userUuid );
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
         m_parent->SendErrorToClient( purchaseTracking->connectionId, PacketErrorReport::ErrorType_Purchase_Success );
         SendTournamentPurchaseResultBackToServer( purchaseTracking->fromOtherServerId, purchaseTracking->fromOtherServerTransactionId, PacketErrorReport::ErrorType_Purchase_Success );
         packet->success = true;
      }
      else
      {
         packet->success = false;
      }

      m_parent->SendPacketToGateway( packet, purchaseTracking->connectionId );

      m_usersBeingServiced.erase( purchaseTracking->userUuid );
      delete purchaseTracking;

      return true;
   }

   return false;
}

//---------------------------------------------------------------

bool     SalesManager::RequestAllProducts()
{
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
   bool found = false;
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

bool     SalesManager::PerformSale( const string& purchaseUuid, const UserTicket& userPurchasing, U32 serverIdentifier, string serverTransactionUuid )
{
   ExchangeEntry ee;
   if( FindItem( purchaseUuid, ee ) == false )
   {
      m_parent->SendErrorToClient( userPurchasing.connectionId, PacketErrorReport::ErrorType_Purchase_BadPurchaseId );
      SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_BadPurchaseId );
      return false;
   }

   if( m_usersBeingServiced.find( userPurchasing.uuid ) != m_usersBeingServiced.end() )
   {
      m_parent->SendErrorToClient( userPurchasing.connectionId, PacketErrorReport::ErrorType_Purchase_StoreBusy );
      SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_StoreBusy );
      return false;
   }

   if( ee.beginDate.size() > 1 )
   {
      int secondsUntil = GetDiffTimeFromRightNow( ee.beginDate.c_str() );
      if( secondsUntil < 0 )
      {
         m_parent->SendErrorToClient( userPurchasing.connectionId, PacketErrorReport::ErrorType_Purchase_TimePeriodHasNotBegunYet );
         SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_TimePeriodHasNotBegunYet );
         return false;
      }
   }
   if( ee.endDate.size() > 1 )
   {
      int secondsUntil = GetDiffTimeFromRightNow( ee.endDate.c_str() );
      if( secondsUntil > 0 )
      {
         m_parent->SendErrorToClient( userPurchasing.connectionId, PacketErrorReport::ErrorType_Purchase_TimePeriodHasExpired );
         SendTournamentPurchaseResultBackToServer( serverIdentifier, serverTransactionUuid, PacketErrorReport::ErrorType_Purchase_TimePeriodHasExpired );
         return false;
      }
   }

   m_usersBeingServiced.insert( userPurchasing.uuid );

   PurchaseTracking* purchaseLookup = new PurchaseTracking;
   purchaseLookup->userUuid = userPurchasing.uuid;
   purchaseLookup->exchangeUuid = purchaseUuid;
   purchaseLookup->connectionId = userPurchasing.connectionId;
   purchaseLookup->fromOtherServerId = serverIdentifier;
   purchaseLookup->fromOtherServerTransactionId = serverTransactionUuid;
   

   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = DiplodocusPurchase::QueryType_VerifyThatUserHasEnoughMoney;

   dbQuery->query = "SELECT SUM( num_purchased ) FROM user_join_product WHERE user_uuid='";
   dbQuery->query += userPurchasing.uuid;
   dbQuery->query += "' AND product_id='";
   dbQuery->query += ee.sourceUuid;
   dbQuery->query += "';";
   dbQuery->customData = purchaseLookup;
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

//---------------------------------------------------------------
/*
void     SalesManager::CreateBlankProfile( const string& user_id, int productId )
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
*/

///////////////////////////////////////////////////////////////////////////////////////////

ExchangeEntry :: ExchangeEntry() : index( 0 )
{
}

ExchangeEntry& ExchangeEntry :: operator = ( ExchangeRateParser::row  row )
{
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


Product&    Product::operator = ( ProductTable::row  row )
{
   id =                 boost::lexical_cast< int > ( row[ TableProduct::Column_id ] );
   productId =          boost::lexical_cast< int > ( row[ TableProduct::Column_product_id ] );
   uuid =               row[ TableProduct::Column_uuid ];
   name =               row[ TableProduct::Column_name ];
   filterName =         row[ TableProduct::Column_filter_name ];
   firstAvail =         row[ TableProduct::Column_begin_date ];
   productType =        boost::lexical_cast< int > ( row[ TableProduct::Column_product_type ] );
   nameStringLookup =       row[ TableProduct::Column_name_string ];
   iconLookup =         row[ TableProduct::Column_icon_lookup ];

   return *this;
}
