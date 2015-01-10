// LoginMainThread.cpp

#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
using boost::format;

#include <assert.h>

#include "../NetworkCommon/Utils/TableWrapper.h"
#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/Utils/StringUtils.h"
#include "../NetworkCommon/Database/StringLookup.h"

#include "../NetworkCommon/ServerConstants.h"

#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/GamePacket.h"
#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/CheatPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "../NetworkCommon/Database/Deltadromeus.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "LoginMainThread.h"
#include "FruitadensLogin.h"

#include "ScheduledOutageDBReader.h"
#include "ProductManager.h"


//////////////////////////////////////////////////////////

LoginMainThread:: LoginMainThread( const string& serverName, U32 serverId )  : 
                  Queryer(),
                  ChainedType( serverName, serverId, 0, ServerType_Login ), 
                  StatTrackingConnections(),
                  m_isInitialized( false ), 
                  m_isInitializing( true ),
                  m_autoAddProductFromWhichUsersLogin( true ),
                  m_stringLookup( NULL ),
                  m_numRelogins( 0 ),
                  m_numFailedLogins( 0 ),
                  m_numSuccessfulLogins( 0 ),
                  m_totalUserLoginSeconds( 0 ),
                  m_totalNumLogouts( 0 ),
                  m_printPacketTypes( false ),
                  m_printFunctionNames( false ),
                  m_productManager( NULL ),
                  m_outageReader( NULL ),
                  m_temporaryConnectionId( 1024 * 1024 * 16 ) // 16 million
{
   SetSleepTime( 19 );
   LogOpen();
   LogMessage( LOG_PRIO_INFO, "LoginMainThread::Login server created" );

   time_t currentTime;
   time( &currentTime );
   m_timestampHourlyStatServerStatisics = currentTime;
}

/////////////////////////////////////////////////////////////////////////////////

void  LoginMainThread:: Init()
{
   vector< string > stringCategories;
   stringCategories.push_back( string( "product" ) );
   stringCategories.push_back( string( "sale" ) );
   m_stringLookup = new StringLookup( QueryType_ProductStringLookup, this, stringCategories );

   m_timestampDailyStatServerStatisics = m_timestampHourlyStatServerStatisics;

   m_timestampHourlyStatServerStatisics = ZeroOutMinutes( m_timestampHourlyStatServerStatisics );
   m_timestampDailyStatServerStatisics = ZeroOutHours( m_timestampDailyStatServerStatisics );

   m_outageReader = new ScheduledOutageDBReader;
   m_outageReader->SetMainLoop( this );

   m_productManager = new ProductManager;
   m_productManager->Init();
   m_productManager->PrintFunctionNames( m_printFunctionNames );
   m_productManager->SetLoginMainThread( this );
}

/////////////////////////////////////////////////////////////////////////////////

void           LoginMainThread:: PrintPacketTypes( bool printingOn )
{
   m_printPacketTypes = printingOn;
   if( printingOn == true )
   {
      cout << "Not functioning" << endl;
      assert(0);
   }
}

void          LoginMainThread::  PrintFunctionNames( bool printingOn )
{
   m_printFunctionNames = printingOn;
}

/////////////////////////////////////////////////////////////////////////////////

void     LoginMainThread:: ServerWasIdentified( IChainedInterface* khaan )
{
   if( m_printFunctionNames )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   Khaan* localKhaan = static_cast< Khaan* >( khaan );
   localKhaan->AddOutputChainDataNoLock( packet );
   // this is not thread safe, but will be invoked from within the same thread.
   m_clientsNeedingUpdate.push_back( localKhaan->GetChainedId() );
}

/////////////////////////////////////////////////////////////////////////////////

void     LoginMainThread::InputConnected( IChainedInterface* chainedInput )
{
   if( m_printFunctionNames )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }
   LogMessage( LOG_PRIO_INFO, "LoginMainThread::InputConnected" );
   KhaanLogin* khaan = static_cast< KhaanLogin* >( chainedInput );
   string currentTime = GetDateInUTC();

   string printer = "Accepted connection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_INFO, printer.c_str() );
   if( m_printFunctionNames )
   {
      LogMessage( LOG_PRIO_INFO, printer.c_str() );
   }
   PrintDebugText( "** InputConnected" , 1 );

   int outputBufferSize = 128 * 1024;
   LogMessage( LOG_PRIO_INFO, "LoginMainThread::SetOutputBufferSize( %d )", outputBufferSize );
   khaan->SetOutboudBufferSize( outputBufferSize );
}

/////////////////////////////////////////////////////////////////////////////////

void     LoginMainThread::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanLogin* khaan = static_cast< KhaanLogin* >( chainedInput );

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "LoginMainThread::Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_INFO, printer.c_str() );

   LogMessage( LOG_PRIO_ERR, "** InputRemovalInProgress" );
}

/////////////////////////////////////////////////////////////////////////////////

bool     LoginMainThread:: AddInputChainData( BasePacket* packet, U32 gatewayId )
{
   if( m_printFunctionNames )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }
   // all packets coming in should be from the gateway only through s2s connections.
   // this is already multi threaded, so putting threading protections here is duplicative.

   if( packet->packetType != PacketType_GatewayWrapper )
   {
      string text = "Login server: received junk packets. Type: ";
      text += packet->packetType;
      LogMessage( LOG_PRIO_ERR, text.c_str() );
      return false;
   }

   m_mutex.lock();
   m_inputPacketsToBeProcessed.push_back( PacketStorage( packet, gatewayId ) );
   m_mutex.unlock();
   return true;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

// data going out can go only a few directions
// coming from the DB, we can have a result or possibly a different packet meant for a single chat UserConnection
// otherwise, coming from a UserConnection, to go out, it will already be packaged as a Gateway Wrapper and then 
// we simply send it on.
bool     LoginMainThread::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      return HandlePacketToOtherServer( packet, connectionId );
   }

   if( packet->packetType == PacketType_ServerToServerWrapper )
   {
      PacketServerToServerWrapper* wrapper = static_cast< PacketServerToServerWrapper* >( packet );

      U32 connectedServerId = wrapper->serverId;
      connectedServerId = connectedServerId;
      if( connectionId == m_serverId || connectionId == 0 )// outgoing... we are send it
      {
         return HandlePacketToOtherServer( wrapper, connectionId );
      }
      else
      {
         HandlePacketFromOtherServer( wrapper->pPacket, connectionId );
         return true;
      }
   }

   if( packet->packetType == PacketType_DbQuery )// coming from the db
   {
      Threading::MutexLock locker( m_mutex );
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast<PacketDbQueryResult*>( packet );
         m_dbQueries.push_back( result );
      /*   if( result->customData != NULL )
            cout << "AddOutputChainData: Non-null custom data " << endl;*/
      }
      return true;
      //assert(0);/// should not happen
   }
   
   return false;
}


