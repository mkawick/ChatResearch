
#include "AssetCommon.h"
#include "DiplodocusAsset.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/AssetPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "AssetOrganizer.h"

#include <iostream>
#include <time.h>
#include <fstream> 

using namespace std;

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusAsset::DiplodocusAsset( const string& serverName, U32 serverId ): Diplodocus< KhaanAsset >( serverName, serverId, 0,  ServerType_Contact ), 
                                                                              m_assetOfAssetFileModificationTime( 0 )
{
   time( &m_checkForFileChangeTimestamp );
   SetSleepTime( 45 );
   m_dummyUser.SetMaxNumerOfAssetsReturnedPerCategory( 12 );
   m_dummyUser.SetServer( this );
}

DiplodocusAsset :: ~DiplodocusAsset()
{
   m_assetsByCategory.clear();
}
//---------------------------------------------------------------

void     DiplodocusAsset::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
  /* khaan->AddOutputChainData( packet, 0 );
   m_serversNeedingUpdate.push_back( static_cast<InputChainType*>( khaan )->GetServerId() );*/
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );

   //Threading::MutexLock locker( m_mutex );
   //m_serversNeedingUpdate.push_back( localKhaan->GetServerId() );
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
               cout << "Invalid file :  " << assetDictionary.GetPath() << endl;
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

int  SetupAssetList( ifstream& infile, DiplodocusAsset::CategorizedAssetLists& categorizedAssetLists )
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

      categorizedAssetLists.insert( DiplodocusAsset::CategorizedAssetPair( ao.GetCategory(), ao ) );
   }
   else
   {
    //  assert( 0 );
   }

   return numLines;
}
//////////////////////////////////////////////////////////////////////////

bool  LoadListOfFiles( const string& assetManifestFile, DiplodocusAsset::CategorizedAssetLists& categorizedAssetLists )
{
   //ifstream infile( assetDictionary.dictionaryPath.c_str() );
   ifstream infile( assetManifestFile.c_str() );
   string line;
   if (!infile) 
   { 
      std::cerr << "Error opening file!" << endl; 
      std::cout << "file not found: " << assetManifestFile << endl; 
      return false; 
   }

   int lineCount = 0;
   const char* bracketPairs = "[]{}<>()";
   DiplodocusAsset::CategorizedAssetLists:: reverse_iterator it = categorizedAssetLists.rend();

   while( safeGetline( infile, line ) )// could be spaces, etc
   {
      lineCount ++;
      if( line.size() == 0 )
         continue;
      if( IsBracketedTag( line, bracketPairs ) ) // start a new asset or item
      {
         string tag = RemoveEnds( line, bracketPairs );
         cout << "INVALID Tag in assets of assets file: " << tag << endl;
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
/*
int  LoadAssetOrganizerFile( AssetOrganizer& assetDictionary )
{
   ifstream infile( assetDictionary.GetPath().c_str() );
   string line;

   if (!infile) 
   { 
      std::cerr << "Error opening file!\n"; 
      return 1; 
   }

   int errorCode = 0;
   int lineCount = 0;
   while( safeGetline( infile, line ) )// could be spaces, etc
   {
      lineCount ++;
      if( line == "[asset]" )
      {
         bool result = ParseNextAsset( infile, lineCount, assetDictionary );
         if( result == false )
         {
            cout << "**********************************************" << endl;
            cout << "Error in asset file reading line " << lineCount << endl;
            cout << "**********************************************" << endl;
            errorCode = true;
            break;
         }
      }
   }
   return 0;
}*/

//////////////////////////////////////////////////////////////////////////

bool     DiplodocusAsset::SetIniFilePath( const string& assetPath, const string& assetOfAssets )
{
   m_assetsByCategory.clear();
   m_mainAssetFilePath = AssembleFullPath( assetPath, assetOfAssets );

   return LoadAllAssetManifests();
   
}

//////////////////////////////////////////////////////////////////////////

bool     DiplodocusAsset::LoadAllAssetManifests()
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

bool     DiplodocusAsset::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayInformation )
   {
      PacketCleaner cleaner( packet );
      return HandleCommandFromGateway( packet, connectionId );
   }

   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      PacketCleaner cleaner( packet );
      HandlePacketFromOtherServer( packet, connectionId );
      return true;
   }

   if( packet->packetType == PacketType_GatewayWrapper )
   {
      PacketCleaner cleaner( packet );
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      //U32 connectionIdToUse = wrapper->connectionId;
      
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
               uuid = packetAsset->uuid;
               loginKey = packetAsset->loginKey;
            }
            break;
         case PacketAsset::AssetType_GetListOfAssets:
            {
               PacketAsset_GetListOfAssets* packetAsset = static_cast< PacketAsset_GetListOfAssets* >( unwrappedPacket );
               uuid = packetAsset->uuid;
               loginKey = packetAsset->loginKey;
            }
            break;
         case PacketAsset::AssetType_RequestAsset:
            {
               PacketAsset_RequestAsset* packetAsset = static_cast< PacketAsset_RequestAsset* >( unwrappedPacket );
               uuid = packetAsset->uuid;
               loginKey = packetAsset->loginKey;
            }
            break;
         case PacketAsset::AssetType_EchoToServer:
            {
               PacketAsset_EchoToServer* packetEcho = static_cast< PacketAsset_EchoToServer* >( unwrappedPacket );
               uuid = packetEcho->uuid;
               loginKey = packetEcho->loginKey;
            }
            break;
         }
         if( uuid.size() )
         {
            U64 userHash = GenerateUniqueHash( uuid );
            Threading::MutexLock locker( m_mutex );
            UAADMapIterator found = m_userTickets.find( userHash );
            if( found == m_userTickets.end() )
               return true;
            if( found->second.LoginKeyMatches( loginKey ) == false )
            {
               SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Asset_BadLoginKey );
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

bool  DiplodocusAsset::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
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

   return false;
}

