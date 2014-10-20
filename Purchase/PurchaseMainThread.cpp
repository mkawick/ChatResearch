// PurchaseMainThread.cpp

#include <string>
using namespace std;

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Utils/StringUtils.h"

#include "PurchaseMainThread.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/Packets/TournamentPacket.h"

#include "../NetworkCommon/Database/StringLookup.h"
#include "../NetworkCommon/Database/Deltadromeus.h"
#include "../NetworkCommon/NetworkIn/DiplodocusServerToServer.h"
#include "../NetworkCommon/NetworkIn/DiplodocusTools.h"

#include <iostream>
#include <time.h>

using namespace std;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusPurchase::DiplodocusPurchase( const string& serverName, U32 serverId ): 
                              ChainedType( serverName, serverId, 0,  ServerType_Purchase ), 
                              m_salesManager( NULL ),
                              m_purchaseReceiptManager( NULL ),
                              m_stringLookup( NULL ),
                              m_isInitializing( true ),
                              m_isWaitingForAdminSettings( false )
                                         
{
   time( &m_initializingAdminSettingsTimeStamp );
   SetSleepTime( 100 );

   
   string exchangeQuery = "SELECT pe.id, pe.begin_date, pe.end_date, pe.enchange_uuid, pe.title_string, pe.description_string, pe.custom_uuid, ";
   exchangeQuery += " p1.product_id AS source_id, p1.uuid source_uuid, p1.name_string source_name, pe.source_count, p1.icon_lookup source_icon, p1.product_type source_type,";
   exchangeQuery += " p2.product_id AS dest_id, p2.uuid dest_uuid, p2.name_string dest_name, pe.dest_count, p2.icon_lookup dest_icon, p1.product_type dest_type ";       

   exchangeQuery += " FROM playdek.product_exchange_rate AS pe";
   exchangeQuery += " INNER JOIN product p1 on pe.product_source_id=p1.product_id";
   exchangeQuery += " INNER JOIN product p2 on pe.product_dest_id=p2.product_id ";

   string ReceiptQuery = "SELECT * FROM playdek.purchase_receipts WHERE validated_result=0 ORDER BY num_attempts_to_validate ASC;";
   
   const int FiveMinutes = 60 * 5;
   m_salesManager = new SalesManager( QueryType_ExchangeRateLookup, this, exchangeQuery, true );
   m_salesManager->SetPeriodicty( FiveMinutes ); //

   const int TwentySeconds = 20;
   m_purchaseReceiptManager = new PurchaseReceiptManager( QueryType_ReceiptLookup, this, ReceiptQuery, true );
   m_purchaseReceiptManager->SetPeriodicty( TwentySeconds ); //
   m_purchaseReceiptManager->SetSalesManager( m_salesManager );

   vector< string > stringCategories;
   stringCategories.push_back( string( "product" ) );
   stringCategories.push_back( string( "sale" ) );
   m_stringLookup = new StringLookup( QueryType_ProductLookup, this, stringCategories );
}

DiplodocusPurchase :: ~DiplodocusPurchase()
{
   //delete m_staticAssets;
   //delete m_dynamicAssets;
}
//---------------------------------------------------------------

void     DiplodocusPurchase::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   Khaan* localKhaan = static_cast< Khaan* >( khaan );
   localKhaan->AddOutputChainDataNoLock( packet );
   // this is not thread safe, but will be invoked from within the same thread.
   m_clientsNeedingUpdate.push_back( localKhaan->GetChainedId() );
}

/////////////////////////////////////////////////////////////////////////////////

void     DiplodocusPurchase::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanPurchase* khaan = static_cast< KhaanPurchase* >( chainedInput );

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "DiplodocusPurchase::Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_ERR, printer.c_str() );

   LogMessage( LOG_PRIO_ERR, "** InputRemovalInProgress" );
}


//---------------------------------------------------------------

void     DiplodocusPurchase::RequestAdminSettings()
{
   if( m_isInitializing == false )
      return;

   time_t currentTime;
   time(& currentTime );
   // try every 15 seconds
   if( difftime( currentTime, m_initializingAdminSettingsTimeStamp ) > 15 )
   {
      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_PurchaseAdminSettings;
      dbQuery->query = "SELECT * FROM admin_purchase";

      m_isWaitingForAdminSettings = true;

      AddQueryToOutput( dbQuery );

      m_initializingAdminSettingsTimeStamp = currentTime;
   }
}

//---------------------------------------------------------------

