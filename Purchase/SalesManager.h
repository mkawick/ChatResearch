#pragma once

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Database/QueryHandler.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"

#include "DiplodocusPurchase.h"
#include "UserAccountPurchase.h"


class DiplodocusPurchase;
class PacketPurchase_RequestListOfSalesResponse;


//------------------------------------------------------------

class TableExchangeRateAggregate
{
public:
   enum Columns
   {
      Column_index,
      Column_begin_date,
      Column_end_date,
      Column_exchange_uuid,
      Column_title_id,
      Column_description_id,
      Column_custom_uuid,
      Column_source_id,
      Column_source_uuid,
      Column_source_name,
      Column_source_count,
      Column_source_icon,
      Column_source_type,
      Column_dest_id,
      Column_dest_uuid,
      Column_dest_name,
      Column_dest_count,
      Column_dest_icon,
      Column_dest_type,

      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableExchangeRateAggregate> ExchangeRateParser;

//------------------------------------------------------------

class ExchangeEntry
{
public:
   ExchangeEntry();
   ExchangeEntry& operator = ( ExchangeRateParser::row  row );

   int      index;
   string   beginDate;
   string   endDate;
   string   exchangeUuid;
   string   titleStringId;
   string   descriptionStringId;

   string   sourceId;
   string   sourceUuid;
   string   sourceNameStringId;
   int      sourceCount;
   string   sourceIcon;
   int      sourceType;

   string   destId;
   string   destUuid;
   string   destNameStringId;
   int      destCount;
   string   destIcon;
   int      destType;

   string   customUuid;
};

//------------------------------------------------------------

class PurchaseTracking
{
public:
   string   userUuid;
   string   exchangeUuid;
   
   U32      connectionId;

   U32      fromOtherServerId;
   string   fromOtherServerTransactionId;

   SerializedVector< PurchaseServerDebitItem > itemsToSpend;
};

//------------------------------------------------------------

class Product
{
public:
   int      id;
   int      productId;
   string   uuid;
   string   name;
   string   vendorUuid;
   string   firstAvail;
   int      productType;
   string   nameStringLookup;
   string   iconLookup;

   Product& operator = ( ProductTable::row  row );
};

///////////////////////////////////////////////////////////////////////////////////////////

class SalesManager : public QueryHandler< DiplodocusPurchase* >
{
public: 
   typedef QueryHandler< DiplodocusPurchase* > ParentType;
public:
   enum ProductType // this is also maintained in the login sever
   {
      ProductType_game = 1,
      ProductType_deck_expansion,
      ProductType_consumable,
      ProductType_ticket,
      ProductType_money,
      ProductType_tournament_entry
   };
public:
   SalesManager( U32 id, ParentQueryerPtr parent, string& query, bool runQueryImmediately );
   bool     HandleResult( const PacketDbQueryResult* dbResult );

   bool     GetListOfItemsForSale( PacketPurchase_RequestListOfSalesResponse* packet, int userLanguageIndex = 1 );// english
   bool     PerformSale( const string& purchaseUuid, const UserTicket& userPurchasing, U32 serverIdentifier = 0, string serverTransactionUuid = "" );
   bool     PerformSale( const SerializedVector< PurchaseServerDebitItem >& itemsToSpend, const UserTicket& userPurchasing, U32 serverIdentifier, string serverTransactionUuid = "" );

   int      GetProductType( const string& uuid );

   void     Update( time_t currentTime );

private:

   bool     FindItem( const string& exchangeUuid, ExchangeEntry& ee );
   string   LookupString( const string& name, int languageId );

   bool     RequestAllProducts();
   bool     SendTournamentPurchaseResultBackToServer( U32 serverIdentifier, string serverTransactionUuid, int result );
   void     VerifyThatUserHasEnoughMoneyForEntry2( const PacketDbQueryResult* dbResult );

   bool     m_isServicingExchangeRates;
   bool     m_isInitializing, m_hasSendProductRequest;
   vector< ExchangeEntry > exchangeRates;
   set< string >           m_usersBeingServiced;

   map< string, Product >  m_productMapByUuid;
   typedef pair< string, Product >  ProductUuidPair;
};
///////////////////////////////////////////////////////////////////////////////////////////
