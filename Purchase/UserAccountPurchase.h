#pragma once

#include <time.h>
#include <set>
#include <vector>
using namespace std;

#include "../NetworkCommon/DataTypes.h"

class UserTicket // *consider making this a packet*
{
public:
   U32      userId;
   string   uuid;
   string   userName;// just for debugging purposes
   U32      connectionId;/// only after this person has logged in
   string   userTicket;
   string   dateOfLastRequest;// once the client tells us what this is
   U8       gameProductId;
   U32      languageId;
};


class DiplodocusPurchase;
class PacketPurchase;
class PacketPurchase_Buy;
class PacketPurchase_RequestListOfSales;
class StringBucket;
class SalesManager;

///////////////////////////////////////////////////////////////////

class UserAccountPurchase
{
   enum Status
   {
      Status_initial_login,
      Status_query_assets_for_product,
      Status_awating_user_login,
      Status_assemble_assets_for_delivery,
      Status_send_files_to_client,
      Status_awaiting_cleanup
   };

public:
   
   //---------------------------------------

   UserAccountPurchase( const UserTicket& ticket );
   ~UserAccountPurchase();

   //---------------------------------------

   bool              LoginKeyMatches( const string& loginKey ) const;
   const UserTicket& GetUserTicket() const { return m_userTicket; }
   void              SetConnectionId( U32 connId ) { m_userTicket.connectionId = connId; }

   void              SetServer( DiplodocusPurchase* assetManager ) { m_purchaseManager = assetManager; }
   void              SetSalesManager( SalesManager* manager ) { m_salesManager = manager; }
   bool              HandleRequestFromClient( const PacketPurchase* packet );

   void              UserLoggedOut();
   bool              LogoutExpired();
   void              Update();

   void              StoreProducts( const StringBucket& bucket );

   //---------------------------------------

private:

   bool              MakePurchase( const PacketPurchase_Buy* packet );
   bool              GetListOfItemsForSale( const PacketPurchase_RequestListOfSales* packet );

   UserTicket        m_userTicket;
   Status            m_status;
   bool              m_readyForCleanup;
   DiplodocusPurchase*  m_purchaseManager;
   SalesManager*     m_salesManager;

   time_t            m_logoutTime;
   set< string >     m_productFilterNames;
};

///////////////////////////////////////////////////////////////////