//////////////////////////////////////////////////////////////////////////

bool     LoginMainThread:: ProcessInboundPacket( PacketStorage& storage )
{
   BasePacket* packet = storage.packet;
   U32 gatewayId = storage.gatewayId;
   U8 gameProductId = packet->gameProductId;

   PacketGatewayWrapper*   wrapper = static_cast< PacketGatewayWrapper * >( packet );
   U32            userConnectionId =   wrapper->connectionId;
   BasePacket*    actualPacket =       wrapper->pPacket;
   U8             packetType =         actualPacket->packetType;
   U8             packetSubType =      actualPacket->packetSubType;

   if( packetType != PacketType_Login ) // we will not accept anything but login packets
   {
      return false;
   }

   // we'll need to add the user... only a few packets will be allowed at this point.
   if( packetSubType == PacketLogin::LoginType_LoginFromGateway )
   {
      const PacketLoginFromGateway* login = static_cast< const PacketLoginFromGateway* >( actualPacket );
      LogUserIn( login, gatewayId, userConnectionId, gameProductId );
      return true;
   }
   if( packetSubType == PacketLogin::LoginType_RequestOtherUserProfile )
   {
      const PacketRequestOtherUserProfile* profileRequest = static_cast< const PacketRequestOtherUserProfile*>( actualPacket );
      LoadUserWithoutLogin( profileRequest->userName, userConnectionId );
      return true;
   }
   if( packetSubType == PacketLogin::LoginType_CreateAccount )
   {
      PacketCreateAccount* createAccount = static_cast<PacketCreateAccount*>( actualPacket );
      //m_accountCreator->CreateUserAccount( userConnectionId, gatewayId, createAccount->userEmail, createAccount->password, createAccount->userName, createAccount->deviceAccountId, createAccount->deviceId, createAccount->languageId, createAccount->gameProductId );
      return true;
   }
   if( packetSubType == PacketLogin::LoginType_LogoutAllUsers )
   {
      PacketLogin_LogoutAllUsers* usersLogout = static_cast< PacketLogin_LogoutAllUsers* >( actualPacket );
      LogoutListOfUsers( usersLogout, gatewayId );
      return true;
   }
   
   //return true;

   //UserConnectionLookupIterator it = m_userConnectionLookupMap.find( userConnectionId );
   //if( it == m_userConnectionLookupMap.end() )
   LoginMainThread::UserLoginMapIterator userIt = FindUserConnection( userConnectionId );
   if( userIt == m_userList.end() )
   {
      LogMessage( LOG_PRIO_ERR, "ProcessPacket cannot process this packet" );
   }
   else
   {
      //we have a user.... what do we do now.
      return userIt->second->HandlePacket( actualPacket, userConnectionId, gatewayId, gameProductId );
      //return true;
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool  LoginMainThread::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   m_mutex.lock();
   m_outputPacketsToBeProcessed.push_back( PacketStorage( packet, connectionId ) );
   m_mutex.unlock();
   return true;
}

//////////////////////////////////////////////////////////////////////////

bool     LoginMainThread:: ProcessOutboundPacket( PacketStorage& storage )
{
   BasePacket* packet = storage.packet;
   U32   connectionId = storage.gatewayId;
   U8    gameProductId = packet->gameProductId;

   U8 packetType = packet->packetType;
   U8 subType = packet->packetSubType;

   switch( packetType )
   {
      case PacketType_Login:
      {
         switch( subType )
         {
         case PacketLogin::LoginType_UserListOfPurchasesWasUpdated:
            {
               PacketListOfUserPurchasesUpdated* userInventory = static_cast<PacketListOfUserPurchasesUpdated*>( packet );
               U32 userConnectionId = userInventory->userConnectionId;
               LoginMainThread::UserLoginMapIterator userIt = FindUserConnection( userConnectionId );
               if( userIt != m_userList.end() )
               {
                  userIt->second->RequestListOfPurchases( userConnectionId );
               }
            }
            break;
         case PacketLogin::LoginType_RequestLoginStatusOfAllUsers:
            {
               FruitadensLogin* fruity = FindNetworkOutLink( connectionId );
               if( fruity )
               {
                  UserLoginMapIterator userIt = m_userList.begin();
                  while( userIt != m_userList.end() )
                  {
                     UserLogin* user = userIt->second;
                     vector< LoginConnectionDetails > ids;
                     user->GetListOfConnectedIds( ids );
                     for( U32 i=0; i<ids.size(); i++ )
                     {
                        SendLoginStatus( fruity, 
                           user->GetUsername(),
                           user->GetUuid(),
                           ids[i].connectionId,
                           ids[i].gameProductId,
                           user->GetLastLoginTime(), 
                           user->GetIsActive(), 
                           user->GetEmail(),
                           user->GetPassword(),
                           user->GetId(),
                           user->GetAssetServerKey(),
                           user->GetLanguageId(),
                           true,// only logged-in connections are in this list
                           false,// not disconnected,
                           ids[i].gatewayId
                           );
                     }
                     userIt++;
                  }
               }
            }
            break;
         case PacketLogin::LoginType_RequestToLogoutAllUsersForGame:
            {
               // copy this locally because we are going to change it as we log users out
               UserConnectionLookup localMap = m_userConnectionLookupMap;
               UserConnectionLookupIterator connIt = localMap.begin();
               while( connIt != localMap.end() )
               {
                  UserLoginMapIterator userIt = FindUserConnection( connIt->first );
                  if( userIt != m_userList.end() )
                  {
                     U32 connectionId, gatewayId;
                     if( userIt->second->GetConnectionInfo( gameProductId, connectionId, gatewayId ) )
                     {
                        ForceUserLogoutAndBlock( connectionId, gatewayId );
                     }
                  }
                  connIt ++;
               }
            }
            break;
         }
      }
      break;
   }
   return false;
}

//////////////////////////////////////////////////////////////////////////

void     LoginMainThread:: LogUserIn( const PacketLoginFromGateway* loginPacket, U32 gatewayId, U32 userConnectionId, U8 gameProductId )
{
   // this could still be a new user or an existing user from a different device
   const string& userName = Trim( loginPacket->userName ); 
   const string& password = Trim( loginPacket->password );
   const string& loginKey = Trim( loginPacket->loginKey );

   if( m_printFunctionNames )
   {
      cout << "fn: " << __FUNCTION__ << endl;

      LogMessage( LOG_PRIO_INFO, "***********************" );
      LogMessage( LOG_PRIO_INFO, "attempt to login user: %s, pwHash:%s for game id=%d and conn: %d", userName.c_str(), password.c_str(), gameProductId, userConnectionId );
      LogMessage( LOG_PRIO_INFO, "***********************" );
   }

   stringhash nameHash( GenerateUniqueHash( loginPacket->userName ) );
   LogMessage( LOG_PRIO_INFO, "User login " );
   LogMessage( LOG_PRIO_INFO, "    User: %s", userName.c_str() );
   //LogMessage( LOG_PRIO_INFO, "    uuid: %s", loginPacket->uuid.c_str() );
   LogMessage( LOG_PRIO_INFO, "    pass: %s", password.c_str() );
   
   LogMessage( LOG_PRIO_INFO, "--------------------------------------" );
   if( gameProductId == 0 )
   {
      LogMessage( LOG_PRIO_ERR, "User logging in without a product id" );
   }
   if( loginPacket->gatewayId == 0 )
   {
      LogMessage( LOG_PRIO_ERR, "User logging in without a gateway id" );
   }

   LoginMainThread::UserLoginMapIterator userIt = FindUserConnection( userConnectionId );
   if( userIt == m_userList.end() )
   {
      userIt = AddUserConnection( userName, password, userConnectionId );
   }

   if( userIt == m_userList.end() )
   {
      cout << "Error finding user connection" << endl;
      assert( 0 );
   }

   UserLogin* user = userIt->second; 
   if( user->GetPassword() == password )
   {
      user->HandlePacket( loginPacket, userConnectionId, gatewayId, gameProductId );
   }
   else
   {
      ForceUserLogoutAndBlock( userConnectionId, gatewayId );
   }   
}

//////////////////////////////////////////////////////////////////////////

bool  LoginMainThread::LoadUserWithoutLogin( const string& userName, U32 userConnectionIdRequester )
{
   UserLoginMapIterator it = FindUser( userName );
   if( it != m_userList.end() )
   {
      UserLoginMapIterator requesterIt = FindUserConnection( userConnectionIdRequester );
      if( requesterIt != m_userList.end())
      {
         U32 gatewayId = requesterIt->second->GetGatewayId( userConnectionIdRequester );
         it->second->PackUserProfile_SendToClient( userConnectionIdRequester, gatewayId );
      }
   }
   else
   {
      U32 id = m_temporaryConnectionId++;
      string password = userName;
      UserLoginMapIterator userIt = AddUserConnection( userName, password, id );
      UserLogin* user = userIt->second;
      user->SetExternalLookups( id, userConnectionIdRequester );
      user->LoadUserAccount_SendingProfileToOtherUser( userName, id );
   }
   return true;
}

//---------------------------------------------------------------

bool  LoginMainThread:: LogoutListOfUsers( const PacketLogin_LogoutAllUsers* usersLogout, U32 gatewayId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   int num = usersLogout->connectionIds.size();
   for( int i=0; i<num; i++ )
   {
      U32 connectionId = usersLogout->connectionIds[i];
      ForceUserLogoutAndBlock( connectionId, gatewayId );
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////   utilities   /////////////////////////////
//////////////////////////////////////////////////////////////////////////

LoginMainThread::UserLoginMapIterator     
   LoginMainThread:: AddUserConnection( const string& userName, const string& password, U32 userConnectionId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   
   stringhash nameHash( GenerateUniqueHash( userName ) );
   UserLoginMapIterator returnValue = m_userList.find( nameHash );
   if( returnValue ==  m_userList.end() )
   {
      UserLogin* user = new UserLogin( userName, password );
      pair< UserLoginMapIterator, bool > insertedItem = m_userList.insert( UserLoginPair( nameHash, user ) );
      assert( insertedItem.second == true );
      returnValue = insertedItem.first; 
   }
    
   returnValue->second->SetManager( this );
   returnValue->second->SetHashLookup( nameHash );

   if( userConnectionId )// only store a real connection. This allows other users to look you up without you being online.
   {
      m_userConnectionLookupMap.insert( UserConnectionLookupPair( userConnectionId, nameHash ) );
   }

   //userPair.second.SetManager( this );
   return returnValue;
}

//////////////////////////////////////////////////////////////////////////

bool  LoginMainThread:: IsUserConnectionValid( U32 userConnectionId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   UserConnectionLookupIterator connIt = m_userConnectionLookupMap.find( userConnectionId );
   if( connIt != m_userConnectionLookupMap.end() )
   {
      return true;
   }
   return false;
}

LoginMainThread::UserLoginMapIterator     
   LoginMainThread:: FindUserConnection( U32 userConnectionId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   UserConnectionLookupIterator connIt = m_userConnectionLookupMap.find( userConnectionId );
   if( connIt == m_userConnectionLookupMap.end() )
   {
      return m_userList.end();
   }

   stringhash nameHash = connIt->second;

   UserLoginMapIterator userIt = m_userList.find( nameHash ) ;
   return userIt;

  /* stringhash nameHash( GenerateUniqueHash( loginPacket->userName ) );
   UserLoginMapIterator iter = m_userList.insert( UserLoginPair( nameHash, UserLogin( userName, password ) ) );
   m_userConnectionLookupMap.insert( UserConnectionLookupPair( userConnectionId, nameHash ) );

   iter->second.SetManager( this );
   return iter;*/
}

//////////////////////////////////////////////////////////////////////////

bool     LoginMainThread:: RemoveUserConnection( U32 userConnectionId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   UserConnectionLookupIterator connIt = m_userConnectionLookupMap.find( userConnectionId );
   if( connIt == m_userConnectionLookupMap.end() )
   {
      return false;
   }

  /* stringhash nameHash = connIt->second;

   UserLoginMapIterator userIt = m_userList.find( nameHash ) ;
   return userIt;*/
   m_userConnectionLookupMap.erase( connIt );
   return true;
}

//////////////////////////////////////////////////////////////////////////

LoginMainThread::UserLoginMapIterator     
   LoginMainThread:: FindUserByUuid( const string& uuid )
{
   UserLoginMapIterator it = m_userList.begin();
   while( it != m_userList.end() )
   {
      UserLogin* user = (it->second);
      if( user->GetUuid() == uuid )
      {
         return it;
      }
      it++;
   }
   return m_userList.end();
}

//////////////////////////////////////////////////////////////////////////

LoginMainThread::UserLoginMapIterator
   LoginMainThread::FindUser( const string& userName )
{
   stringhash nameHash( GenerateUniqueHash( userName ) );

   UserLoginMapIterator userIt = m_userList.find( nameHash ) ;
   return userIt;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////// database related ////////////////////////////
//////////////////////////////////////////////////////////////////////////

void     LoginMainThread::UpdateDbResults()
{
   PacketFactory factory;

   m_mutex.lock();
      deque< PacketDbQueryResult* > tempQueue = m_dbQueries;
      m_dbQueries.clear();
   m_mutex.unlock();

   if( tempQueue.size() )
   {
      if( m_printFunctionNames == true )
      {
         cout << "fn: " << __FUNCTION__ << endl;
      }
   }

   deque< PacketDbQueryResult* >::iterator it = tempQueue.begin();
   while( it != tempQueue.end() )
   {
      PacketDbQueryResult* result = *it++;
    /*  if( result->customData != NULL )
            cout << "UpdateDbResults: Non-null custom data " << endl;*/
      BasePacket* packet = static_cast<BasePacket*>( result );

      HandleDbResult( result );
      factory.CleanupPacket( packet );
   }
}

bool  LoginMainThread:: UserAccountCreationHandler( U32 userConnectionId, const PacketDbQueryResult* dbResult )
{
   if( dbResult->lookup != QueryType_CreateAccountResults )
      return false;

   //return m_accountCreator->HandleResult( dbResult );
   return false;
}

//////////////////////////////////////////////////////////////////////////

bool     LoginMainThread:: HandleDbResult( const PacketDbQueryResult* dbResult )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   U32 userConnectionId = dbResult->id;
   if( userConnectionId && UserAccountCreationHandler( userConnectionId, dbResult ) == true )
      return true;
   

   bool  wasHandled = false;
   if( m_stringLookup->HandleResult( dbResult ) == true )
   {
      wasHandled = true;
   }
   else if( m_outageReader->HandleResult( dbResult ) == true )
   {
      wasHandled = true;
   }
   else
   {
      switch( dbResult->lookup )
      {
         cout << "Db query type:"<< dbResult->lookup << ", success=" << dbResult->successfulQuery << endl;

      case QueryType_UserQuery:
         {
            LoginMainThread::UserLoginMapIterator iter = FindUserConnection( userConnectionId );
            wasHandled = iter->second->HandleQueryResult( dbResult );
         }
         break;
      case QueryType_Products:
         {
            wasHandled = m_productManager->HandleQueryResult( dbResult );
         }
         break;
   /*   case QueryType_CreateAccountResults:
         {
         };*/
    /*  case QueryType_LoadProductInfo:
         {
            if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )
            {
               string str = "Initialization failed: table does not exist ";
               wasHandled = false;
            }
            else
            {
               StoreAllProducts( dbResult );
               wasHandled = true;
            }
         }
         break;
      case QueryType_GetSingleProductInfo:
         {
            if( dbResult->successfulQuery == false ||
               dbResult->GetBucket().size() == 0 )
            {
               string str = "Product not found ";
               str += dbResult->meta;
               LogMessage( LOG_PRIO_ERR, str.c_str() );
               wasHandled = false;
            }
            else
            {
               StoreSingleProduct( dbResult );
               wasHandled = true;
            }
         }
         break;*/

    /*  case QueryType_UserLoginInfo:
         {
            if( HandleUserLoginResult( connectionId, dbResult ) == false )
            {
               SendErrorToClient( connectionId, connection->GetGatewayId(), PacketErrorReport::ErrorType_UserBadLogin );  
               string str = "User not valid and db query failed, userName: ";
               str += connection->m_userName;
               str += ", password: ";
               str += connection->m_passwordHash;
               str += ", connectionId: ";
               str += boost::lexical_cast< string> ( connectionId );
               str += ", gatewayId: ";
               str += boost::lexical_cast< string> ( connection->m_connectionDetails.gatewayId );
               
               cout << "Error:" << str << endl;
               cout << connection->m_passwordHash << endl;

               LogMessage( LOG_PRIO_ERR, str.c_str() );
               connection->ForceCleanupState();
               time_t currentTime;
               time( &currentTime );
               currentTime += normalLogoutExpireTime - 3;// give us 3 seconds to cleanup
               connection->SetLoggedOutTime( currentTime );
               ForceUserLogoutAndBlock( connectionId, connection->GetGatewayId() );
               wasHandled = false;
            }
            else
            {
               wasHandled = true;
            }
         }
         break;
      case QueryType_AdminRequestUserProfile:
         {
            // in some weird circustance, we could end up in an infinite loop here.
           // if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )// no records found
           // {
           //    connection->AddBlankUserProfile();
           // }
           // else
           // {
           //    HandleUserProfileFromDb( connectionId, dbResult );
           // }
            HandleAdminRequestUserProfile( connectionId, dbResult );
            wasHandled = true;
         }
         break;
      case QueryType_UserListOfGame:
         {
            if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )// no records found
            {
               string str = "List of games not valid db query failed, userName: ";
               str += connection->m_userName;
               str += ", uuid: ";
               str += connection->m_userUuid;
               LogMessage( LOG_PRIO_ERR, str.c_str() );
               ForceUserLogoutAndBlock( connectionId, connection->GetGatewayId() );
               wasHandled = false;
            }
            else
            {
               wasHandled = true;
            
               KeyValueVector             key_value_array;

               SimpleGameTable            enigma( dbResult->bucket );
               SimpleGameTable::iterator it = enigma.begin();
               
               while( it != enigma.end() )
               {
                  SimpleGameTable::row       row = *it++;
                  string name =              row[ SimpleGame::Column_name ];
                  string uuid =              row[ SimpleGame::Column_uuid ];

                  key_value_array.push_back( KeyValueString ( uuid, name ) );
               }

               SendListOfOwnedGamesToGameServers( connectionId, key_value_array );
            }
         }
         break;
      case QueryType_UserListOfUserProducts:
         {
            if( dbResult->successfulQuery == false )//|| dbResult->GetBucket().size() == 0 )
            {
               string str = "Query failed looking up a user products ";
               str += connection->m_userName;
               str += ", uuid: ";
               str += connection->m_userUuid;
               LogMessage( LOG_PRIO_ERR, str.c_str() );
               ForceUserLogoutAndBlock( connectionId, connection->GetGatewayId() );
               wasHandled = false;
            }
            else
            {
               StoreListOfUsersProductsFromDB( connectionId, dbResult );
               wasHandled = true;
            }
         }
         break;
      case QueryType_LookupUserByUsernameOrEmail:// these should never happen since these are handled elsewhere
      case QueryType_LookupTempUserByUsernameOrEmail:
      case QueryType_LookupUserNameForInvalidName:
      case QueryType_LookupUserByGkHash:
         {
            if( dbResult->successfulQuery == false || dbResult->GetBucket().size() == 0 )
            {
               string str = "Query failed looking up a user ";
               str += connection->m_userName;
               str += ", uuid: ";
               str += connection->m_userUuid;
               LogMessage( LOG_PRIO_ERR, str.c_str() );
               ForceUserLogoutAndBlock( connectionId, connection->GetGatewayId() );
               wasHandled = false;
            }
            else
            {
               wasHandled = true;
            }
         }
         break;
      
      case QueryType_GetProductListForUser:
         {
            SendListOfPurchasesToUser( connectionId, dbResult );
            wasHandled = true;
         }
         break;*/
      }
   }
   return wasHandled;
}

//////////////////////////////////////////////////////////////////////////

bool  LoginMainThread:: ForceUserLogoutAndBlock( U32 userConnectionId, U32 gatewayId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }
   
   LoginMainThread::UserLoginMapIterator userIt = FindUserConnection( userConnectionId );
   if( userIt == m_userList.end() )
   {
      SendErrorToClient( userConnectionId, gatewayId, PacketErrorReport::ErrorType_UserBadLogin );
      LogMessage( LOG_PRIO_ERR, "ForceUserLogoutAndBlock cannot find this connection" );
   }

   UserLogin& connection = *(userIt->second);

   string                     userName       ( connection.GetUsername() );
   string                     uuid           ( connection.GetUuid() );
   string                     lastLoginTime  ( connection.GetLastLoginTime() );
   string                     email          ( connection.GetEmail() );
   bool                       active         ( connection.GetIsActive() );
   string                     passwordHash   ( connection.GetPassword() );
   string                     userId         ( connection.GetId() );
   string                     assetServerKey ( connection.GetAssetServerKey() );
   int                        languageId     ( connection.GetLanguageId() );
   U8                         gameProductId = connection.GetProductId( userConnectionId );

   // now disconnect him/her
   PacketLoginToGateway* loginStatus = new PacketLoginToGateway();
   loginStatus->userName = userName;
   loginStatus->userEmail = email;
   loginStatus->uuid = uuid;
   loginStatus->lastLogoutTime = GetDateInUTC();
   loginStatus->loginKey = assetServerKey;
   loginStatus->languageId = languageId;

   loginStatus->wasLoginSuccessful = false;
   loginStatus->adminLevel = 0;
   loginStatus->gameProductId = gameProductId;
   
   SendPacketToGateway( loginStatus, userConnectionId, gatewayId );
   const bool isLoggedIn = false; 
   const bool wasDisconnectedByError = false;

   SendLoginStatusToOtherServers( userName, uuid, userConnectionId, gameProductId, lastLoginTime, active, email, passwordHash, userId, assetServerKey, languageId, isLoggedIn, wasDisconnectedByError,
                                             gatewayId );

   connection.Disconnect( userConnectionId );

   return true;
}

//////////////////////////////////////////////////////////////////////////

bool  LoginMainThread:: BroadcastLoginStatus( U32 userConnectionId, 
                                               U8 gameProductId, 
                                               bool isLoggedIn, 
                                               bool wasDisconnectedByError,
                                               U32 gatewayId )
{
   LoginMainThread::UserLoginMapIterator userIt = FindUserConnection( userConnectionId );
   if( userIt == m_userList.end() )
   {
      SendErrorToClient( userConnectionId, gatewayId, PacketErrorReport::ErrorType_UserBadLogin );
      LogMessage( LOG_PRIO_ERR, "BroadcastLoginStatus cannot find this connection" );
      return false;
   }

   UserLogin& connection = *( userIt->second );
   bool loginSuccess = connection.LoginFinishedSuccessfully( userConnectionId );

   SendLoginStatusTo_Non_GameServers( connection.GetUsername(), 
                                       connection.GetUuid(), 
                                       userConnectionId, 
                                       gameProductId, 
                                       connection.GetLastLoginTime(), 
                                       connection.GetIsActive(), 
                                       connection.GetEmail(), 
                                       connection.GetPassword(), 
                                       connection.GetId(), 
                                       connection.GetAssetServerKey(), 
                                       connection.GetLanguageId(), 
                                       isLoggedIn, 
                                       wasDisconnectedByError,
                                       gatewayId );

   LogMessage( LOG_PRIO_INFO, "--------------------------------------------" );
   LogMessage( LOG_PRIO_INFO, "User login:      %s", connection.GetUsername().c_str() );
   LogMessage( LOG_PRIO_INFO, "User uuid:       %s", connection.GetUuid().c_str() );
   LogMessage( LOG_PRIO_INFO, "lastLoginTime =  %s", connection.GetLastLoginTime().c_str() );
   LogMessage( LOG_PRIO_INFO, "--------------------------------------------" );
   
   connection.SendStatusToGateway( userConnectionId, loginSuccess );

   if( loginSuccess == true )
   {
      LogMessage( LOG_PRIO_INFO, "User connection success" );
      m_uniqueUsers.insert( connection.GetUuid() );

      //UpdateLastLoggedInTime( userConnectionId ); // update the user logged in time
      SendLoginStatusTo_GameServers( connection.GetUsername(), 
                                       connection.GetUuid(), 
                                       userConnectionId, 
                                       gameProductId, 
                                       connection.GetLastLoginTime(), 
                                       connection.GetIsActive(), 
                                       connection.GetEmail(), 
                                       connection.GetPassword(), 
                                       connection.GetId(), 
                                       connection.GetAssetServerKey(), 
                                       connection.GetLanguageId(), 
                                       isLoggedIn, 
                                       wasDisconnectedByError,
                                       gatewayId );
      m_numSuccessfulLogins++;
      //m_numRelogins ++;
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////

bool  LoginMainThread:: BroadcastLoginStatusSimple( U32 userConnectionId, 
                                               U8 gameProductId, 
                                               bool isLoggedIn, 
                                               bool wasDisconnectedByError,
                                               U32 gatewayId )
{
   LoginMainThread::UserLoginMapIterator userIt = FindUserConnection( userConnectionId );
   if( userIt == m_userList.end() )
   {
      SendErrorToClient( userConnectionId, gatewayId, PacketErrorReport::ErrorType_UserBadLogin );
      LogMessage( LOG_PRIO_ERR, "BroadcastLoginStatus cannot find this connection" );
      return false;
   }
   UserLogin& connection = *( userIt->second );

   SendLoginStatusTo_Non_GameServers( connection.GetUsername(), 
                                       connection.GetUuid(), 
                                       userConnectionId, 
                                       gameProductId, 
                                       connection.GetLastLoginTime(), 
                                       connection.GetIsActive(), 
                                       connection.GetEmail(), 
                                       connection.GetPassword(), 
                                       connection.GetId(), 
                                       connection.GetAssetServerKey(), 
                                       connection.GetLanguageId(), 
                                       isLoggedIn, 
                                       wasDisconnectedByError,
                                       gatewayId );

   LogMessage( LOG_PRIO_INFO, "--------------------------------------------" );
   LogMessage( LOG_PRIO_INFO, "User login:      %s", connection.GetUsername().c_str() );
   LogMessage( LOG_PRIO_INFO, "User uuid:       %s", connection.GetUuid().c_str() );
   LogMessage( LOG_PRIO_INFO, "lastLoginTime =  %s", connection.GetLastLoginTime().c_str() );
   LogMessage( LOG_PRIO_INFO, "--------------------------------------------" );
   
   connection.SendStatusToGateway( userConnectionId, isLoggedIn );

   LogMessage( LOG_PRIO_INFO, "User connection success" );
  

   SendLoginStatusTo_GameServers( connection.GetUsername(), 
                                    connection.GetUuid(), 
                                    userConnectionId, 
                                    gameProductId, 
                                    connection.GetLastLoginTime(), 
                                    connection.GetIsActive(), 
                                    connection.GetEmail(), 
                                    connection.GetPassword(), 
                                    connection.GetId(), 
                                    connection.GetAssetServerKey(), 
                                    connection.GetLanguageId(), 
                                    isLoggedIn, 
                                    wasDisconnectedByError,
                                    gatewayId );
   m_numSuccessfulLogins++;

   return true;
}

//////////////////////////////////////////////////////////////////////////
bool  LoginMainThread:: SendLoginStatus(  ChainType*  destinationServerPtr,
                                          const string& userName, 
                                          const string& userUuid, 
                                          U32 connectionId, 
                                          U8 gameProductId, 
                                          const string& lastLoginTime, 
                                          bool isActive, 
                                          const string& email, 
                                          const string& passwordHash, 
                                          const string& userId, 
                                          const string& loginKey,
                                          int languageId, 
                                          bool isLoggedIn, 
                                          bool wasDisconnectedByError,
                                          U32 gatewayId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   if( destinationServerPtr == NULL )
      return false;

   BasePacket* packetToSend = NULL;
   if( isLoggedIn )
   {
      PacketPrepareForUserLogin* prepareForUser = new PacketPrepareForUserLogin;
      prepareForUser->connectionId = connectionId;
      prepareForUser->userName = userName;
      prepareForUser->uuid = userUuid;
      prepareForUser->lastLoginTime = lastLoginTime;
      prepareForUser->gameProductId = gameProductId;

      prepareForUser->active = isActive;
      prepareForUser->email= email;
      string tempUserId = userId;
      if( userId.size() == 0 )
         tempUserId = "0";
      prepareForUser->userId = boost::lexical_cast<U32>( tempUserId );
      prepareForUser->password = passwordHash;
      prepareForUser->loginKey = loginKey;
      prepareForUser->languageId = languageId;
      prepareForUser->gatewayId = gatewayId;

      packetToSend = prepareForUser;
      
   }
   else
   {
      PacketPrepareForUserLogout* logout = new PacketPrepareForUserLogout;
      logout->uuid = userUuid;
      logout->connectionId = connectionId;
      logout->wasDisconnectedByError = wasDisconnectedByError;

      packetToSend = logout;
   }

   if( destinationServerPtr->AddOutputChainData( packetToSend, m_chainId ) == false )
   {
      delete packetToSend;
   }

   return true;
}

bool  LoginMainThread:: SendLoginStatusToOtherServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 userConnectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError,
                                                     U32 gatewayId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   //cout << "SendLoginStatusToOtherServers" << endl;
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      itOutputs++;

      if( outputPtr->GetChainedType() != ChainedType_OutboundSocketConnector )
      {
         continue;
      }
      SendLoginStatus( outputPtr, 
                        userName, 
                       userUuid, 
                       userConnectionId, 
                       gameProductId, 
                       lastLoginTime, 
                       isActive, 
                       email, 
                       passwordHash, 
                       userId, 
                       loginKey,
                       languageId, 
                       isLoggedIn, 
                       wasDisconnectedByError,
                       gatewayId );
   }

   if( isLoggedIn )
   {
      m_numSuccessfulLogins++;
   }

   return true;
}

//////////////////////////////////////////////////////////////////////////

bool  LoginMainThread:: SendLoginStatusTo_Non_GameServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 userConnectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError,
                                                     U32 gatewayId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   LogMessage( LOG_PRIO_ERR, "SendLoginStatusTo_Non_GameServers for user: %s", userName.c_str() );
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      itOutputs++;

      //cout << outputPtr->GetClassName() << endl;
      if( outputPtr->GetChainedType() != ChainedType_OutboundSocketConnector )
      {
         continue;
      }
      FruitadensLogin* login = static_cast< FruitadensLogin* >( outputPtr );
      if( login->IsGameServer() == true )
      {
         continue;
      }

      SendLoginStatus( outputPtr, 
                       userName, 
                       userUuid, 
                       userConnectionId, 
                       gameProductId, 
                       lastLoginTime, 
                       isActive, 
                       email, 
                       passwordHash, 
                       userId, 
                       loginKey,
                       languageId, 
                       isLoggedIn, 
                       wasDisconnectedByError,
                       gatewayId );
   }

   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << " exit " << endl;
   }

   return true;
}

