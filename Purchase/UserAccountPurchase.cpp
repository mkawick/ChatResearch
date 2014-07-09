// UserAccountPurchase.cpp

#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"

#include "UserAccountPurchase.h"
#include "DiplodocusPurchase.h"

#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/PurchasePacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

UserAccountPurchase::UserAccountPurchase( const UserTicket& ticket ) : m_userTicket( ticket ), m_status( Status_initial_login ), m_readyForCleanup( false ), m_purchaseManager( NULL )
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
   m_status = Status_awaiting_cleanup;
   m_readyForCleanup = true;
   time( &m_logoutTime );
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
   cout << "User purchase receipt" << endl;
   cout <<  "  purchaseItemId: " << receiptPacket->purchaseItemId << endl;
   cout <<  "  quantity      : " << receiptPacket->quantity << endl;
   cout <<  "  transactionId : " << receiptPacket->transactionId << endl;
   cout <<  "  platformId    : " << receiptPacket->platformId << endl;
   cout <<  "  receipt       : " << receiptPacket->receipt << endl;

   PacketPurchase_ValidatePurchaseReceiptResponse* response = new PacketPurchase_ValidatePurchaseReceiptResponse;
   response->transactionId = receiptPacket->transactionId;
   response->errorCode = rand() % 1;
   PacketGatewayWrapper* wrapper = new PacketGatewayWrapper;
   wrapper->SetupPacket( response, m_userTicket.connectionId );
   m_purchaseManager->AddOutputChainData( wrapper, m_userTicket.connectionId );

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
   assert( m_salesManager != NULL || m_userTicket.connectionId == 0 );

   bool success = m_salesManager->PerformSale( packet->itemsToSpend, m_userTicket, connectionId, packet->uniqueTransactionId );
   success = success;// warnings

   return true;
}


//------------------------------------------------------------------------------------------------