//---------------------------------------------------------------

bool     DiplodocusAsset::ConnectUser( PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;
   string uuid = loginPacket->uuid;
   cout << "Prep for logon: " << connectionId << ", " << loginPacket->userName << ", " << uuid << ", " << loginPacket->password << endl;

   U64 hashForUser = GenerateUniqueHash( loginPacket->uuid );

   Threading::MutexLock locker( m_mutex );
   UAADMapIterator it = m_userTickets.find( hashForUser );
   if( it != m_userTickets.end() )// user may be reloggin and such.. no biggie.. just ignore
   {
      it->second.SetConnectionId( 0 );
      return false;
   }

   bool found = false;
   if( found == false )
   {

      UserTicket ut;
      ut.userName =        loginPacket->userName;
      ut.uuid =            loginPacket->uuid.c_str();
      ut.userTicket =      loginPacket->loginKey;
      ut.connectionId =    0;
      ut.gameProductId =   loginPacket->gameProductId;
      ut.userId =          loginPacket->userId;

      UserAccountAssetDelivery user( ut );
      user.SetServer( this );

      //m_mutex.lock(); // locked from above
      m_userTickets.insert( UAADPair( hashForUser, user ) );
      //m_mutex.unlock();
   }
   return true;
}

//---------------------------------------------------------------

bool     DiplodocusAsset::DisconnectUser( PacketPrepareForUserLogout* loginPacket )
{
   cout << "Prep for logout: " << loginPacket->connectionId << ", " << loginPacket->uuid << endl;

   U32 connectionId = loginPacket->connectionId;
   connectionId = connectionId;
   string uuid = loginPacket->uuid;
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

bool     DiplodocusAsset::StoreUserProductsOwned( PacketListOfUserProductsS2S* productNamesPacket )
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
bool     DiplodocusAsset::AddOutputChainData( BasePacket* packet, U32 connectionId )
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
         ChainLink& chainedInput = *itInputs++;
         ChainedInterface* interfacePtr = static_cast<ChainedInterface*>( chainedInput.m_interface );
         if( interfacePtr->DoesNameMatch( "KhaanAsset" ) )
         {
            KhaanAsset* khaan = static_cast< KhaanAsset* >( interfacePtr );
            if( khaan->GetServerId() == m_connectionIdGateway )
            {
               khaan->AddOutputChainData( packet );
               //khaan->Update();// the gateway may not have a proper connection id.

               MarkConnectionAsNeedingUpdate( khaan->GetChainedId() );
               return true;
            }
         }
      }
      return false;
   }

   return false;
}

//---------------------------------------------------------------
void     DiplodocusAsset::ExpireOldConnections()
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

int      DiplodocusAsset::CallbackFunction()
{
   ExpireOldConnections();

   UpdateAllConnections();

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

const AssetOrganizer*   DiplodocusAsset::GetAssetOrganizer( const string& AssetCategory ) const
{
   CategorizedAssetLists::const_iterator it = m_assetsByCategory.find( AssetCategory );
   if( it != m_assetsByCategory.end() )
      return &it->second;

   return NULL;
}

//---------------------------------------------------------------

bool                    DiplodocusAsset::GetListOfAssetCategories( vector<string>& categories ) const
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

const AssetDefinition*  DiplodocusAsset::GetAsset( const string& hash ) const
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