//////////////////////////////////////////////////////////////////////////

bool  LoginMainThread:: SendLoginStatusTo_GameServers( const string& userName, 
                                                     const string& userUuid, 
                                                     U32 userConnectionId, 
                                                     U8 gameProductId, 
                                                     const string& lastLoginTime, 
                                                     bool isActive, 
                                                     const string& email, 
                                                     const string& passwordHash, 
                                                     const string& userId, 
                                                     const string& loginKey,
                                                     int languageId, 
                                                     bool isLoggedIn, 
                                                     bool wasDisconnectedByError,
                                                     U32 gatewayId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   LogMessage( LOG_PRIO_ERR, "SendLoginStatusTo_GameServers for user: %s", userName.c_str() );
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      itOutputs++;

      if( outputPtr->GetChainedType() != ChainedType_OutboundSocketConnector )
      {
         continue;
      }
      FruitadensLogin* login = static_cast< FruitadensLogin* >( outputPtr );      
      if( login->IsGameServer() == false )
      {
         continue;
      }
      SendLoginStatus( outputPtr, 
                        userName, 
                       userUuid, 
                       userConnectionId, 
                       gameProductId, 
                       lastLoginTime, 
                       isActive, 
                       email, 
                       passwordHash, 
                       userId, 
                       loginKey,
                       languageId, 
                       isLoggedIn, 
                       wasDisconnectedByError,
                       gatewayId );
   }

   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << " exit " << endl;
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////

