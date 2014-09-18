// UserAccountPurchase.cpp

#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "UserAccountPurchase.h"
#include "PurchaseMainThread.h"

#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/PurchasePacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

UserAccountPurchase::UserAccountPurchase( const UserTicket& ticket ) : 
                     m_userTicket( ticket ), 
                     m_status( Status_initial_login ), 
                     m_readyForCleanup( false ), 
                     m_purchaseManager( NULL ),
                     m_salesManager( NULL ),
                     m_purchaseReceiptManager( NULL )
{
}

   //---------------------------------------

UserAccountPurchase::~UserAccountPurchase()
{
}

   //---------------------------------------

void  UserAccountPurchase::Update()
{
   if( m_purchaseManager == NULL )
      return;
}

   //---------------------------------------

void     UserAccountPurchase::UserLoggedOut()
{
   if( m_salesManager )
      m_salesManager->UserLoggedOut( m_userTicket.uuid );
   m_status = Status_awaiting_cleanup;
   m_readyForCleanup = true;
   time( &m_logoutTime );
}

   //---------------------------------------

void     UserAccountPurchase::ClearUserLogout()
{
   m_status = Status_initial_login;
   m_readyForCleanup = false;
   m_logoutTime = 0;
}

   //---------------------------------------

bool     UserAccountPurchase::LogoutExpired()
{
   if( m_readyForCleanup == false )
      return false;

   time_t currentTime;
   time( &currentTime );
   if( difftime( currentTime, m_logoutTime ) >= 3 ) // 3 seconds
   {
      return true;
   }
   return false;
}

   //---------------------------------------

void     UserAccountPurchase::StoreProducts( const StringBucket& bucket )
{
   m_productFilterNames.clear();

   list< string >::const_iterator it = bucket.bucket.begin();
   while( it != bucket.bucket.end() )
   {
      m_productFilterNames.insert( *it++ );
   }
}

   //---------------------------------------
   //---------------------------------------
   //---------------------------------------