void     DiplodocusPurchase::HandleAdminSettings( const PacketDbQueryResult* dbResult )
{
   KeyValueParser              enigma( dbResult->bucket );
   KeyValueParser::iterator    it = enigma.begin();

   string   enpoint;

   if( enigma.size() == 0 )
   {
      LogMessage( LOG_PRIO_INFO, "No admin settings" );
      return;
   }
   while( it != enigma.end() )
   {
      KeyValueParser::row      row = *it++;
      string setting =         row[ TableKeyValue::Column_key ];
      string value =           row[ TableKeyValue::Column_value ];

      if( setting == "vendor.receipt_validation.apple" )
      {
         const string& endpoint = value;
         LogMessage( LOG_PRIO_INFO, "Vendor: Apple receipt validation value" );
         LogMessage( LOG_PRIO_INFO, value.c_str() );
         
         m_purchaseReceiptManager->SetValidationEndpoint( endpoint, Platform_ios );
      }
   }

   m_isInitializing = false;
   m_isWaitingForAdminSettings = false;
   
}

//---------------------------------------------------------------

//---------------------------------------------------------------
/*
string AssembleFullPath( const string& path, const string& fileName )
{
   string finalPath = path;

   char endValue = *path.rbegin() ;   
   if( endValue != '/' && endValue != '\\' ) // the back slash is windows support. Obviously will not work on linux.
   {
      char beginValue = *fileName.begin(); // note... different value from above
      if( beginValue != '/' && beginValue != '\\' )
      {
         finalPath += '/';
      }
   }

   finalPath += fileName;
   std::replace( finalPath.begin(), finalPath.end(), '\\', '/'); // convert everything to forward slashes

   return finalPath;
}*/


//---------------------------------------------------------------

bool     DiplodocusPurchase::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketCleaner cleaner( packet );
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32 connectionIdToUse = wrapper->connectionId;
      
      if( unwrappedPacket->packetType == PacketType_Purchase )
      {
         bool found = false;
         UAADMapIterator it = GetUserByConnectionId( connectionIdToUse );
         if( it != m_userTickets.end() )
            found = true;
         if( found == false )
         {
            LogMessage( LOG_PRIO_INFO, "packet from unknown connection", connectionIdToUse );
            PacketFactory factory;
            return true;
         }
         UserAccountPurchase& uap = it->second;
         PacketPurchase* pp = reinterpret_cast< PacketPurchase* >( unwrappedPacket );
         return uap.HandleRequestFromClient( pp );
      }
      else
      {
         assert( 0 );
      }
      // we handle all packets from the gateway here.
      return true;
   }
   else
   {
      m_mutex.lock();
      m_inputPacketsToBeProcessed.push_back( PacketStorage( packet, connectionId ) );
      m_mutex.unlock();
   }
   return true;
}

//---------------------------------------------------------------

bool     DiplodocusPurchase:: ProcessPacket( PacketStorage& storage )
{
   BasePacket* packet = storage.packet;
   U32 gatewayId = storage.gatewayId;

   U8 packetType = packet->packetType;

   if( packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );
      HandleCommandFromGateway( packet, gatewayId );
      return true;
   }

   if( packetType == PacketType_ServerJobWrapper )
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, gatewayId );
      return true;
   }
   
   PacketCleaner cleaner( packet );
   return false;
}



bool  DiplodocusPurchase::GetUser( const string& uuid, UserAccountPurchase*& user )
{
   U64 hashForUser = GenerateUniqueHash( uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it != m_userTickets.end() )// user may be reloggin and such.. no biggie.. just ignore
   {
      user = &it->second;
      return true;
   }
   return false;
}


DiplodocusPurchase::UAADMapIterator   
DiplodocusPurchase::GetUserByConnectionId( U32 connectionId )
{
   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.begin();
   while( it != m_userTickets.end() )
   {
      if( it->second.GetUserTicket().connectionId == connectionId )
      {
         return it;
      }
      it++;
   }

   return it;
}


//---------------------------------------------------------------

bool  DiplodocusPurchase::HandlePacketFromOtherServer( BasePacket* packet, U32 gatewayId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   //bool success = false;

   if( unwrappedPacket->packetType == PacketType_Login )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< PacketPrepareForUserLogin* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< PacketPrepareForUserLogout* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_ListOfProductsS2S:
         StoreUserProductsOwned( static_cast< PacketListOfUserProductsS2S* >( unwrappedPacket ) );
         return true;
      }
   }
   else if( unwrappedPacket->packetType == PacketType_Tournament )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketTournament::TournamentType_PurchaseTournamentEntry:
         {
            return HandlePurchaseRequest( static_cast< PacketTournament_PurchaseTournamentEntry* >( unwrappedPacket ), serverIdLookup );
         }
         break;
      case PacketTournament::TournamentType_PurchaseTournamentEntryRefund:
         {
            return HandlePurchaseRefund( static_cast< PacketTournament_PurchaseTournamentEntryRefund* >( unwrappedPacket ), serverIdLookup );
         }
         break;
      }
      return false;
   }

   return false;
}