bool     LoginMainThread:: SendPacketToOtherServer( BasePacket* packet, U32 connectionId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   if( GetGatewayId( connectionId ) != 0 )
   //UserLogin* connection = GetUserConnection( connectionId );
   //if( connection )
   {
      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );

         if( outputPtr->AddOutputChainData( packet, m_chainId ) == true )
         {
            // this will be cleaned up much later
            //PacketFactory factory;
            //factory.CleanupPacket( packet );  
            return true;
         }
         itOutputs++;
      }
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////

void     LoginMainThread:: SendListOfUserProductsToAssetServer( U32 userConnectionId, U32 gatewayId )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }
   LoginMainThread::UserLoginMapIterator userIt = FindUserConnection( userConnectionId );
   if( userIt == m_userList.end() )
   {
      SendErrorToClient( userConnectionId, gatewayId, PacketErrorReport::ErrorType_UserBadLogin );
      LogMessage( LOG_PRIO_ERR, "ForceUserLogoutAndBlock cannot find this connection" );
      return;
   }

   UserLogin& userLogin = *( userIt->second );
   //ConnectionToUser* connection = GetUserConnection( connectionId );
   //if( connection )
   {
      PacketListOfUserProductsS2S* packet = new PacketListOfUserProductsS2S;
      packet->uuid = userLogin.GetUuid();
      vector< string >::const_iterator it =  userLogin.GetProductVendorUuids().begin();
      while( it != userLogin.GetProductVendorUuids().end() )
      {
         ProductInfo productDefn;
         if( m_productManager->FindProductByLookupName( *it++, productDefn ) == true )
         {
            packet->products.insert( productDefn.vendorUuid );
         }
      }


      BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainType*  outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );

         if( outputPtr->AddOutputChainData( packet, m_chainId ) == true )
         {
            return;
         }
         itOutputs++;
      }

      delete packet;
   }
}

