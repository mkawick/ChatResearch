
#include "AssetCommon.h"
#include "AssetMainThread.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "../NetworkCommon/NetworkIn/DiplodocusTools.h"
#include "../NetworkCommon/Utils/StringUtils.h"

#include "AssetOrganizer.h"

#include <iostream>
#include <time.h>
#include <fstream> 

using namespace std;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

AssetMainThread::AssetMainThread( const string& serverName, U32 serverId ): 
                              ChainedType( serverName, serverId, 0,  ServerType_Contact ), 
                              m_assetOfAssetFileModificationTime( 0 )
{
   time( &m_checkForFileChangeTimestamp );
   SetSleepTime( 45 );
   m_dummyUser.SetMaxNumerOfAssetsReturnedPerCategory( 12 );
   m_dummyUser.SetServer( this );
}

AssetMainThread :: ~AssetMainThread()
{
   m_assetsByCategory.clear();
}
//---------------------------------------------------------------

void     AssetMainThread::ServerWasIdentified( IChainedInterface* khaan )
{
  /* BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
     ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );*/

   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   Khaan* localKhaan = static_cast< Khaan* >( khaan );
   localKhaan->AddOutputChainDataNoLock( packet );
   // this is not thread safe, but will be invoked from within the same thread.
   m_clientsNeedingUpdate.push_back( localKhaan->GetChainedId() );
}

void     AssetMainThread::InputRemovalInProgress( IChainedInterface * chainedInput )
{
   KhaanAsset* khaan = static_cast< KhaanAsset* >( chainedInput );

   SetupClientWaitingToBeRemoved( khaan );

   string currentTime = GetDateInUTC();
   string printer = "AssetMainThread::Client disconnection at time:" + currentTime + " from " + inet_ntoa( khaan->GetIPAddress().sin_addr );
   LogMessage( LOG_PRIO_ERR, printer.c_str() );

   LogMessage( LOG_PRIO_ERR, "** InputRemovalInProgress" );
}

//---------------------------------------------------------------

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
}

//////////////////////////////////////////////////////////////////////////

bool  FillInAssetOrganizer( string& line, AssetOrganizer& assetDictionary )
{
   vector< string > listOfStuff;

   splitOnFirstFound( listOfStuff, line );

   if( listOfStuff.size() != 0 )
   {
      const string& potentionalKey = ConvertStringToLower( listOfStuff[ 0 ] );
      const string& value = ConvertStringToLower( listOfStuff[ 1 ] );
      const string undecoratedValue = RemoveEnds( listOfStuff[ 1 ] );

      if( listOfStuff.size() == 2 )// simplest case
      {
         if( potentionalKey == "type" || potentionalKey == "name" )
         {
            assetDictionary.SetCategory( undecoratedValue.c_str() );
            if( assetDictionary.GetCategory().size() == 0 ) // bad id
               return true;
         }
         else if( potentionalKey == "path" || potentionalKey == "file" )
         {
            assetDictionary.SetPath( undecoratedValue );
            if( DoesFileExist( assetDictionary.GetPath() ) == true )
            {
               return true;
            }
            else
            {
               LogMessage( LOG_PRIO_INFO, "Invalid file :  %s", assetDictionary.GetPath().c_str() );
            }
         }
         else
         {
            return false; // no other usable keys
         }
      }
      else
      {
         return false; // no support for one or three things on a line.
      }
   }

   return false;
}

int  SetupAssetList( ifstream& infile, AssetMainThread::CategorizedAssetLists& categorizedAssetLists )
{
   AssetOrganizer ao;
   string line;
   // we are expecting two lines
   int numLines = 0;

   if( safeGetline( infile, line ) == 0 )// could be spaces, etc
      assert( 0 );
   FillInAssetOrganizer( line, ao );

   numLines ++;

   if( safeGetline( infile, line ) == 0 )// could be spaces, etc
      assert( 0 );
   FillInAssetOrganizer( line, ao );

   numLines ++;

   if( ao.GetPath().size() != 0 && ao.GetCategory().size() != 0 )
   {
      if( categorizedAssetLists.find( ao.GetCategory() ) != categorizedAssetLists.end() ) // we already have this category so move on.
         return 0;

      categorizedAssetLists.insert( AssetMainThread::CategorizedAssetPair( ao.GetCategory(), ao ) );
   }
   else
   {
    //  assert( 0 );
   }

   return numLines;
}
//////////////////////////////////////////////////////////////////////////