//---------------------------------------------------------------

bool     DiplodocusPurchase::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   string uuid = loginPacket->uuid;
   U32 gatewayId = loginPacket->gatewayId;
   //LogMessage( LOG_PRIO_INFO, "Prep for logon: %d, %s, %s, %s", connectionId, loginPacket->userName.c_str(), uuid.c_str(), loginPacket->password.c_str() );
   LogMessage_LoginPacket( loginPacket );
            
   U64 hashForUser = GenerateUniqueHash( loginPacket->uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it != m_userTickets.end() )// user may be reloggin and such.. no biggie.. just ignore
   {
      it->second.SetConnectionId( connectionId );
      it->second.SetGatewayId( gatewayId );
      it->second.ClearUserLogout();
      return false;
   }

   bool found = false;
   if( found == false )
   {

      UserTicket ut;
      ut.userName =        loginPacket->userName.c_str();
      ut.uuid =            loginPacket->uuid.c_str();
      ut.userTicket =      loginPacket->loginKey.c_str();
      ut.connectionId =    loginPacket->connectionId;
      ut.gatewayId =       gatewayId;
      ut.gameProductId =   loginPacket->gameProductId;
      ut.userId =          loginPacket->userId;
      ut.languageId =      loginPacket->languageId;

      UserAccountPurchase user( ut );
      user.SetServer( this );
      user.SetSalesManager( m_salesManager );
      user.SetReceiptManager( m_purchaseReceiptManager );

      m_mutex.lock();
      m_userTickets.insert( UAADPair( hashForUser, user ) );
      m_mutex.unlock();
   }
   return true;
}

//---------------------------------------------------------------

bool     DiplodocusPurchase::DisconnectUser( const PacketPrepareForUserLogout* logoutPacket )
{
   LogMessage_LogoutPacket( logoutPacket );
   //LogMessage( LOG_PRIO_INFO, "Prep for logout: %d, %s", logoutPacket->connectionId, logoutPacket->uuid.c_str() );
            
   U32 connectionId = logoutPacket->connectionId;
   connectionId = connectionId;

   string uuid = logoutPacket->uuid;
   U64 hashForUser = GenerateUniqueHash( uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it == m_userTickets.end() )
      return false;

   //it->second.NeedsUpdate();
   it->second.UserLoggedOut();
   it->second.SetConnectionId( 0 );
   // we need to send notifications

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusPurchase::StoreUserProductsOwned( PacketListOfUserProductsS2S* productNamesPacket )
{
   string uuid = productNamesPacket->uuid;
   U64 hashForUser = GenerateUniqueHash( uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it == m_userTickets.end() )
      return false;

   it->second.StoreProducts( productNamesPacket->products );

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusPurchase::HandlePurchaseRequest( const PacketTournament_PurchaseTournamentEntry* packet, U32 connectionId )
{
   U64 hashForUser = GenerateUniqueHash( packet->userUuid.c_str() );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it != m_userTickets.end() )// user may be reloggin and such.. no biggie.. just ignore
   {
      //it->second.SetConnectionId( loginPacket->connectionId );
      it->second.MakePurchase( packet, connectionId );
      return true;
   }
   //packet->
   return false;
}

//---------------------------------------------------------------

bool     DiplodocusPurchase::HandlePurchaseRefund( const PacketTournament_PurchaseTournamentEntryRefund* packet, U32 connectionId )
{
   U64 hashForUser = GenerateUniqueHash( packet->userUuid.c_str() );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it != m_userTickets.end() )// user may be reloggin and such.. no biggie.. just ignore
   {
      //it->second.SetConnectionId( loginPacket->connectionId );
      it->second.MakeRefund( packet, connectionId );
      return true;
   }
   //packet->
   return false;
}

//---------------------------------------------------------------
//---------------------------------------------------------------
/*
bool     DiplodocusPurchase::AddQueryToOutput( PacketDbQuery* packet )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( packet, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   PacketFactory factory;
   BasePacket* delPacket = packet;
   factory.CleanupPacket( delPacket );
   //delete packet;/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}*/
////////////////////////////////////////////////////////////////////////////////////////