//---------------------------------------------------------------

bool     LoginMainThread:: AddQueryToOutput( PacketDbQuery* dbQuery )
{
   if( m_printFunctionNames )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

  /* ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( packet, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   delete packet;/// normally, we'd leave this up to the invoker to cleanup. 
   return false;*/

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

//////////////////////////////////////////////////////////////////////////

U32      LoginMainThread::GetGatewayId( U32 userConnectionId )
{
   UserLoginMapIterator userIt = FindUserConnection( userConnectionId );
   if( userIt == m_userList.end() )
   {
      return 0;
   }
   return userIt->second->GetGatewayId( userConnectionId );
}

//////////////////////////////////////////////////////////////////////////

bool     LoginMainThread::FixupLookupInfo( U32 userConnectionId, const string& userName, const string& userUuid )
{
   UserConnectionLookupIterator connIt = m_userConnectionLookupMap.find( userConnectionId );
   if( connIt == m_userConnectionLookupMap.end() )
   {
      return false;
   }

   stringhash oldNameHash = connIt->second;

   UserLoginMapIterator userIt = m_userList.find( oldNameHash ) ;
   UserLogin* user = NULL;
   if( userIt != m_userList.end() )
   {
      user = userIt->second;
      m_userList.erase( userIt ); // remove old entry
   }

   stringhash newNameHash( GenerateUniqueHash( userName ) );
   
   // fix up old reference
   connIt->second = newNameHash;
   m_userList.insert( UserLoginPair( newNameHash, user ) );

   return true;
}

//////////////////////////////////////////////////////////////////////////
////////////////////////// stats //////////////////////////////
//////////////////////////////////////////////////////////////////////////

void     LoginMainThread::TrackCountStats( StatTracking stat, float value, int sub_category )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   StatTrackingConnections::TrackCountStats( m_serverName, m_serverId, stat, value, sub_category );
}