bool  LoadListOfFiles( const string& assetManifestFile, AssetMainThread::CategorizedAssetLists& categorizedAssetLists )
{
   //ifstream infile( assetDictionary.dictionaryPath.c_str() );
   ifstream infile( assetManifestFile.c_str() );
   string line;
   if (!infile) 
   { 
      LogMessage( LOG_PRIO_ERR, "Error opening file!" ); 
      LogMessage( LOG_PRIO_ERR, "file not found: %s", assetManifestFile.c_str() ); 
      return false; 
   }

   int lineCount = 0;
   const char* bracketPairs = "[]{}<>()";
   AssetMainThread::CategorizedAssetLists:: reverse_iterator it = categorizedAssetLists.rend();

   while( safeGetline( infile, line ) )// could be spaces, etc
   {
      lineCount ++;
      if( line.size() == 0 )
         continue;
      if( IsBracketedTag( line, bracketPairs ) ) // start a new asset or item
      {
         string tag = RemoveEnds( line, bracketPairs );
         LogMessage( LOG_PRIO_ERR, "INVALID Tag in assets of assets file: %s", tag.c_str() );
         assert( tag == "file" );

         // we'll keep reading after this
         int num = static_cast< int >( categorizedAssetLists.size() );
         lineCount += SetupAssetList( infile, categorizedAssetLists );
         if( num == categorizedAssetLists.size() )// nothing was added
         {
            //assert( 0 );
            continue;
         }

         it = categorizedAssetLists.rbegin();
         if( it == categorizedAssetLists.rend() )
         {
            assert( 0 );
         }
      }
      else
      {
         //assert( 0 );
         continue;
      }
   }
   return true;
}


//////////////////////////////////////////////////////////////////////////

bool     AssetMainThread::SetIniFilePath( const string& assetPath, const string& assetOfAssets )
{
   m_assetsByCategory.clear();
   m_mainAssetFilePath = AssembleFullPath( assetPath, assetOfAssets );

   return LoadAllAssetManifests();
   
}

//////////////////////////////////////////////////////////////////////////

bool     AssetMainThread::LoadAllAssetManifests()
{
   if( LoadListOfFiles( m_mainAssetFilePath, m_assetsByCategory ) == false )
      return false;

   m_assetOfAssetFileModificationTime = GetFileModificationTime( m_mainAssetFilePath );
   if( m_assetOfAssetFileModificationTime == 0 )
      return false;

   if( m_assetsByCategory.size() == 0 )
      return false;

   int errorCode = 0;

   CategorizedAssetLists::iterator it = m_assetsByCategory.begin();
   while( it != m_assetsByCategory.end() )
   {
      AssetOrganizer& assetDictionary = it->second;
      it++;
      if( assetDictionary.IsFullyLoaded() == false )
      {
         assetDictionary.LoadAssetManifest();
      }
   }

   return true;
}

//---------------------------------------------------------------

bool     AssetMainThread::AddInputChainData( BasePacket* packet, U32 gatewayId )
{
   m_mutex.lock();
   m_inputPacketsToBeProcessed.push_back( PacketStorage( packet, gatewayId ) );
   m_mutex.unlock();

   return true;
}
//---------------------------------------------------------------

bool     AssetMainThread::ProcessPacket( PacketStorage& storage )
{
   BasePacket* packet = storage.packet;
   U32 gatewayId = storage.gatewayId;

   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );
      return HandleCommandFromGateway( packet, gatewayId );
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, gatewayId );
      return true;
   }

   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketCleaner cleaner( packet );
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32 connectionId = wrapper->connectionId;
      
      if( unwrappedPacket->packetType == PacketType_Asset)
      {
         int type = unwrappedPacket->packetSubType;
         string uuid;
         string loginKey;

         switch( type )
         {
            case PacketAsset::AssetType_GetListOfAssetCategories:
            {
               PacketAsset_GetListOfAssetCategories* packetAsset = static_cast< PacketAsset_GetListOfAssetCategories* >( unwrappedPacket );
               uuid = packetAsset->uuid.c_str();
               loginKey = packetAsset->loginKey.c_str();
            }
            break;
         case PacketAsset::AssetType_GetListOfAssets:
            {
               PacketAsset_GetListOfAssets* packetAsset = static_cast< PacketAsset_GetListOfAssets* >( unwrappedPacket );
               uuid = packetAsset->uuid.c_str();
               loginKey = packetAsset->loginKey.c_str();
            }
            break;
         case PacketAsset::AssetType_RequestAsset:
            {
               PacketAsset_RequestAsset* packetAsset = static_cast< PacketAsset_RequestAsset* >( unwrappedPacket );
               uuid = packetAsset->uuid.c_str();
               loginKey = packetAsset->loginKey.c_str();
            }
            break;
         case PacketAsset::AssetType_EchoToServer:
            {
               PacketAsset_EchoToServer* packetEcho = static_cast< PacketAsset_EchoToServer* >( unwrappedPacket );
               uuid = packetEcho->uuid.c_str();
               loginKey = packetEcho->loginKey.c_str();
            }
            break;
         }
         if( uuid.size() )
         {
            U64 userHash = GenerateUniqueHash( uuid );
            Threading::MutexLock locker( m_mutex );
            UAADMapIterator found = m_userTickets.find( userHash );
            if( found == m_userTickets.end() )
            {
               LogMessage( LOG_PRIO_INFO, "Request received but user not found: connId:%d, gatewayId:%d, uuid:%s, numUsersInList:%d", connectionId, gatewayId, uuid.c_str(), m_userTickets.size() );
               return true;
            }
            if( found->second.LoginKeyMatches( loginKey ) == false )
            {
               SendErrorToClient( connectionId, gatewayId, PacketErrorReport::ErrorType_Asset_BadLoginKey );
            }
            else
            {
               found->second.SetConnectionId( connectionId );
               found->second.HandleRequestFromClient( static_cast< PacketAsset* >( unwrappedPacket ) );
            }
         }
         else
         {
            // we have a user who has not logged in yet (or may never login)
            // we'll use a generic user to handle all of the filtering.
            Threading::MutexLock locker( m_mutex );
            m_dummyUser.SetConnectionId( connectionId );
            m_dummyUser.HandleRequestFromClient( static_cast< PacketAsset* >( unwrappedPacket ) );
         }
         
        // we will cleanup here... see cleaner
         return true;
      }
      else
      {
         assert( 0 );
      }
      // we handle all packets from the gateway here.
      return true;
   }
   return false;
}