bool     DiplodocusPurchase::AddQueryToOutput( PacketDbQuery* dbQuery )
{
   PacketFactory factory;
   m_outputChainListMutex.lock();
   BaseOutputContainer tempOutputContainer = m_listOfOutputs;
   m_outputChainListMutex.unlock();

   ChainLinkIteratorType itOutputs = tempOutputContainer.begin();
   while( itOutputs != tempOutputContainer.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->GetChainedType() == ChainedType_DatabaseConnector )
      {
         bool isValidConnection = false;
         Database::Deltadromeus* delta = static_cast< Database::Deltadromeus* >( outputPtr );
         if( dbQuery->dbConnectionType != 0 )
         {
            if( delta->WillYouTakeThisQuery( dbQuery->dbConnectionType ) )
            {
               isValidConnection = true;
            }
         }
         else // if this query is not set, default to true
         {
            isValidConnection = true;
         }
         if( isValidConnection == true )
         {
            if( outputPtr->AddInputChainData( dbQuery, m_chainId ) == true )
            {
               return true;
            }
         }
      }
      itOutputs++;
   }

   BasePacket* deleteMe = static_cast< BasePacket*>( dbQuery );

   factory.CleanupPacket( deleteMe );
   return false;
}

//---------------------------------------------------------------

// make sure to follow the model of the account server regarding queries. Look at CreateAccount
// 
bool     DiplodocusPurchase::AddOutputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      if( m_connectionIdGateway == 0 )
         return false;

      U32 gatewayId = m_connectionIdGateway;
      UAADMapIterator it = GetUserByConnectionId( connectionId );
      if( it != m_userTickets.end() )
         gatewayId = it->second.GetUserTicket().gatewayId;  

      Threading::MutexLock locker( m_inputChainListMutex );
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      if( itInputs != m_listOfInputs.end() )// only one output currently supported.
      {
         ChainType* inputPtr = static_cast< ChainType*> ( (*itInputs).m_interface );
         if( inputPtr->GetChainedType() == ChainedType_InboundSocketConnector )
         {
            KhaanPurchase* khaan = static_cast< KhaanPurchase* >( inputPtr );
            if( khaan->GetServerId() == gatewayId )
            {
               khaan->AddOutputChainData( packet );
               //khaan->Update();// the gateway may not have a proper connection id.

               //AddServerNeedingUpdate( khaan->GetServerId() );
               MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );
               return true;
            }
         }
         itInputs++;
      }
      return true;
   }
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      return HandlePacketToOtherServer( packet, connectionId );
   }
   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         bool wasHandled = false;
         PacketDbQueryResult* dbResult = static_cast<PacketDbQueryResult*>( packet );
         if( m_salesManager->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }
         else if( m_purchaseReceiptManager->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }
         else if( m_stringLookup->HandleResult( dbResult ) == true )
         {
            wasHandled = true;
         }
         else
         {
            switch( dbResult->lookup )
            {
            case QueryType_PurchaseAdminSettings:
               {
                  HandleAdminSettings( dbResult );
                  wasHandled = true;
               }
               break;
            }
         }

         if( wasHandled == true )
         {
            PacketFactory factory;
            factory.CleanupPacket( packet );
         }
         return wasHandled;
      }
   }
   if( m_userTickets.size() )
   {
      Threading::MutexLock locker( m_mutex );
      UAADMapIterator it = m_userTickets.begin();
      while( it != m_userTickets.end() )
      {
         UAADMapIterator currentIt = it++;
         if( currentIt->second.LogoutExpired() )
         {
            //delete it->second;// bad idea
            m_userTickets.erase( currentIt );
         }
      }
   }

   return false;
}

/////////////////////////////////////////////////////////////////

bool  DiplodocusPurchase::SendPacketToLoginServer( BasePacket* packet, U32 connectionId )
{
   PacketServerJobWrapper* wrapper = new PacketServerJobWrapper();

   wrapper->serverId =  m_serverId;
   wrapper->pPacket = packet;
   wrapper->jobId = 0;

   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   if( itInputs != m_listOfInputs.end() )// only one output currently supported.
   {
      ChainLink & chainedInput = *itInputs;
      DiplodocusServerToServer* diplodocusS2s = static_cast< DiplodocusServerToServer* >( chainedInput.m_interface );
      U32 serverId = diplodocusS2s->FindServerIdByType( ServerType_Login );
      if( diplodocusS2s->AddOutputChainData( wrapper, serverId ) == true )
      {
         return true;
      }
   }


   PacketFactory factory;
   packet = wrapper;
   factory.CleanupPacket( packet );
   

   return false;
}

/////////////////////////////////////////////////////////////////

//---------------------------------------------------------------

int      DiplodocusPurchase::CallbackFunction()
{
   RequestAdminSettings();

   CleanupOldClientConnections( "KhaanPurchase" );
   UpdateAllConnections( "KhaanPurchase" );

   UpdateInputPacketToBeProcessed();

   time_t currentTime;
   time( &currentTime );
   m_salesManager->Update( currentTime );
   m_purchaseReceiptManager->Update( currentTime );
   m_stringLookup->Update( currentTime );


   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, static_cast<int>( m_connectedClients.size() ), m_listeningPort, m_serverName );
   // check for new friend requests and send a small list of notifications

   return 1;
}


//---------------------------------------------------------------
//---------------------------------------------------------------