bool     UserAccountPurchase::LoginKeyMatches( const string& loginKey ) const
{
   if( loginKey == m_userTicket.userTicket )
      return true;

   return false;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountPurchase::HandleRequestFromClient( const PacketPurchase* packet )
{
   switch( packet->packetSubType )
   {
   case PacketPurchase::PurchaseType_Buy:
      return MakePurchase( static_cast< const PacketPurchase_Buy* >( packet ) );

   case PacketPurchase::PurchaseType_RequestListOfSales:
      return GetListOfItemsForSale( static_cast< const PacketPurchase_RequestListOfSales* >( packet ) );

   case PacketPurchase::PurchaseType_EchoToServer:
      return EchoHandler();

   case PacketPurchase::PurchaseType_ValidatePurchaseReceipt:
      return HandleReceipt( static_cast< const PacketPurchase_ValidatePurchaseReceipt* >( packet ));
   }

   return false;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountPurchase::GetListOfItemsForSale( const PacketPurchase_RequestListOfSales* packet )
{
   assert( m_purchaseManager != NULL && m_userTicket.connectionId != 0 );

   PacketPurchase_RequestListOfSalesResponse* response = new PacketPurchase_RequestListOfSalesResponse();
   m_salesManager->GetListOfItemsForSale( response, m_userTicket.languageId );

   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->SetupPacket( response, m_userTicket.connectionId );

   m_purchaseManager->AddOutputChainData( wrapper, m_userTicket.connectionId );
   return true;
}


//---------------------------------------------------------------

bool  UserAccountPurchase::EchoHandler()
{
   PacketPurchase_EchoToClient* echo = new PacketPurchase_EchoToClient;
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->SetupPacket( echo, m_userTicket.connectionId );

   m_purchaseManager->AddOutputChainData( wrapper, m_userTicket.connectionId );

   return true;
}

//---------------------------------------------------------------

bool  UserAccountPurchase::HandleReceipt( const PacketPurchase_ValidatePurchaseReceipt* receiptPacket )
{
   LogMessage( LOG_PRIO_INFO, "User purchase receipt" );
   LogMessage( LOG_PRIO_INFO, "  purchaseItemId: %s", receiptPacket->purchaseItemId.c_str() );
   LogMessage( LOG_PRIO_INFO, "  quantity      : %d", receiptPacket->quantity );
   LogMessage( LOG_PRIO_INFO, "  transactionId : %s", receiptPacket->transactionId.c_str() );
   LogMessage( LOG_PRIO_INFO, "  platformId    : %d", receiptPacket->platformId );
   LogMessage( LOG_PRIO_INFO, "  Time received : %s", GetDateInUTC().c_str() );

   const string& temp = receiptPacket->receipt;
   
   int maxStrLen = 200;
   int strLength = temp.size();
   LogMessage( LOG_PRIO_INFO, "  receipt length: %d", strLength );

   if( strLength > maxStrLen )
      strLength = maxStrLen;
   

   LogMessage( LOG_PRIO_INFO, "   receipt overview + [" );

   for(int i=0; i< strLength; i += 20 )
   {
      int remaining = strLength - i;
      if ( remaining > 10 )
         remaining = 10;

      string str;
      for( int j=0; j<remaining; j++ )
      {
         str += ToHexString( temp[i+j] ) + " ";
      }
      LogMessage( LOG_PRIO_ERR, str.c_str() );
   }
   LogMessage( LOG_PRIO_INFO, "\n]" );

   bool  success = false;
   if( m_purchaseReceiptManager )
   {
      success = m_purchaseReceiptManager->WriteReceipt( receiptPacket, m_userTicket.userId, m_userTicket.uuid );
   }

   PacketPurchase_ValidatePurchaseReceiptResponse* response = new PacketPurchase_ValidatePurchaseReceiptResponse;
   response->transactionId = receiptPacket->transactionId;
   response->errorCode = ( success != true ) ;// 0 means things went well. 1 means bad
   m_purchaseManager->SendPacketToGateway( response, m_userTicket.connectionId, m_userTicket.gatewayId );

   return true;
}

//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

bool     UserAccountPurchase::MakePurchase( const PacketPurchase_Buy* packet )
{
   assert( m_salesManager != NULL || m_userTicket.connectionId == 0 );

   bool success = m_salesManager->PerformSale( packet->purchaseUuid.c_str(), m_userTicket );
   success = success;// warnings

   return true;
}
//------------------------------------------------------------------------------------------------

bool     UserAccountPurchase::MakePurchase( const PacketTournament_PurchaseTournamentEntry* packet, U32 connectionId )
{
   LogMessage( LOG_PRIO_INFO, "UserAccountPurchase::MakePurchase" );
   assert( m_salesManager != NULL || m_userTicket.connectionId == 0 );

   bool success = m_salesManager->PerformSale( packet->itemsToSpend, m_userTicket, connectionId, packet->uniqueTransactionId );
   success = success;// warnings

   return true;
}

//------------------------------------------------------------------------------------------------

bool     UserAccountPurchase::MakeRefund( const PacketTournament_PurchaseTournamentEntryRefund* refundPacket, U32 connectionId )
{
   LogMessage( LOG_PRIO_INFO, "UserAccountPurchase::MakeRefund" );
   assert( m_salesManager != NULL || m_userTicket.connectionId == 0 );
   int num = refundPacket->itemsToRefund.size();
   for( int i=0; i<num; i++ )
   {

      const PurchaseServerDebitItem& item = refundPacket->itemsToRefund[i];
      bool success = m_salesManager->PerformSimpleInventoryAddition( m_userTicket.uuid, item.productUuidToSpend, item.numToDebit );
   }

   PacketTournament_PurchaseTournamentEntryRefundResponse* response = new PacketTournament_PurchaseTournamentEntryRefundResponse;
   response->uniqueTransactionId = refundPacket->uniqueTransactionId;
   response->result = 0;// success
   m_purchaseManager->SendPacketToGateway( response, connectionId, m_userTicket.gatewayId );

   return true;
}


//------------------------------------------------------------------------------------------------