//////////////////////////////////////////////////////////////////////////

void     LoginMainThread::RunHourlyStats()
{
   /*if( m_userConnectionMap.size() == 0 )
      return;*/

   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampHourlyStatServerStatisics ) >= timeoutHourlyStatisics ) 
   {
      if( m_printFunctionNames == true )
      {
         cout << "fn: " << __FUNCTION__ << endl;
      }
      m_timestampHourlyStatServerStatisics = ZeroOutMinutes( currentTime );

      //--------------------------------
      float numConnections = static_cast<float>( m_userList.size() );
      float totalNumSeconds = 0;

      UserLoginMapConstIterator nextIt = m_userList.begin();
      while( nextIt != m_userList.end() )
      {
         const UserLogin& user = *( nextIt->second );
         nextIt++;
         time_t loginTime = user.GetEarliestLoginTime();
         if( loginTime == 0 )
            loginTime = currentTime;

         totalNumSeconds += static_cast<float>( difftime( currentTime, loginTime ) );
      }

      totalNumSeconds += m_totalUserLoginSeconds;
      numConnections += m_totalNumLogouts;

      float averageNumSeconds = totalNumSeconds / numConnections;
      TrackCountStats( StatTracking_UserAverageTimeOnline, averageNumSeconds, 0 );
      TrackCountStats( StatTracking_UserTotalTimeOnline, totalNumSeconds, 0 );
      TrackCountStats( StatTracking_NumUsersOnline, numConnections, 0 );

      // if we decide to track these
      m_numFailedLogins = 0;
      m_numRelogins = 0;
      m_numSuccessfulLogins = 0;
      m_totalUserLoginSeconds = 0;
      m_totalNumLogouts = 0;
   }
}