//---------------------------------------------------------------

bool  AssetMainThread::HandlePacketFromOtherServer( BasePacket* packet, U32 gatewayId )// not thread safe
{
   if( packet->packetType != PacketType_ServerJobWrapper )
   {
      return false;
   }

   PacketServerJobWrapper* wrapper = static_cast< PacketServerJobWrapper* >( packet );
   BasePacket* unwrappedPacket = wrapper->pPacket;
   U32  serverIdLookup = wrapper->serverId;
   serverIdLookup = serverIdLookup;

   bool success = false;

   if( unwrappedPacket->packetType == PacketType_Login )
   {
      switch( unwrappedPacket->packetSubType )
      {
      case PacketLogin::LoginType_PrepareForUserLogin:
         ConnectUser( static_cast< const PacketPrepareForUserLogin* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_PrepareForUserLogout:
         DisconnectUser( static_cast< const PacketPrepareForUserLogout* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_ListOfProductsS2S:
         StoreUserProductsOwned( static_cast< const PacketListOfUserProductsS2S* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_ExpireUserLogin:
         ExpireUser( static_cast< const PacketLoginExpireUser* >( unwrappedPacket ) );
         return true;

      case PacketLogin::LoginType_RequestServiceToFlushAllUserLogins:
         DeleteAllUsers();
         return true;
      }
   }

   return false;
}

//---------------------------------------------------------------

U32      AssetMainThread::GetServerIdOfConnectedGateway()
{
   Threading::MutexLock locker( m_inputChainListMutex );

   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainType* inputPtr = static_cast< ChainType*> ( (*itInputs).m_interface );
      if( inputPtr->GetChainedType() == ChainedType_InboundSocketConnector )
      {
         KhaanAsset* khaan = static_cast< KhaanAsset* >( inputPtr );
         return khaan->GetServerId();
      }
      itInputs++;
   }

   return 0;
}

//---------------------------------------------------------------

bool     AssetMainThread::ConnectUser( const PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   string uuid = loginPacket->uuid;
   U32 gatewayId = GetServerIdOfConnectedGateway();//loginPacket->gatewayId;
   LogMessage_LoginPacket( loginPacket );
            
   U64 hashForUser = GenerateUniqueHash( loginPacket->uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it != m_userTickets.end() )// user may be reloggin and such.. no biggie.. just ignore
   {
      it->second.ClearLoggedOutStatus();
      it->second.SetConnectionId( connectionId );
      it->second.SetGatewayId( gatewayId );
      return false;
   }

   bool found = false;
   if( found == false )
   {

      UserTicket ut;
      ut.userName =        loginPacket->userName.c_str();
      ut.uuid =            loginPacket->uuid.c_str();
      ut.userTicket =      loginPacket->loginKey.c_str();
      ut.gatewayId =       gatewayId;
      ut.connectionId =    connectionId;
      ut.gameProductId =   loginPacket->gameProductId;
      ut.userId =          loginPacket->userId;

      UserAccountAssetDelivery user( ut );
      user.SetServer( this );

      //m_mutex.lock(); // locked from above
      m_userTickets.insert( UAADPair( hashForUser, user ) );
      //m_mutex.unlock();

      LogMessage( LOG_PRIO_INFO, "Login: User: %s, uuid: %s, gatewayId: %d, userId: %d, connId: %d", ut.userName.c_str(), ut.uuid.c_str(), ut.gatewayId, ut.userId, ut.connectionId );
   }
   return true;
}

//---------------------------------------------------------------

bool     AssetMainThread::DisconnectUser( const PacketPrepareForUserLogout* logoutPacket )
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

   it->second.UserLoggedOut();
   // we need to send notifications

   return true;
}


//---------------------------------------------------------------

bool  AssetMainThread::ExpireUser( const PacketLoginExpireUser* expirePacket )
{
   string uuid = expirePacket->uuid;
   U64 hashForUser = GenerateUniqueHash( uuid );
   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it == m_userTickets.end() )
      return false;

   m_userTickets.erase( it );

   return true;
}