//---------------------------------------------------------------

void     LoginMainThread::RunDailyStats()
{
   time_t currentTime;
   time( &currentTime );
   if( difftime( currentTime, m_timestampDailyStatServerStatisics ) >= timeoutDailyStatisics ) 
   {
      if( m_printFunctionNames == true )
      {
         cout << "fn: " << __FUNCTION__ << endl;
      }
      m_timestampDailyStatServerStatisics = ZeroOutHours( currentTime );

      float numUniqueUsers = 0;
      UserLoginMapConstIterator nextIt = m_userList.begin();
      while( nextIt != m_userList.end() )
      {
         const UserLogin& user = *( nextIt->second );
         nextIt++;
         if( user.IsLoggedIntoAProduct() == false )
         {
            numUniqueUsers ++;
         }
      }

      ClearOutUniqueUsersNotLoggedIn();

      if( numUniqueUsers == 0 )
         return;

      TrackCountStats( StatTracking_UniquesUsersPerDay, numUniqueUsers, 0 );
   }
}

//---------------------------------------------------------------

void     LoginMainThread::ClearOutUniqueUsersNotLoggedIn()
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }

   UserLoginMapConstIterator nextIt = m_userList.begin();
   while( nextIt != m_userList.end() )
   {
      UserLoginMapConstIterator currentIt = nextIt++;
      const UserLogin& user = *( currentIt->second );
      
      if( user.IsLoggedIntoAProduct() == false )
      {
         m_userList.erase( currentIt );
      }
   }
}

//////////////////////////////////////////////////////////////////////////

void     LoginMainThread:: RemoveOldConnections()
{
   Threading::MutexLock    locker( m_inputChainListMutex );
   UserLoginMapIterator it = m_userList.begin();

   time_t testTimer;
   time( &testTimer );

   while( it != m_userList.end() )
   {
      UserLoginMapIterator temp = it++;
      const UserLogin& connection = *( temp->second );
      time_t mostRecentLogoutTime = connection.GetLatestLogoutTime();
      if( mostRecentLogoutTime != 0 &&
         difftime( testTimer, mostRecentLogoutTime ) >= timeoutLogoutExpireTime )
      {
         SendExpirationToAllOtherServers( connection.GetUsername(), connection.GetUuid() );
         m_userList.erase( temp );
      }
   }
}

//////////////////////////////////////////////////////////////////////////

bool     LoginMainThread:: SendExpirationToAllOtherServers( const string& userName, const string& uuid )
{
   if( m_printFunctionNames == true )
   {
      cout << "fn: " << __FUNCTION__ << endl;
   }
   PacketFactory factory;
   // send this to every other listening server
   BaseOutputContainer::iterator itOutputs = m_listOfOutputs.begin();
   LogMessage( LOG_PRIO_ERR, "SendExpirationToAllOtherServers for user: %s, uuid: %s", userName.c_str(), uuid.c_str() );
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainType*  destinationServerPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      itOutputs++;

      if( destinationServerPtr->GetChainedType() != ChainedType_OutboundSocketConnector )
      {
         continue;
      }

      PacketLoginExpireUser* expirePacket = new PacketLoginExpireUser;
      expirePacket->userName = userName;
      expirePacket->uuid = uuid;

      if( destinationServerPtr->AddOutputChainData( expirePacket, m_chainId ) == false )
      {
         BasePacket* packet = static_cast<BasePacket*>( expirePacket );
         factory.CleanupPacket( packet );
      }
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////
///////////////////////////////// main loop //////////////////////////////
//////////////////////////////////////////////////////////////////////////

int      LoginMainThread:: CallbackFunction()
{
   /*// too chatty to be useful
   if( m_printFunctionNames == true )
   {
      cout << "fn: ClearOutUniqueUsersNotLoggedIn" << endl;
   }

   */
   CommonUpdate();

   if( m_isInitializing == true )
   {
      if( m_productManager == NULL )
         return 0;
      if( m_productManager->IsFinishedInitializing() == true )
      {
         m_isInitialized = true;
         m_isInitializing = false;
      }
      else
      {
         m_productManager->Init();
      }
   }

   time_t currentTime;
   time( &currentTime );
   if( m_stringLookup )
      m_stringLookup->Update( currentTime );

   CleanupOldClientConnections( "KhaanLogin" );
   UpdateAllConnections( "KhaanLogin" );

   UpdateInputPacketToBeProcessed();
   UpdateOutputPacketToBeProcessed();

   RemoveOldConnections();

   UpdatePendingGatewayPackets();

   if( m_outageReader )
      m_outageReader->Update();

   RunHourlyStats();
   RunDailyStats();

   UpdateDbResults();

   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   return 1;
}

//////////////////////////////////////////////////////////////////////////