//---------------------------------------------------------------

bool  AssetMainThread::DeleteAllUsers()
{
   Threading::MutexLock locker( m_mutex );
   m_userTickets.clear();
   return true;
}

//---------------------------------------------------------------
//---------------------------------------------------------------

bool     AssetMainThread::StoreUserProductsOwned( const PacketListOfUserProductsS2S* productNamesPacket )
{
   string uuid = productNamesPacket->uuid;
   U64 hashForUser = GenerateUniqueHash( uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it == m_userTickets.end() )
      return false;

   it->second.SetupProductFilterNames( productNamesPacket->products );

   return true;
}

//---------------------------------------------------------------

// make sure to follow the model of the account server regarding queries. Look at CreateAccount
// 
bool     AssetMainThread::AddOutputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      if( m_connectionIdGateway == 0 )
         return false;

      Threading::MutexLock locker( m_inputChainListMutex );
      //BaseOutputContainer localInputs = m_listOfInputs;

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainType* inputPtr = static_cast< ChainType*> ( (*itInputs).m_interface );
         if( inputPtr->GetChainedType() == ChainedType_InboundSocketConnector )
         {
            KhaanAsset* khaan = static_cast< KhaanAsset* >( inputPtr );
            if( khaan->GetServerId() == m_connectionIdGateway )
            {
               khaan->AddOutputChainData( packet );
               //khaan->Update();// the gateway may not have a proper connection id.

               MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );
               return true;
            }
         }
         itInputs++;
      }
      return false;
   }

   return false;
}

//---------------------------------------------------------------
void     AssetMainThread::ExpireOldConnections()
{
   Threading::MutexLock locker( m_mutex );
   if( m_userTickets.size() )
   {
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
}

//---------------------------------------------------------------

int      AssetMainThread::CallbackFunction()
{
   CleanupOldClientConnections( "KhaanAsset" );

   ExpireOldConnections();

   UpdateAllConnections( "KhaanAsset" );

   UpdateInputPacketToBeProcessed();

   CategorizedAssetLists::iterator it = m_assetsByCategory.begin();
   while( it != m_assetsByCategory.end() )
   {
      AssetOrganizer& assetDictionary = it->second;
      it++;
      //if( assetDictionary.IsFullyLoaded() == false )

      assetDictionary.Update();
   }

   time_t currentTime;
   time( &currentTime );
   if( m_assetOfAssetFileModificationTime && 
      difftime( currentTime, m_checkForFileChangeTimestamp ) >= CheckForFileModificationTimeout )  
   {
      m_checkForFileChangeTimestamp = currentTime;

      time_t fileTime = GetFileModificationTime( m_mainAssetFilePath );
      if( fileTime != m_assetOfAssetFileModificationTime ) // m_mainAssetFilePath
      {
         LoadAllAssetManifests();
      }
   }

   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, m_connectedClients.size(), m_listeningPort, m_serverName );
   // check for new friend requests and send a small list of notifications

   return 1;
}

//---------------------------------------------------------------

const AssetOrganizer*   AssetMainThread::GetAssetOrganizer( const string& AssetCategory ) const
{
   CategorizedAssetLists::const_iterator it = m_assetsByCategory.find( AssetCategory );
   if( it != m_assetsByCategory.end() )
      return &it->second;

   return NULL;
}

//---------------------------------------------------------------

bool                    AssetMainThread::GetListOfAssetCategories( vector<string>& categories ) const
{
   CategorizedAssetLists::const_iterator it = m_assetsByCategory.begin();
   while( it != m_assetsByCategory.end() )
   {
      categories.push_back( it->first );
      it++;
   }
   return true;
}

//---------------------------------------------------------------

const AssetDefinition*  AssetMainThread::GetAsset( const string& hash ) const
{
   const AssetDefinition* assetDefn = NULL;
   CategorizedAssetLists::const_iterator it = m_assetsByCategory.begin();
   while( it != m_assetsByCategory.end() )
   {
      const AssetOrganizer& assetDictionary = it->second;
      assetDictionary.FindByHash( hash, assetDefn );
      if( assetDefn != NULL )
         return assetDefn;
      it++;
   }
   return NULL;
}

//---------------------------------------------------------------
//---------------------------------------------------------------