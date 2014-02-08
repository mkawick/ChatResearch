#include "NetworkLayer.h"

#include "../ServerStack/NetworkCommon/Utils/Utils.h"
#include "../ServerStack/NetworkCommon/Packets/CheatPacket.h"
#include "../ServerStack/NetworkCommon/Packets/PacketFactory.h"
#include "../ServerStack/NetworkCommon/ChainedArchitecture/Thread.h"

#include <assert.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
using namespace Mber;

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning ( disable: 4996 )
#endif

///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////

AssetInfoExtended:: AssetInfoExtended() : data(NULL), size( NULL ), AssetInfo() 
{
}

AssetInfoExtended:: AssetInfoExtended( const AssetInfo& asset ) : data(NULL), size( NULL ), AssetInfo( asset ) 
{
}

AssetInfoExtended:: ~AssetInfoExtended()
{
   ClearData();
}

void  AssetInfoExtended:: SetData( const U8* _data, int _size )
{
   ClearData();

   if( _size == 0 || _data == NULL )
      return;

   size = _size;
   data = new U8[ size ];
   memcpy( data, _data, size );
}

void  AssetInfoExtended:: ClearData()
{
   if( data )
      delete [] data;
   data = NULL;
   size = 0;
}

void  AssetInfoExtended:: operator = ( const AssetInfo& asset )
{
   productId = asset.productId;
   assetHash = asset.assetHash;
   assetName = asset.assetName;
   version = asset.version;
   beginDate = asset.beginDate;
   endDate = asset.endDate;
   isOptional = asset.isOptional;
   category = asset.category;
}

void  AssetInfoExtended:: operator = ( const AssetInfoExtended& asset )
{
   productId = asset.productId;
   assetHash = asset.assetHash;
   assetName = asset.assetName;
   version = asset.version;
   beginDate = asset.beginDate;
   endDate = asset.endDate;
   isOptional = asset.isOptional;
   category = asset.category;

   SetData( asset.data, asset.size );
}

void  AssetInfoExtended:: MoveData( AssetInfoExtended& source )
{
   data = source.data; source.data = NULL;
   size = source.size; source.size = 0;
}

///////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------

NetworkLayer::NetworkLayer( U8 gameProductId, bool processOnlyOneIncommingPacketPerLoop  ) : Fruitadens( "Networking Layer", processOnlyOneIncommingPacketPerLoop ),
      m_gameProductId( gameProductId ), 
      m_isLoggingIn( false ),
      m_isLoggedIn( false ),
      m_connectionId( 0 ), 
      m_selectedGame( 0 ),
      m_normalSleepTime( 50 ),
      m_boostedSleepTime( 8 ),
      m_isThreadPerformanceBoosted( 0 ),
      m_lastRawDataIndex( 0 ),
      m_connectionPort( 9600 )
{
   SetSleepTime( m_normalSleepTime );
   //m_serverDns = "chat.mickey.playdekgames.com";
   m_serverDns = "gateway.internal.playdekgames.com";
   InitializeSockets();
}

NetworkLayer::~NetworkLayer()
{
   Exit();
   ShutdownSockets();
}

//------------------------------------------------------------
//------------------------------------------------------------

void  NetworkLayer::CheckForReroutes( bool checkForReroutes )
{
   m_checkForReroute = checkForReroutes;
}

void  NetworkLayer::Init( const char* serverDNS )
{
   if( m_clientSocket != SOCKET_ERROR || m_isConnected == true )
   {
      Disconnect();
   }

   

   if( serverDNS && strlen( serverDNS ) > 5 )
   {
      m_serverDns = serverDNS;
   }
   Connect( m_serverDns.c_str(), m_connectionPort );
}

void  NetworkLayer::Exit()
{
   Disconnect();
   m_isLoggingIn = false;
   m_isLoggedIn = false;

   m_invitationsReceived.clear();
   m_invitationsSent.clear();
   m_assets.clear();
   m_availableTournaments.clear();
}

string   NetworkLayer::GenerateHash( const string& stringThatIWantHashed )
{
   string value;
   string lowerCaseString = ConvertStringToLower( stringThatIWantHashed );
   ::ConvertToString( ::GenerateUniqueHash( lowerCaseString ), value );
   if( value.size() > TypicalMaxHexLenForNetworking )// limit
   {
      value = value.substr( value.size()-TypicalMaxHexLenForNetworking, TypicalMaxHexLenForNetworking); 
   }
   return value;
}

//------------------------------------------------------------

static inline U64 CreatePasswordHash( const char *pPassword )
{
   char salted_password[256];
   sprintf( salted_password, "pd%-14s", pPassword );

   const char *pPasswordText = salted_password;

   U64 hash = 5381; // definitely a magic number
   U64 c;

   while( (c = *pPasswordText++) != 0 )
   {
      hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
   }

   return hash;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestLogin( const string& userName, const string& password, const string& languageCode )
{
   if( m_isConnected == false )
   {
      Init( NULL );
   }
   PacketLogin login;
   login.loginKey = "deadbeef";// currently unused
   login.uuid = userName;
   login.userName = userName;
   login.loginKey = password;
   login.languageCode = languageCode;
   login.password = boost::lexical_cast< string >( CreatePasswordHash( password.c_str() ) );
   m_attemptedUsername = userName;

   m_isLoggingIn = true;
   SerializePacketOut( &login );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestAccountCreate( const string& userName, const string& useremail, const string& password, int languageId, const string& deviceId, const string& gkHash )
{
   if( m_isConnected == false )
   {
      Init();
   }
   assert( m_isLoggedIn == false );

   PacketCreateAccount createAccount;
   createAccount.userName = userName;
   createAccount.useremail = useremail;
   createAccount.password = boost::lexical_cast< string >( CreatePasswordHash( password.c_str() ) );
   createAccount.deviceId = deviceId;
   createAccount.deviceAccountId = boost::lexical_cast< string >( CreatePasswordHash( gkHash.c_str() ) );
   createAccount.languageId = languageId;

   m_attemptedUsername = userName;

   m_isCreatingAccount = true;
   SerializePacketOut( &createAccount );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestLogout() const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketLogout logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestProfile( const string userName )//if empty, profile for currently logged in user is used
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketRequestUserProfile request;
   request.userName = userName;
   if( userName == "" )
      request.uuid = m_uuid;
   SerializePacketOut( &request );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestOtherUserInGameProfile( const string& userName ) // friends, games list, etc
{
   if( m_isConnected == false || userName.size() == 0 )
   {
      return false;
   }
   PacketRequestOtherUserProfile request;
   request.userName = userName;

   SerializePacketOut( &request );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::UpdateUserProfile( const string userName, const string& email, const string& userUuid, int adminLevel, int languageId, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile )
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketUpdateUserProfile       update;
   update.userName =             userName;
   update.email =                email;
   update.userUuid =             userUuid;
   
   update.adminLevel =           adminLevel;
   update.languageId =           languageId;
   update.isActive =             isActive;
   update.showWinLossRecord =    showWinLossRecord;
   update.marketingOptOut =      marketingOptOut;
   update.showGenderProfile =    showGenderProfile;

   SerializePacketOut( &update );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestListOfFriends() const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketContact_GetListOfContacts friends;
   SerializePacketOut( &friends );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestListOfProducts() const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketRequestListOfProducts products;
   SerializePacketOut( &products );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestListOfProductsInStore() const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketPurchase_RequestListOfSales products;
   SerializePacketOut( &products );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestListOfPurchases( const string userUuid ) const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketListOfUserPurchasesRequest purchases;
   if( userUuid.size() > 0 )
   {
      purchases.userUuid = userUuid;
   }
   else 
   {
      purchases.userUuid = m_uuid;
   }
   SerializePacketOut( &purchases );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::MakePurchase( const string& exchangeUuid ) const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketPurchase_Buy purchase;
   purchase.purchaseUuid = exchangeUuid;
   SerializePacketOut( &purchase );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::RequestListOfTournaments()
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketTournament_RequestListOfTournaments tournaments;
   SerializePacketOut( &tournaments );// only requests tournaments for the current game

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer::PurchaseEntryIntoTournament( const string& tournamentUuid )
{
   if( m_isConnected == false )
   {
      return false;
   }

   PacketTournament_UserRequestsEntryInTournament entry;
   entry.tournamentUuid = tournamentUuid;

   SerializePacketOut( &entry );

   return true;
}

//-----------------------------------------------------------------------------
  
bool  NetworkLayer::RequestChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex ) const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketChatHistoryRequest history;
   history.chatChannelUuid = channelUuid;
   history.numRecords = numRecords;
   history.startingIndex = startingIndex;
   //SerializePacketOut( &history );
   return true;
}

//-----------------------------------------------------------------------------
  
bool  NetworkLayer::RequestChatP2PHistory( const string& userUuid, int numRecords, int startingIndex ) const
{
   if( m_isConnected == false )
   {
      return false;
   }
   PacketChatHistoryRequest history;
   history.userUuid = userUuid;
   history.numRecords = numRecords;
   history.startingIndex = startingIndex;
   SerializePacketOut( &history );
   return true;
}



//-----------------------------------------------------------------------------
/*
bool  NetworkLayer::ChangeGame( const string& gameName )// either name or shortname
{
   U32 id = FindGame( gameName );
   if( id == 0 )
   {
      return false;
   }
   else
   {
      m_selectedGame = id;
   }
   return true;
}

//-----------------------------------------------------------------------------

U32   NetworkLayer::FindGame( const string& name ) const
{
   GameList::const_iterator it = m_gameList.begin();
   while( it != m_gameList.end() )
   {
      const PacketGameIdentification& gameId = *it++;
      if( gameId.name == name || gameId.shortName == name )
      {
         return gameId.gameId;
      }
   }
   return 0;
}

//-----------------------------------------------------------------------------

string   NetworkLayer::FindGameNameFromGameId( U32 id ) const
{
   GameList::const_iterator it = m_gameList.begin();
   while( it != m_gameList.end() )
   {
      const PacketGameIdentification& gameId = *it++;
      if( gameId.gameId == id )
      {
         return gameId.name;
      }
   }
   return 0;
}
*/
//-----------------------------------------------------------------------------

string   NetworkLayer::FindFriend( const string& name ) const 
{
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      const BasicUser&  kvpFriend = itFriends->value;
      
      if( kvpFriend.userName == name )
      {
         return itFriends->key;
      }
      itFriends++;
   }
   if( name == m_userName )// this method is commonly used as a lookup.. let's make it simple to use
      return m_uuid;

   return string();
}

//-----------------------------------------------------------------------------

string   NetworkLayer::FindFriendFromUuid( const string& uuid ) const 
{
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      if( itFriends->key == uuid )
      {
         return itFriends->value.userName;
      }
   }
   if( uuid == m_uuid )// this method is commonly used as a lookup.. let's make it simple to use
      return m_userName;

   return string();
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::GetFriend( int index, const BasicUser*& user )
{
   if( index < 0 || index >= m_friends.size() )
   {
      user = NULL;
      return false;
   }

   int i = 0;
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      if( i == index )
      {
         user = &itFriends->value;
         return true;
      }
      i ++;
      itFriends++;
   }

   return false;

}

//-----------------------------------------------------------------------------

bool    NetworkLayer::GetChannel( int index, ChatChannel& channel )
{
   if( index < 0 || index >= (int) m_channels.size() )
   {
      channel.Clear();
      return false;
   }

   int i = 0;
   vector< ChatChannel >::const_iterator itChannels = m_channels.begin();
   while( itChannels != m_channels.end() )
   {
      if( i == index )
      {
         channel = *itChannels;
         return true;
      }
      i ++;
      itChannels++;
   }

   return false;
}

//-----------------------------------------------------------------------------

int      NetworkLayer::GetAssetCategories( vector< string >& categories ) const
{
   AssetMapConstIter it = m_assets.begin();
   while( it != m_assets.end() )
   {
      categories.push_back( it->first );
      it++;
   }

   return categories.size();
}

//-----------------------------------------------------------------------------

int      NetworkLayer::GetNumAssets( const string& category )
{
   AssetMapConstIter it = m_assets.find( category );
   if( it == m_assets.end() )
      return 0;
   return it->second.size();
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::GetAssetInfo( const string& category, int index, AssetInfoExtended& assetInfo )
{
   AssetMapConstIter it = m_assets.find( category );
   if( it == m_assets.end() )
      return false;
   
   if( index < 0 || index >= static_cast< int >( it->second.size() ) )
      return false;

   assetInfo = it->second[ index ];
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer:: ClearAssetInfo( const string& category, const string& hash )
{
   AssetMapIter it = m_assets.find( category );
   if( it == m_assets.end() )
      return false;

   vector< AssetInfoExtended >& arrayOfAssets = it->second;
   vector< AssetInfoExtended >::iterator itAssets = arrayOfAssets.begin();
   while( itAssets != arrayOfAssets.end() )
   {
      if( itAssets->assetHash == hash )
      {
         itAssets->ClearData();
         return true;
      }
      itAssets++;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool    NetworkLayer::GetTournamentInfo( int index, TournamentInfo& tournamentInfo )
{
   if( index < 0 || index >= (int) m_availableTournaments.size() )
   {
      tournamentInfo.Clear();
      return false;
   }

   int i = 0;
   vector< TournamentInfo >::const_iterator itTournament = m_availableTournaments.begin();
   while( itTournament != m_availableTournaments.end() )
   {
      if( i == index )
      {
         tournamentInfo = *itTournament;
         return true;
      }
      i ++;
      itTournament++;
   }

   return false;
}


//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfGames() const
{
   PacketRequestListOfGames listOfGames;
   SerializePacketOut( &listOfGames );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfUsersLoggedIntoGame( ) const
{
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestFriendDemographics( const string& userName ) const
{
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestUserWinLossRecord( const string& userName ) const
{
   PacketRequestUserWinLoss packet;
   packet.userUuid;
   packet.gameUuid;
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfInvitationsSent() const
{
   PacketContact_GetListOfInvitationsSent    invitationRequest;
   return SerializePacketOut( &invitationRequest );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfInvitationsReceived() const
{
   PacketContact_GetListOfInvitations    invitationRequest;
   return SerializePacketOut( &invitationRequest );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::AcceptInvitation( const string& uuid ) const
{
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->key == uuid )
      {
         PacketContact_AcceptInvite invitationAccepted;
         invitationAccepted.invitationUuid = uuid;
        
         return SerializePacketOut( &invitationAccepted );
      }
      it++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::AcceptInvitationFromUsername( const string& userName ) const
{
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->value.inviterName == userName )
      {
         PacketContact_AcceptInvite invitationAccepted;
         invitationAccepted.invitationUuid = it->key;
        
         return SerializePacketOut( &invitationAccepted );
      }
      it++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::DeclineInvitation( const string& uuid, string message ) const
{
   PacketContact_DeclineInvitation invitationDeclined;
   invitationDeclined.invitationUuid = uuid;
   invitationDeclined.message = message;

   return SerializePacketOut( &invitationDeclined );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::GetListOfInvitationsSent( list< InvitationInfo >& listOfInvites )
{
   listOfInvites.clear();
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsSent;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      listOfInvites.push_back( it->value );
      it++;
   }
   return true;
}
//-----------------------------------------------------------------------------

bool     NetworkLayer::GetListOfInvitationsReceived( list< InvitationInfo >& listOfInvites )
{
   listOfInvites.clear();
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      listOfInvites.push_back( it->value );
      it++;
   }
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RegisterCallbackInterface( UserNetworkEventNotifier* _callbacks ) 
{
   for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
   {
      if( *it == _callbacks )
         return false;
   }
   
   m_callbacks.push_back( _callbacks );  

   _callbacks->network = this;
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SendRawPacket( const char* buffer, int length ) const
{
   PacketGameplayRawData raw;
   memcpy( raw.data, buffer, length );
   raw.size = length;

   return SerializePacketOut( &raw );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SendSearchForUsers( const string& searchString, int numRequested, int offset ) const // min 2 char
{
   if( searchString.size() < 2 )
      return false;

   assert( 0 ); // ContactType_Search
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::InviteUserToBeFriend( const string& uuid, const string& userName, const string& message )
{
   if( uuid.size() < 2 && userName.size() < 2 )
      return false;

   PacketContact_InviteContact invitation;
   invitation.userName = userName;
   invitation.uuid = uuid;
   invitation.message = message;

   return SerializePacketOut( &invitation );
}

//-----------------------------------------------------------------------------

bool	   NetworkLayer::SendP2PTextMessage( const string& message, const string& destinationUserUuid )
{
   PacketChatToServer chat;
   chat.message = message;
   chat.userUuid = destinationUserUuid;
   // chat.channelUuid; // not used
   SerializePacketOut( &chat );

   return true;
}

//-----------------------------------------------------------------------------

bool	   NetworkLayer::SendChannelTextMessage( const string& message, const string& chatChannelUuid, U32 gameTurn )
{
   PacketChatToServer chat;
   chat.message = message;
   chat.gameTurn = gameTurn;
   // chat.userUuid = m_uuid; // not used
   chat.channelUuid = chatChannelUuid;
   SerializePacketOut( &chat );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfAssetCategories()
{
   PacketAsset_GetListOfAssetCategories assetCategoryRequest;
   assetCategoryRequest.uuid = m_uuid;
   assetCategoryRequest.loginKey = m_loginKey;
   SerializePacketOut( &assetCategoryRequest );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestListOfAssets( const string& category, int platformId )
{
   PacketAsset_GetListOfAssets assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.platformId = platformId;
   assetRequest.assetCategory = category;

   SerializePacketOut( &assetRequest );
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestAssetByHash( const string& assetHash )
{
   AssetInfoExtended asset;
   if( GetAsset( assetHash, asset ) == false )
      return false;

   PacketAsset_RequestAsset assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.assetHash = assetHash;

   SerializePacketOut( &assetRequest );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::RequestAssetByName( const string& assetName )
{
   AssetInfoExtended asset;
   const string assetHash = GenerateHash( assetName );
   if( GetAsset( assetHash, asset ) == false )
      return false;

   PacketAsset_RequestAsset assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.assetHash = assetHash ;

   SerializePacketOut( &assetRequest );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SendPurchases( const vector< RegisteredProduct >& purchases, int platformId )
{
   PacketListOfUserAggregatePurchases packet;
   packet.platformId = platformId;

   vector< RegisteredProduct >::const_iterator it = purchases.begin();
   while( it != purchases.end() )
   {
      const RegisteredProduct& rp = *it++;
      
      PurchaseEntry pe;
      pe.productStoreId = rp.id;
      pe.name = rp.title;
      pe.number_price = rp.number_price;
      pe.price = rp.price;
      pe.date = GetDateInUTC();

      packet.purchases.push_back( pe );
   }

   SerializePacketOut( &packet );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::GiveProduct( const string& userUuid, const string& productUuid, int quantity, const string& notes, int platformId )
{
   PacketAddPurchaseEntry purchaseEntry;
   purchaseEntry.userName = userUuid;
   purchaseEntry.userEmail = userUuid;
   purchaseEntry.userUuid = userUuid;
   purchaseEntry.productUuid = productUuid;
   purchaseEntry.quantity = quantity;

   purchaseEntry.adminNotes  = notes;
   purchaseEntry.platformId = platformId;

   SerializePacketOut( &purchaseEntry );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SendCheat( const string& cheatText )
{
   PacketCheat cheat;
   cheat.cheat = cheatText;
   cheat.whichServer = ServerType_Login;// needs improvement
   SerializePacketOut( &cheat );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer::SerializePacketOut( BasePacket* packet ) const 
{
   U8 buffer[ MaxBufferSize ];
   int offset = 0;

   packet->gameInstanceId = m_selectedGame;
   packet->gameProductId = m_gameProductId;
   packet->SerializeOut( buffer, offset );
   m_beginTime = GetCurrentMilliseconds();
   return Fruitadens::SendPacket( buffer, offset );
}


//-----------------------------------------------------------------------------

int      NetworkLayer::ProcessInputFunction()
{
   ExpireThreadPerformanceBoost();
   return Fruitadens::ProcessInputFunction();
}

void     NetworkLayer::BoostThreadPerformance()
{
   SetSleepTime( m_boostedSleepTime );
   SetPriority( ePriorityHigh );
   m_isThreadPerformanceBoosted = true;
   time( &m_timeWhenThreadPerformanceBoosted );
}

void     NetworkLayer::RestoreNormalThreadPerformance()
{
   SetSleepTime( m_normalSleepTime );
   m_isThreadPerformanceBoosted = false;
   SetPriority( ePriorityNormal );
}

void     NetworkLayer::ExpireThreadPerformanceBoost()
{
   if( m_isThreadPerformanceBoosted )
   {
      time_t currentTime;// expire after 5 seconds.
      time( &currentTime );
      if( difftime( currentTime, m_timeWhenThreadPerformanceBoosted ) > 5.0 )
      {
         RestoreNormalThreadPerformance();
      }
   }
}
//-----------------------------------------------------------------------------

bool  NetworkLayer::HandlePacketReceived( BasePacket* packetIn )
{
   PacketCleaner cleaner( packetIn );

   switch( packetIn->packetType )
   {
      case PacketType_Base:
      {
         switch( packetIn->packetSubType )
         {
         case BasePacket::BasePacket_Hello:
            {
               PacketHello* packet = static_cast< PacketHello* >( packetIn );
               cout << "Server protocol version: " << (int)(packet->versionNumber) << endl;
               cout << "Client protocol version: " << (int)(GlobalNetworkProtocolVersion) << endl;

               for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->AreWeUsingCorrectNetworkVersion( GlobalNetworkProtocolVersion == packet->versionNumber );
               }
            }
         }
         break;
      }
      break;
      case PacketType_Contact:
      {
         switch( packetIn->packetSubType )
         {
         case PacketContact::ContactType_GetListOfContactsResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfContactsResponse* packet = static_cast< PacketContact_GetListOfContactsResponse* >( packetIn );
               cout << "num records: " << packet->friends.size() << endl;
               if( m_callbacks.size() )
               {
                  m_friends.clear();

                  SerializedKeyValueVector< FriendInfo >::const_KVIterator it = packet->friends.begin();
                  while( it != packet->friends.end() )
                  {
                     BasicUser bu;
                     bu.isOnline = it->value.isOnline;
                     bu.userName = it->value.userName;
                     bu.UUID = it->key;

                     m_friends.insert( it->key, bu );
                     it++;
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendsUpdate();
                  }
               }
            }
            break;
         case PacketContact::ContactType_UserOnlineStatusChange:
            {
               cout << "contacts received" << endl;
               PacketContact_FriendOnlineStatusChange* packet = static_cast< PacketContact_FriendOnlineStatusChange* >( packetIn );
               cout << packet->friendInfo.userName << " online status = " << boolalpha << packet->friendInfo.isOnline << noboolalpha << endl;
               
               if( m_callbacks.size() )
               {
                  bool updated = false;
                  UserNameKeyValue::KVIterator  itFriends = m_friends.begin();
                  while( itFriends != m_friends.end() )
                  {
                     if( itFriends->key == packet->uuid )
                     {
                        itFriends->value.isOnline = packet->friendInfo.isOnline;
                        updated = true;
                     }
                     itFriends++;
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendOnlineStatusChanged( packet->uuid );
                  }
               }
            }
            break;
         case PacketContact::ContactType_GetListOfInvitationsResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfInvitationsResponse* packet = static_cast< PacketContact_GetListOfInvitationsResponse* >( packetIn );
               cout << "Num invites received: " << packet->invitations.size() << endl;

               m_invitationsReceived.clear();
               SerializedKeyValueVector< InvitationInfo >& kvVector = packet->invitations;
               m_invitationsReceived = kvVector;

               SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
               while (it != kvVector.end() )
               {
                  const InvitationInfo& invite = it->value;
                  cout << " **invite from: " << invite.inviterName << endl;
                  cout << "   invite to: " << invite.inviteeName << endl;
                  cout << "   message: " << invite.message << endl;
                  it++;
               }

               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationsReceivedUpdate();
                  }
               }
            }
            break;
         case PacketContact::ContactType_GetListOfInvitationsSentResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfInvitationsSentResponse* packet = static_cast< PacketContact_GetListOfInvitationsSentResponse* >( packetIn );
               cout << "Num invites sent: " << packet->invitations.size() << endl;

               m_invitationsSent.clear();
               SerializedKeyValueVector< InvitationInfo >& kvVector = packet->invitations;
               m_invitationsSent = kvVector;// expensive copy

               SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
               while (it != kvVector.end() )
               {
                  const InvitationInfo& invite = it->value;
                  cout << " **invite from: " << invite.inviterName << endl;
                  cout << "   invite to: " << invite.inviteeName << endl;
                  cout << "   message: " << invite.message << endl;
                  it++;
               }

               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationsSentUpdate();
                  }
               }
               
            }
            break;
         case PacketContact::ContactType_InviteSentNotification:
            {
               cout << "new invite received" << endl;
               PacketContact_InviteSentNotification* packet = static_cast< PacketContact_InviteSentNotification* >( packetIn );
               cout << "From " << packet->info.inviterName << endl;
               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationReceived( packet->info );
                  }
               }
               m_invitationsReceived.insert( packet->info.uuid, packet->info );
            }
            break;
         case PacketContact::ContactType_InvitationAccepted:
            {
               cout << "invite accepted" << endl;
               PacketContact_InvitationAccepted* packet = static_cast< PacketContact_InvitationAccepted* >( packetIn );
               cout << "From " << packet->fromUsername << endl;
               cout << "To " << packet->toUsername << endl;
               cout << "Was accepted " << packet->wasAccepted << endl;

               for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->InvitationAccepted( packet->fromUsername, packet->toUsername, packet->wasAccepted );
               }

               cout << "request a new list of friends" << endl;
            }
            break;
         }
         
      }
      break;
      case PacketType_ErrorReport:
      {
         PacketErrorReport* errorReport = static_cast<PacketErrorReport*>( packetIn );
         switch( errorReport->errorCode )
         {
         case PacketErrorReport::ErrorType_CreateFailed_BadPassword:
            cout << "Bad password" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DisallowedUsername:
            cout << "Bad username" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DuplicateUsername:
            cout << "Duplicate username" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DuplicateEmail:
            cout << "Duplicate email" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateAccount_Success:
            cout << "Account created" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateAccount_AccountUpdated:
            cout << "Account update" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_UserCreateAccountPending:
            cout << "Account creation pending. Check your email." << endl;
            break;

         case PacketErrorReport::ErrorType_UserBadLogin:
            cout << "Login not valid... either user email is wrong or the password." << endl;
            break;

         case PacketErrorReport::ErrorType_Contact_Invitation_success:
         case PacketErrorReport::ErrorType_Contact_Invitation_ProblemFindingUser:
         case PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyInvited:
         case PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyFriend:
         case PacketErrorReport::ErrorType_Contact_Invitation_InvalidUser:
         case PacketErrorReport::ErrorType_Contact_Invitation_CannotInviteSelf:
         case PacketErrorReport::ErrorType_Contact_Invitation_Accepted:
         case PacketErrorReport::ErrorType_Contact_Invitation_BadInvitation:
            cout << "Contacts code: " << (int)errorReport->packetSubType  << endl;
            break;

         case PacketErrorReport::ErrorType_ChatNotCurrentlyAvailable:// reported at the gateway
         case PacketErrorReport::ErrorType_BadChatChannel:
         case PacketErrorReport::ErrorType_NoChatChannel:
         case PacketErrorReport::ErrorType_UserNotOnline:
         case PacketErrorReport::ErrorType_NotAMemberOfThatChatChannel:
         case PacketErrorReport::ErrorType_YouAreTheOnlyPersonInThatChatChannel:
         case PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists:
         case PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel:
         case PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser:
            cout << "Chat code: " << (int)errorReport->packetSubType  << endl;
            break;
         }
         m_isCreatingAccount = false;
         for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
         {
            (*it)->OnError( errorReport->errorCode, errorReport->statusInfo );
         }
      }
      break;
      case PacketType_Login:
      {
         switch( packetIn->packetSubType )
         {
            case PacketLogin::LoginType_InformClientOfLoginStatus:
               {
                  PacketLoginToClient* login = static_cast<PacketLoginToClient*>( packetIn );
                  if( login->wasLoginSuccessful == true )
                  {
                     m_userName = login->userName;
                     m_uuid = login->uuid;
                     m_connectionId = login->connectionId;
                     m_lastLoggedOutTime = login->lastLogoutTime;
                     m_loginKey = login->loginKey;
                     m_isLoggedIn = true;
                  }
                  else
                  {
                     Disconnect();// server forces a logout.
                  }
                  NotifyClientLoginStatus( m_isLoggedIn );
                  
                  m_isLoggingIn = false;
               }
               break;
            case PacketLogin::LoginType_PacketLogoutToClient:
               {
                  PacketLogout* logout = static_cast<PacketLogout*>( packetIn );
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->UserLogout();
                  }
                  Disconnect();
                  m_isLoggingIn = false;
                  m_isLoggedIn = false;
               }
               break;
            case PacketLogin::LoginType_RequestListOfPurchases:
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->ServerRequestsListOfUserPurchases();
                  }
               }
               break;
            case PacketLogin::LoginType_ListOfAggregatePurchases:
               {
                  PacketListOfUserAggregatePurchases* purchases = static_cast<PacketListOfUserAggregatePurchases*>( packetIn );
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {  
                     (*it)->ListOfAggregateUserPurchases( purchases->purchases, purchases->platformId );
                  }
               }
               break;
            case PacketLogin::LoginType_RequestUserProfileResponse:
               {
                  PacketRequestUserProfileResponse* profile = static_cast<PacketRequestUserProfileResponse*>( packetIn );
                  map< string, string > profileKeyValues;

                  const SerializedKeyValueVector< string >& kvVector = profile->profileKeyValues;
                  SerializedKeyValueVector< string >::const_KVIterator profileIt = kvVector.begin();
                  while( profileIt != kvVector.end() )
                  {
                     profileKeyValues.insert( pair< string, string >( profileIt->key, profileIt->value ) );
                     profileIt++;
                  }

                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {  
                     (*it)->UserProfileResponse( profile->userName, 
                                          profile->email, 
                                          profile->userUuid, 
                                          profile->lastLoginTime, 
                                          profile->loggedOutTime, 
                                          profile->adminLevel, 
                                          profile->isActive, 
                                          profile->showWinLossRecord, 
                                          profile->marketingOptOut, 
                                          profile->showGenderProfile );
                     (*it)->UserProfileResponse( profileKeyValues );
                  }
               }
               break;
            case PacketLogin::LoginType_RequestListOfProductsResponse:
               {
                  PacketRequestListOfProductsResponse* products = static_cast<PacketRequestListOfProductsResponse*>( packetIn );
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {  
                     (*it)->ListOfAvailableProducts( products->products, products->platformId );
                  }
               }
               break;
            case PacketLogin::LoginType_RequestOtherUserProfileResponse:
               {
                  PacketRequestOtherUserProfileResponse* profile = static_cast<PacketRequestOtherUserProfileResponse*>( packetIn );
                  string userName =       profile->basicProfile.find( "name" );
                  string userUuid =       profile->basicProfile.find( "uuid" );
                  bool showWinLoss =      boost::lexical_cast< bool >( profile->basicProfile.find( "show_win_loss_record" ) );
                  int timeZoneGMT =       boost::lexical_cast< int >( profile->basicProfile.find( "time_zone" ) );
                  string avatarIcon =     profile->basicProfile.find( "avatar_icon" );

                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  { 
                     (*it)->OtherUsersProfile( userName, userUuid, avatarIcon, showWinLoss, timeZoneGMT, profile->productsOwned );
                  }
               }
               break;
         }
         
      }
      break;
      case PacketType_UserInfo:
      {
         switch( packetIn->packetSubType )
         {
         case PacketUserInfo::InfoType_ChatChannelList:
            {
               PacketChatChannelList* channelList = static_cast<PacketChatChannelList*>( packetIn );

               cout << " chat channel list received " << channelList->channelList.size() << endl;
               const SerializedKeyValueVector< ChannelInfo >& kvVector = channelList->channelList;
               SerializedKeyValueVector< ChannelInfo >::const_KVIterator channelIt = kvVector.begin();
               while( channelIt != kvVector.end() )
               {
                  ChatChannel newChannel;
                  newChannel.uuid = channelIt->value.channelUuid;
                  newChannel.channelName = channelIt->value.channelName;
                  newChannel.gameProductId = channelIt->value.gameProduct;
                  newChannel.gameInstanceId = channelIt->value.gameId;
                  newChannel.channelDetails= channelIt->value.channelName;
                  //newChannel.userList = channelIt->value.userList;
                  if( AddChatChannel( newChannel ) )
                  {
                     for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                     {
                        (*it)->ChatChannelUpdate( channelIt->value.channelUuid );
                     }
                  }
                  channelIt++;
               }
            }
            break;
            // demographics, winloss, etc.
         }
      }
      break;
      case PacketType_Purchase:
         {
            switch( packetIn->packetSubType )
            {
            case PacketPurchase::PurchaseType_BuyResponse:
               {
                  PacketPurchase_BuyResponse* purchase = static_cast<PacketPurchase_BuyResponse*>( packetIn );
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->PurchaseSuccess( purchase->purchaseUuid, purchase->success );
                  }
               }
               break;
            case PacketPurchase::PurchaseType_RequestListOfSalesResponse:
               {
                  PacketPurchase_RequestListOfSalesResponse* purchase = static_cast<PacketPurchase_RequestListOfSalesResponse*>( packetIn );
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->ProductsForSale( purchase->thingsForSale );
                  }
               }
               break;
            }

         }
         break;
      case PacketType_Chat:
      {
         switch( packetIn->packetSubType )
         {
          case PacketChatToServer::ChatType_ChatToClient:
            {
               cout << "You received a message: " << endl;
               PacketChatToClient* chat = static_cast<PacketChatToClient*>( packetIn );
               cout << " *** from: " << chat->userName;
               cout << "     message: " << chat->message << endl;
               for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->ChatReceived( chat->message, chat->channelUuid, chat->userUuid, chat->timeStamp );
               }

               // add to some form of history?

               //m_friends = login->friendList.GetData();
               //DumpFriends();
            }
            break;
          case PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer:
             {
               cout << "You received a chat channel addition: " << endl;
               PacketChatUserAddedToChatChannelFromGameServer* chat = static_cast<PacketChatUserAddedToChatChannelFromGameServer*>( packetIn );
               if( m_callbacks.size() )
               {
                  bool found = false;
                  vector< ChatChannel >::iterator it = m_channels.begin();
                  while( it != m_channels.end() )
                  {
                     ChatChannel& channel = *it;
                     if( channel.uuid == chat->channelUuid )
                     {
                        found = true;
                        channel.userList = chat->userList;
                        for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                        {
                           (*it)->ChatChannelUpdate( channel.uuid );
                        }
                        break;
                     }
                  }
                  if( found == false )
                  {
                     ChatChannel newChannel;
                     newChannel.uuid = chat->channelUuid;
                     newChannel.channelName = chat->gameName;
                     newChannel.gameInstanceId = chat->gameId;
                     newChannel.channelDetails= chat->gameName;
                     newChannel.userList = chat->userList;
                     if( AddChatChannel( newChannel ) )
                     {
                        for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                        {
                           (*it)->ChatChannelUpdate( newChannel.uuid );
                        }
                     }
                  }
               }
             }
             break;
          case PacketChatToServer::ChatType_RequestHistoryResult:
            {
               PacketChatHistoryResult* history = static_cast<PacketChatHistoryResult*>( packetIn );
               int num = history->chat.size();

               
               /**/
               if( m_callbacks.size() == 0 || ( history->chatChannelUuid.size() == 0 && history->userUuid.size() == 0 ) )
               {
                  cout << "Chat items for this channel are [" << num << "] = {";

                  for( int i=0; i<num; i++ )
                  {
                     cout << history->chat[i].userName << " said " << history->chat[i].message;
                     if( i < num-1 )
                         cout << ", ";
                  }
                  cout << "}" << endl;
               }
               else
               {
                  list< ChatEntry > listOfChats;
                  for( int i=0; i<num; i++ )
                  {
                     listOfChats.push_back( history->chat[i] );
                  }
                  bool invokeChannelNotifier = false;

                  if( history->chatChannelUuid.size() )
                  {
                     invokeChannelNotifier  = true;
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     if( invokeChannelNotifier )
                     {
                        (*it)->ChatChannelHistory( history->chatChannelUuid, listOfChats );
                     }
                     else
                     {
                        (*it)->ChatP2PHistory( history->userUuid, listOfChats );
                     }
                  }
               }
            }
            break;
          case PacketChatToServer::ChatType_UserChatStatusChange:
            {
               cout << "char contacts received" << endl;
               PacketChatUserStatusChangeBase* packet = static_cast< PacketChatUserStatusChangeBase* >( packetIn );
              // cout << "num records: " << packet->m_>friends.size() << endl;
               cout << packet->userName << " online status = " << boolalpha << packet->statusChange << noboolalpha << endl;
               
               if( m_callbacks.size() )
               {
                  bool updated = false;
                  UserNameKeyValue::KVIterator  itFriends = m_friends.begin();
                  while( itFriends != m_friends.end() )
                  {
                     if( itFriends->key == packet->uuid )
                     {
                        itFriends->value.isOnline = packet->statusChange ? true : false; // 0=offline, everything else is some kind of online
                        updated = true;
                     }
                     itFriends++;
                  }
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendOnlineStatusChanged( packet->uuid );
                  }
               }
             }
            break;
         }
      }
      break;
      case PacketType_Gameplay:
      {
         switch( packetIn->packetSubType )
         {
         case PacketGameToServer::GamePacketType_GameIdentification:
            {
               PacketGameIdentification* gameId = 
                  static_cast<PacketGameIdentification*>( packetIn );

               m_gameList.push_back( PacketGameIdentification( *gameId ) );
               if( gameId->gameProductId == m_gameProductId ) 
               {
                  m_selectedGame = gameId->gameId;
               }

               NotifyClientToBeginSendingRequests();
            }
            break;
         case PacketGameToServer::GamePacketType_RawGameData:
            {
               PacketGameplayRawData* data = 
                  static_cast<PacketGameplayRawData*>( packetIn );

               HandleData( data );
               cleaner.Clear();
            }
            //return false;// this is important... we will delete the packets.
            break;
         
         case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
            {
               PacketRequestUserWinLossResponse* response = 
                     static_cast<PacketRequestUserWinLossResponse*>( packetIn );
               for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->UserWinLoss( response->userUuid, response->winLoss );
               }
               assert( 0 );// not finished, wrong user data
            }
            break;
         }
      }
      break;
      //------------------------------------------------------------
      case PacketType_Asset:
      {
         switch( packetIn->packetSubType )
         {
            case PacketAsset::AssetType_GetListOfAssetCategoriesResponse:
            {
               PacketAsset_GetListOfAssetCategoriesResponse* categoryList = 
                  static_cast<PacketAsset_GetListOfAssetCategoriesResponse*>( packetIn );

               int num = categoryList->assetcategory.size();
               m_assets.clear();

               for( int i=0; i< num; i++ )
               {
                  const string& name = categoryList->assetcategory[i];
                  AssetMapIter it = m_assets.find( name );
                  if( it == m_assets.end() )
                  {
                     m_assets.insert( AssetMapPair( name, AssetCollection () ) );
                  }
               }
               for( list< UserNetworkEventNotifier* >::iterator callbackIt = m_callbacks.begin(); callbackIt != m_callbacks.end(); ++callbackIt )
               {
                  (*callbackIt)->AssetCategoriesLoaded();
               }
            }
            break;

            case PacketAsset::AssetType_GetListOfAssetsResponse:
            {
               PacketAsset_GetListOfAssetsResponse* assetList = 
                  static_cast<PacketAsset_GetListOfAssetsResponse*>( packetIn );

               string& category = assetList->assetCategory;
               assert( category.size() > 0 );

               AssetMapIter it = m_assets.find( category );
               if( it == m_assets.end() )
               {
                  m_assets.insert( AssetMapPair( category, AssetCollection () ) );
                  it = m_assets.find( category );
               }

               AssetCollection& collection = it->second;
               collection.clear();


               SerializedKeyValueVector< AssetInfo >::KeyValueVectorIterator updatedAssetIt = assetList->updatedAssets.begin();
               while ( updatedAssetIt != assetList->updatedAssets.end() )
               {
                  AssetInfoExtended extender( updatedAssetIt->value );
                  collection.push_back( extender );
                  updatedAssetIt ++;
               }

               for( list< UserNetworkEventNotifier* >::iterator callbackIt = m_callbacks.begin(); callbackIt != m_callbacks.end(); ++callbackIt )
               {
                  (*callbackIt)->AssetManifestAvailable( category );
               }
            }
            break;
          }
      }
      break;
      case PacketType_Tournament:
      {
         switch( packetIn->packetSubType )
         {
            case PacketTournament::TournamentType_RequestListOfTournamentsResponse:
            {
               PacketTournament_RequestListOfTournamentsResponse* response = 
                  static_cast<PacketTournament_RequestListOfTournamentsResponse*>( packetIn );
               m_availableTournaments.clear();

               SerializedKeyValueVector< TournamentInfo >::KeyValueVectorIterator it = response->tournaments.begin();
               while ( it != response->tournaments.end() )
               {
                  TournamentInfo ti = it->value;
                  m_availableTournaments.push_back( ti );
                  it++;
               }
               if( m_availableTournaments.size() )
               {
                  for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->TournamentListAvalable();
                  }
               }
               else
               {
                  cout << "No tournaments available" << endl;
               }
            }
            break;
            case PacketTournament::TournamentType_UserRequestsEntryInTournamentResponse:
            {
               PacketTournament_UserRequestsEntryInTournamentResponse* response = 
                  static_cast<PacketTournament_UserRequestsEntryInTournamentResponse*>( packetIn );
              
               for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->TournamentPurchaseResult( response->tournamentUuid, response->result );
               }
            }
            break;
         }
      }
      break;
   }

   return true;
}

//------------------------------------------------------------------------

void     NetworkLayer::HandleData( PacketGameplayRawData* data )
{
   int dataType = data->dataType;
   RawDataAccumulator& rawDataBuffer = m_rawDataBuffer[ dataType ];
   rawDataBuffer.AddPacket( data );
   string hash = data->identifier;
   if( rawDataBuffer.GetRemainingSize() > 10*1024 )
   {
      BoostThreadPerformance();
   }
   
   cout << "packet type: " << dataType << ", index: " << (int) data->index << ", size:" << (int) data->size;
   if( m_lastRawDataIndex - data->index > 1 )
   {
      cout << "***";
   }
   m_lastRawDataIndex = data->index;
   cout << endl;

   if( rawDataBuffer.IsDone() )
   {
      if( dataType == PacketGameplayRawData::Game )
      {
         U8* buffer = NULL;
         int size = 0;
         rawDataBuffer.PrepPackage( buffer, size );
         for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
         {
            (*it)->GameData( size, buffer );
         }
         delete buffer;
      }
      else
      {
         AssetInfoExtended asset;
         if( GetAsset( hash, asset ) )
         {
            rawDataBuffer.PrepPackage( asset );
            UpdateAssetData( hash, asset );
            for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
            {
               (*it)->AssetDataAvailable( asset.category, hash );
            }
         }
         else
         {
            cout << " unknown asset has arrived " << hash << endl;
         }
      }
      
      RestoreNormalThreadPerformance();
   }
   
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

void     NetworkLayer::NotifyClientLoginStatus( bool isLoggedIn )
{
   for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
   {
      (*it)->UserLogin( isLoggedIn );
   }

   NotifyClientToBeginSendingRequests();   
}

//------------------------------------------------------------------------

void     NetworkLayer::NotifyClientToBeginSendingRequests()
{
   if( m_isLoggedIn == false )
      return;

   if( m_selectedGame )
   {
      for( list< UserNetworkEventNotifier* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
      {
         (*it)->ReadyToStartSendingRequestsToGame();
      }
   }
}

//------------------------------------------------------------------------

void     NetworkLayer::InitalConnectionCallback()
{
   PacketHello hello;

   SerializePacketOut( &hello );
}

//------------------------------------------------------------------------

bool     NetworkLayer::AddChatChannel( const ChatChannel& channel )
{
   vector< ChatChannel >::iterator it =  m_channels.begin();
   while( it != m_channels.end() )
   {
      if( it->uuid == channel.uuid )
      {
         return false;
      }
      it++;
   }
   m_channels.push_back( channel );
   return true;
}

//------------------------------------------------------------------------

bool     NetworkLayer::GetAsset( const string& hash, AssetInfoExtended& asset )
{
   AssetMapConstIter it = m_assets.begin();//find( category );
   while( it != m_assets.end() )
   {
      const vector< AssetInfoExtended >& arrayOfAssets = it->second;
      it++;

      vector< AssetInfoExtended >::const_iterator itAssets = arrayOfAssets.begin();
      while( itAssets != arrayOfAssets.end() )
      {
         if( itAssets->assetHash == hash )
         {
            asset = *itAssets;
            return true;
         }
         itAssets++;
      }
   }

   return false;
}

//------------------------------------------------------------------------

bool     NetworkLayer::GetAsset( const string& category, const string& hash, AssetInfoExtended& asset )
{
   AssetMapConstIter it = m_assets.find( category );
   if( it != m_assets.end() )
   {
      const vector< AssetInfoExtended >& arrayOfAssets = it->second;
      vector< AssetInfoExtended >::const_iterator itAssets = arrayOfAssets.begin();
      while( itAssets != arrayOfAssets.end() )
      {
         if( itAssets->assetHash == hash )
         {
            asset = *itAssets;
            return true;
         }
         itAssets++;
      }
   }

   return false;
}

//------------------------------------------------------------------------

bool     NetworkLayer::UpdateAssetData( const string& hash, AssetInfoExtended& asset )
{
   AssetMapIter it = m_assets.begin();
   while( it != m_assets.end() )
   {
      vector< AssetInfoExtended >& arrayOfAssets = it->second;
      it++;

      vector< AssetInfoExtended >::iterator itAssets = arrayOfAssets.begin();
      while( itAssets != arrayOfAssets.end() )
      {
         if( itAssets->assetHash == hash )
         {
            itAssets->MoveData( asset );
            return true;
         }
         itAssets++;
      }
   }

   return false;
}
///////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------

NetworkLayer2::NetworkLayer2( U8 gameProductId, bool isTestingOnly ) : 
      m_gameProductId( gameProductId ), 
      m_requiresGatewayDiscovery( true ),
      m_isLoggingIn( false ),
      m_isLoggedIn( false ),
      m_connectionId( 0 ), 
      m_selectedGame( 0 ),
      m_normalSleepTime( 50 ),
      m_boostedSleepTime( 8 ),
      m_isThreadPerformanceBoosted( 0 ),
      m_lastRawDataIndex( 0 ),
      m_connectionPort( 9500 )
{
   InitializeSockets();

   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      m_fruitadens[i] = new Fruitadens( "Networking Layer", false );
      
      m_fruitadens[i]->SetSleepTime( m_normalSleepTime );
   }

   
   if( isTestingOnly )
   {
      m_serverDns = "chat.mickey.playdekgames.com";
   }
   else
   {
      m_serverDns = "gateway.internal.playdekgames.com";
   }

   ReconnectMain();
}

NetworkLayer2::~NetworkLayer2()
{
   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      delete m_fruitadens[i];
   }
   Exit();
   ShutdownSockets();
}

bool     NetworkLayer2::IsConnected() const
{
   return m_fruitadens[ ConnectionNames_Main ]->IsConnected();
}

void     NetworkLayer2::ReconnectMain()
{
   if( IsConnected() == false )
   {
      m_fruitadens[ ConnectionNames_Main ]->Connect( m_serverDns.c_str(), m_connectionPort );
      m_fruitadens[ ConnectionNames_Main ]->RegisterPacketHandlerInterface( this );

      if( m_requiresGatewayDiscovery )
      {
         PacketRerouteRequest* request = new PacketRerouteRequest;
         SerializePacketOut( request );
      }
   }
}

void     NetworkLayer2::Disconnect()
{
   if( IsConnected() == true )
   {
      m_fruitadens[ ConnectionNames_Main ]->RegisterPacketHandlerInterface( NULL );
      m_fruitadens[ ConnectionNames_Main ]->Disconnect();
   }
}

//------------------------------------------------------------
//------------------------------------------------------------

void  NetworkLayer2::Exit()
{
   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      m_fruitadens[i]->Disconnect();
   }

   m_requiresGatewayDiscovery = true;
   m_isLoggingIn = false;
   m_isLoggedIn = false;

   m_invitationsReceived.clear();
   m_invitationsSent.clear();
   m_staticAssets.clear();
   m_dynamicAssets.clear();
   m_availableTournaments.clear();
}

string   NetworkLayer2::GenerateHash( const string& stringThatIWantHashed )
{
   string value;
   string lowerCaseString = ConvertStringToLower( stringThatIWantHashed );
   ::ConvertToString( ::GenerateUniqueHash( lowerCaseString ), value );
   if( value.size() > TypicalMaxHexLenForNetworking )// limit
   {
      value = value.substr( value.size()-TypicalMaxHexLenForNetworking, TypicalMaxHexLenForNetworking); 
   }
   return value;
}

//------------------------------------------------------------

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestLogin( const string& userName, const string& password, const string& languageCode )
{
   ReconnectMain();

   PacketLogin login;
   login.loginKey = "deadbeef";// currently unused
   login.uuid = userName;
   login.userName = userName;
   login.loginKey = password;
   login.languageCode = languageCode;
   login.password = boost::lexical_cast< string >( CreatePasswordHash( password.c_str() ) );
   m_attemptedUsername = userName;

   m_isLoggingIn = true;
   SerializePacketOut( &login );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestAccountCreate( const string& userName, const string& useremail, const string& password, int languageId, const string& deviceId, const string& gkHash )
{
   ReconnectMain();

   assert( m_isLoggedIn == false );

   PacketCreateAccount createAccount;
   createAccount.userName = userName;
   createAccount.useremail = useremail;
   createAccount.password = boost::lexical_cast< string >( CreatePasswordHash( password.c_str() ) );
   createAccount.deviceId = deviceId;
   createAccount.deviceAccountId = boost::lexical_cast< string >( CreatePasswordHash( gkHash.c_str() ) );
   createAccount.languageId = languageId;

   m_attemptedUsername = userName;

   m_isCreatingAccount = true;
   SerializePacketOut( &createAccount );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestLogout() const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketLogout logout;
   SerializePacketOut( &logout );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::IsReadyToLogin() const 
{ 
   if( m_requiresGatewayDiscovery )
      return false;

   return !m_isLoggingIn & !m_isLoggedIn; 
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestProfile( const string userName )//if empty, profile for currently logged in user is used
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketRequestUserProfile request;
   request.userName = userName;
   if( userName == "" )
      request.uuid = m_uuid;
   SerializePacketOut( &request );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestOtherUserInGameProfile( const string& userName ) // friends, games list, etc
{
   if( IsConnected() == false || userName.size() == 0 )
   {
      return false;
   }
   PacketRequestOtherUserProfile request;
   request.userName = userName;

   SerializePacketOut( &request );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::UpdateUserProfile( const string userName, const string& email, const string& userUuid, int adminLevel, int languageId, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile )
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketUpdateUserProfile       update;
   update.userName =             userName;
   update.email =                email;
   update.userUuid =             userUuid;
   
   update.adminLevel =           adminLevel;
   update.languageId =           languageId;
   update.isActive =             isActive;
   update.showWinLossRecord =    showWinLossRecord;
   update.marketingOptOut =      marketingOptOut;
   update.showGenderProfile =    showGenderProfile;

   SerializePacketOut( &update );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestListOfFriends() const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketContact_GetListOfContacts friends;
   SerializePacketOut( &friends );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestListOfProducts() const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketRequestListOfProducts products;
   SerializePacketOut( &products );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestListOfProductsInStore() const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketPurchase_RequestListOfSales products;
   SerializePacketOut( &products );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestListOfPurchases( const string userUuid ) const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketListOfUserPurchasesRequest purchases;
   if( userUuid.size() > 0 )
   {
      purchases.userUuid = userUuid;
   }
   else 
   {
      purchases.userUuid = m_uuid;
   }
   SerializePacketOut( &purchases );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::MakePurchase( const string& exchangeUuid ) const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketPurchase_Buy purchase;
   purchase.purchaseUuid = exchangeUuid;
   SerializePacketOut( &purchase );

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::RequestListOfTournaments()
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketTournament_RequestListOfTournaments tournaments;
   SerializePacketOut( &tournaments );// only requests tournaments for the current game

   return true;
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::PurchaseEntryIntoTournament( const string& tournamentUuid )
{
   if( IsConnected() == false )
   {
      return false;
   }

   PacketTournament_UserRequestsEntryInTournament entry;
   entry.tournamentUuid = tournamentUuid;

   SerializePacketOut( &entry );

   return true;
}

//-----------------------------------------------------------------------------
  
bool  NetworkLayer2::RequestChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex ) const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketChatHistoryRequest history;
   history.chatChannelUuid = channelUuid;
   history.numRecords = numRecords;
   history.startingIndex = startingIndex;
   SerializePacketOut( &history );
   return true;
}

//-----------------------------------------------------------------------------
  
bool  NetworkLayer2::RequestChatP2PHistory( const string& userUuid, int numRecords, int startingIndex ) const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketChatHistoryRequest history;
   history.userUuid = userUuid;
   history.numRecords = numRecords;
   history.startingIndex = startingIndex;
   SerializePacketOut( &history );
   return true;
}

//-----------------------------------------------------------------------------

string   NetworkLayer2::FindFriend( const string& name ) const 
{
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      const BasicUser&  kvpFriend = itFriends->value;
      
      if( kvpFriend.userName == name )
      {
         return itFriends->key;
      }
      itFriends++;
   }
   if( name == m_userName )// this method is commonly used as a lookup.. let's make it simple to use
      return m_uuid;

   return string();
}

//-----------------------------------------------------------------------------

string   NetworkLayer2::FindFriendFromUuid( const string& uuid ) const 
{
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      if( itFriends->key == uuid )
      {
         return itFriends->value.userName;
      }
   }
   if( uuid == m_uuid )// this method is commonly used as a lookup.. let's make it simple to use
      return m_userName;

   return string();
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::GetFriend( int index, const BasicUser*& user )
{
   if( index < 0 || index >= m_friends.size() )
   {
      user = NULL;
      return false;
   }

   int i = 0;
   UserNameKeyValue::const_KVIterator  itFriends = m_friends.begin();
   while( itFriends != m_friends.end() )
   {
      if( i == index )
      {
         user = &itFriends->value;
         return true;
      }
      i ++;
      itFriends++;
   }

   return false;

}

//-----------------------------------------------------------------------------

bool    NetworkLayer2::GetChannel( int index, ChatChannel& channel )
{
   if( index < 0 || index >= (int) m_channels.size() )
   {
      channel.Clear();
      return false;
   }

   int i = 0;
   vector< ChatChannel >::const_iterator itChannels = m_channels.begin();
   while( itChannels != m_channels.end() )
   {
      if( i == index )
      {
         channel = *itChannels;
         return true;
      }
      i ++;
      itChannels++;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool    NetworkLayer2::GetStaticAssetInfo( int index, AssetInfoExtended& asset )
{
   if( index < 0 || index >= (int) m_staticAssets.size() )
   {
      asset.Clear();
      return false;
   }

   int i = 0;
   vector< AssetInfoExtended >::const_iterator itAssets = m_staticAssets.begin();
   while( itAssets != m_staticAssets.end() )
   {
      if( i == index )
      {
         asset = *itAssets;
         return true;
      }
      i ++;
      itAssets++;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool    NetworkLayer2::GetDynamicAssetInfo( int index, AssetInfoExtended& asset )
{
   if( index < 0 || index >= (int) m_dynamicAssets.size() )
   {
      asset.Clear();
      return false;
   }

   int i = 0;
   vector< AssetInfoExtended >::const_iterator itAssets = m_dynamicAssets.begin();
   while( itAssets != m_dynamicAssets.end() )
   {
      if( i == index )
      {
         asset = *itAssets;
         return true;
      }
      i ++;
      itAssets++;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2:: ClearAssetInfo( const string& hash )
{
   vector< AssetInfoExtended >::iterator itAssets = m_dynamicAssets.begin();
   while( itAssets != m_dynamicAssets.end() )
   {
      if( itAssets->assetHash == hash )
      {
         //itAssets->Clear();
         itAssets->ClearData();
         return true;
      }
      itAssets++;
   }

   itAssets = m_staticAssets.begin();
   while( itAssets != m_staticAssets.end() )
   {
      if( itAssets->assetHash == hash )
      {
         //itAssets->Clear();
         itAssets->ClearData();
         return true;
      }
      itAssets++;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool    NetworkLayer2::GetTournamentInfo( int index, TournamentInfo& tournamentInfo )
{
   if( index < 0 || index >= (int) m_availableTournaments.size() )
   {
      tournamentInfo.Clear();
      return false;
   }

   int i = 0;
   vector< TournamentInfo >::const_iterator itTournament = m_availableTournaments.begin();
   while( itTournament != m_availableTournaments.end() )
   {
      if( i == index )
      {
         tournamentInfo = *itTournament;
         return true;
      }
      i ++;
      itTournament++;
   }

   return false;
}


//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestListOfGames() const
{
   PacketRequestListOfGames listOfGames;
   SerializePacketOut( &listOfGames );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestListOfUsersLoggedIntoGame( ) const
{
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestFriendDemographics( const string& userName ) const
{
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestUserWinLossRecord( const string& userName ) const
{
   PacketRequestUserWinLoss packet;
   packet.userUuid;
   packet.gameUuid;
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestListOfInvitationsSent() const
{
   PacketContact_GetListOfInvitationsSent    invitationRequest;
   return SerializePacketOut( &invitationRequest );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestListOfInvitationsReceived() const
{
   PacketContact_GetListOfInvitations    invitationRequest;
   return SerializePacketOut( &invitationRequest );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::AcceptInvitation( const string& uuid ) const
{
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->key == uuid )
      {
         PacketContact_AcceptInvite invitationAccepted;
         invitationAccepted.invitationUuid = uuid;
        
         return SerializePacketOut( &invitationAccepted );
      }
      it++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::AcceptInvitationFromUsername( const string& userName ) const
{
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->value.inviterName == userName )
      {
         PacketContact_AcceptInvite invitationAccepted;
         invitationAccepted.invitationUuid = it->key;
        
         return SerializePacketOut( &invitationAccepted );
      }
      it++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::DeclineInvitation( const string& uuid, string message ) const
{
   PacketContact_DeclineInvitation invitationDeclined;
   invitationDeclined.invitationUuid = uuid;
   invitationDeclined.message = message;

   return SerializePacketOut( &invitationDeclined );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::GetListOfInvitationsSent( list< InvitationInfo >& listOfInvites )
{
   listOfInvites.clear();
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsSent;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      listOfInvites.push_back( it->value );
      it++;
   }
   return true;
}
//-----------------------------------------------------------------------------

bool     NetworkLayer2::GetListOfInvitationsReceived( list< InvitationInfo >& listOfInvites )
{
   listOfInvites.clear();
   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      listOfInvites.push_back( it->value );
      it++;
   }
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RegisterCallbackInterface( UserNetworkEventNotifier2* _callbacks ) 
{
   for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
   {
      if( *it == _callbacks )
         return false;
   }
   
   m_callbacks.push_back( _callbacks );  

   _callbacks->network2 = this;
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::SendRawPacket( const char* buffer, int length ) const
{
   PacketGameplayRawData raw;
   memcpy( raw.data, buffer, length );
   raw.size = length;

   return SerializePacketOut( &raw );
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::SendSearchForUsers( const string& searchString, int numRequested, int offset ) const // min 2 char
{
   if( searchString.size() < 2 )
      return false;

   assert( 0 ); // ContactType_Search
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::InviteUserToBeFriend( const string& uuid, const string& userName, const string& message )
{
   if( uuid.size() < 2 && userName.size() < 2 )
      return false;

   PacketContact_InviteContact invitation;
   invitation.userName = userName;
   invitation.uuid = uuid;
   invitation.message = message;

   return SerializePacketOut( &invitation );
}

//-----------------------------------------------------------------------------

bool	   NetworkLayer2::SendP2PTextMessage( const string& message, const string& destinationUserUuid )
{
   PacketChatToServer chat;
   chat.message = message;
   chat.userUuid = destinationUserUuid;
   // chat.channelUuid; // not used
   SerializePacketOut( &chat );

   return true;
}

//-----------------------------------------------------------------------------

bool	   NetworkLayer2::SendChannelTextMessage( const string& message, const string& chatChannelUuid, U32 gameTurn )
{
   PacketChatToServer chat;
   chat.message = message;
   chat.gameTurn = gameTurn;
   // chat.userUuid = m_uuid; // not used
   chat.channelUuid = chatChannelUuid;
   SerializePacketOut( &chat );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestListOfStaticAssets( int platformId )
{
   PacketAsset_GetListOfStaticAssets assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.platformId = platformId;
   SerializePacketOut( &assetRequest );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestListOfDynamicAssets( int platformId )
{
   PacketAsset_GetListOfDynamicAssets assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.platformId = platformId;
   SerializePacketOut( &assetRequest );
   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::RequestAsset( const string& assetName )
{
   PacketAsset_RequestAsset assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.assetHash = assetName;
  /* assetRequest.asset.version = "1.0";
   assetRequest.asset.productId = m_gameProductId;
   assetRequest.asset.beginDate = "";
   assetRequest.asset.beginDate = "";*/

   SerializePacketOut( &assetRequest );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::SendPurchases( const vector< RegisteredProduct >& purchases, int platformId )
{
   PacketListOfUserAggregatePurchases packet;
   packet.platformId = platformId;

   vector< RegisteredProduct >::const_iterator it = purchases.begin();
   while( it != purchases.end() )
   {
      const RegisteredProduct& rp = *it++;
      
      PurchaseEntry pe;
      pe.productStoreId = rp.id;
      pe.name = rp.title;
      pe.number_price = rp.number_price;
      pe.price = rp.price;
      pe.date = GetDateInUTC();

      packet.purchases.push_back( pe );
   }

   SerializePacketOut( &packet );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::GiveProduct( const string& userUuid, const string& productUuid, int quantity, const string& notes, int platformId )
{
   PacketAddPurchaseEntry purchaseEntry;
   purchaseEntry.userName = userUuid;
   purchaseEntry.userEmail = userUuid;
   purchaseEntry.userUuid = userUuid;
   purchaseEntry.productUuid = productUuid;
   purchaseEntry.quantity = quantity;

   purchaseEntry.adminNotes  = notes;
   purchaseEntry.platformId = platformId;

   SerializePacketOut( &purchaseEntry );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::SendCheat( const string& cheatText )
{
   PacketCheat cheat;
   cheat.cheat = cheatText;
   cheat.whichServer = ServerType_Login;// needs improvement
   SerializePacketOut( &cheat );

   return true;
}

//-----------------------------------------------------------------------------

bool     NetworkLayer2::SerializePacketOut( BasePacket* packet ) const 
{
   U8 buffer[ MaxBufferSize ];
   int offset = 0;

   packet->gameInstanceId = m_selectedGame;
   packet->gameProductId = m_gameProductId;
   packet->SerializeOut( buffer, offset );
   m_beginTime = GetCurrentMilliseconds();
   return m_fruitadens[ ConnectionNames_Main ]->SendPacket( buffer, offset );
}


//-----------------------------------------------------------------------------


void     NetworkLayer2::BoostThreadPerformance()
{
   m_fruitadens[ ConnectionNames_Main ]->SetSleepTime( m_boostedSleepTime );
   m_fruitadens[ ConnectionNames_Main ]->SetPriority( Threading::CAbstractThread:: ePriorityHigh );
   m_isThreadPerformanceBoosted = true;
   time( &m_timeWhenThreadPerformanceBoosted );
}

void     NetworkLayer2::RestoreNormalThreadPerformance()
{
   m_fruitadens[ ConnectionNames_Main ]->SetSleepTime( m_normalSleepTime );
   m_isThreadPerformanceBoosted = false;
   m_fruitadens[ ConnectionNames_Main ]->SetPriority( Threading::CAbstractThread:: ePriorityNormal );
}

void     NetworkLayer2::ExpireThreadPerformanceBoost()
{
   if( m_isThreadPerformanceBoosted )
   {
      time_t currentTime;// expire after 5 seconds.
      time( &currentTime );
      if( difftime( currentTime, m_timeWhenThreadPerformanceBoosted ) > 5.0 )
      {
         RestoreNormalThreadPerformance();
      }
   }
}

//-----------------------------------------------------------------------------

void     NetworkLayer2::LoadBalancedNewAddress( const PacketRerouteRequestResponse* response )
{
   if( m_requiresGatewayDiscovery == false )
      return;

   m_requiresGatewayDiscovery = false;// clear this flag

   string   connectionIp;
   U16      connectionPort;
   int num = response->locations.size();
   for( int i=0; i< num; i++ )
   {
      const PacketRerouteRequestResponse::Address& address = response->locations[i];
      if( address.whichLocationId == PacketRerouteRequestResponse::LocationId_Gateway )
      {
         Disconnect();
         connectionPort = address.port;
         connectionIp = address.address;
         //Connect( it->address, it->port ); // << slight danger here... disconnecting when we are servicing a previous request
         break;// finish loop
      }
   }

   m_serverDns = connectionIp;
   m_connectionPort = connectionPort;
   /*if( m_awaitingReroute == false )
   {
      InitalConnectionCallback();
   }*/
}

//-----------------------------------------------------------------------------

void      NetworkLayer2::Update()
{
   if( IsConnected() == false )
   {
      if( m_requiresGatewayDiscovery == true )
         return;
      ReconnectMain();
   }

   ExpireThreadPerformanceBoost();

   // Service any sends.
}

//-----------------------------------------------------------------------------

bool  NetworkLayer2::HandlePacketReceived( BasePacket* packetIn )
{
   PacketCleaner cleaner( packetIn );

   switch( packetIn->packetType )
   {
      case PacketType_Base:
      {
         if( packetIn->packetSubType == BasePacket::BasePacket_RerouteRequestResponse )
         {
            PacketRerouteRequestResponse* unwrappedPacket = static_cast< PacketRerouteRequestResponse * > ( packetIn );

            LoadBalancedNewAddress( unwrappedPacket );
         }
      }
      break;
      case PacketType_Contact:
      {
         switch( packetIn->packetSubType )
         {
         case PacketContact::ContactType_GetListOfContactsResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfContactsResponse* packet = static_cast< PacketContact_GetListOfContactsResponse* >( packetIn );
               cout << "num records: " << packet->friends.size() << endl;
               if( m_callbacks.size() )
               {
                  m_friends.clear();

                  SerializedKeyValueVector< FriendInfo >::const_KVIterator it = packet->friends.begin();
                  while( it != packet->friends.end() )
                  {
                     BasicUser bu;
                     bu.isOnline = it->value.isOnline;
                     bu.userName = it->value.userName;
                     bu.UUID = it->key;

                     m_friends.insert( it->key, bu );
                     it++;
                  }
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendsUpdate();
                  }
               }
            }
            break;
         case PacketContact::ContactType_UserOnlineStatusChange:
            {
               cout << "contacts received" << endl;
               PacketContact_FriendOnlineStatusChange* packet = static_cast< PacketContact_FriendOnlineStatusChange* >( packetIn );
               cout << packet->friendInfo.userName << " online status = " << boolalpha << packet->friendInfo.isOnline << noboolalpha << endl;
               
               if( m_callbacks.size() )
               {
                  bool updated = false;
                  UserNameKeyValue::KVIterator  itFriends = m_friends.begin();
                  while( itFriends != m_friends.end() )
                  {
                     if( itFriends->key == packet->uuid )
                     {
                        itFriends->value.isOnline = packet->friendInfo.isOnline;
                        updated = true;
                     }
                     itFriends++;
                  }
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendOnlineStatusChanged( packet->uuid );
                  }
               }
            }
            break;
         case PacketContact::ContactType_GetListOfInvitationsResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfInvitationsResponse* packet = static_cast< PacketContact_GetListOfInvitationsResponse* >( packetIn );
               cout << "Num invites received: " << packet->invitations.size() << endl;

               m_invitationsReceived.clear();
               SerializedKeyValueVector< InvitationInfo >& kvVector = packet->invitations;
               m_invitationsReceived = kvVector;

               SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
               while (it != kvVector.end() )
               {
                  const InvitationInfo& invite = it->value;
                  cout << " **invite from: " << invite.inviterName << endl;
                  cout << "   invite to: " << invite.inviteeName << endl;
                  cout << "   message: " << invite.message << endl;
                  it++;
               }

               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationsReceivedUpdate();
                  }
               }
            }
            break;
         case PacketContact::ContactType_GetListOfInvitationsSentResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfInvitationsSentResponse* packet = static_cast< PacketContact_GetListOfInvitationsSentResponse* >( packetIn );
               cout << "Num invites sent: " << packet->invitations.size() << endl;

               m_invitationsSent.clear();
               SerializedKeyValueVector< InvitationInfo >& kvVector = packet->invitations;
               m_invitationsSent = kvVector;// expensive copy

               SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
               while (it != kvVector.end() )
               {
                  const InvitationInfo& invite = it->value;
                  cout << " **invite from: " << invite.inviterName << endl;
                  cout << "   invite to: " << invite.inviteeName << endl;
                  cout << "   message: " << invite.message << endl;
                  it++;
               }

               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationsSentUpdate();
                  }
               }
               
            }
            break;
         case PacketContact::ContactType_InviteSentNotification:
            {
               cout << "new invite received" << endl;
               PacketContact_InviteSentNotification* packet = static_cast< PacketContact_InviteSentNotification* >( packetIn );
               cout << "From " << packet->info.inviterName << endl;
               if( m_callbacks.size() )
               {
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->InvitationReceived( packet->info );
                  }
               }
               m_invitationsReceived.insert( packet->info.uuid, packet->info );
            }
            break;
         case PacketContact::ContactType_InvitationAccepted:
            {
               cout << "invite accepted" << endl;
               PacketContact_InvitationAccepted* packet = static_cast< PacketContact_InvitationAccepted* >( packetIn );
               cout << "From " << packet->fromUsername << endl;
               cout << "To " << packet->toUsername << endl;
               cout << "Was accepted " << packet->wasAccepted << endl;

               for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->InvitationAccepted( packet->fromUsername, packet->toUsername, packet->wasAccepted );
               }

               cout << "request a new list of friends" << endl;
            }
            break;
         }
         
      }
      break;
      case PacketType_ErrorReport:
      {
         PacketErrorReport* errorReport = static_cast<PacketErrorReport*>( packetIn );
         switch( errorReport->errorCode )
         {
         case PacketErrorReport::ErrorType_CreateFailed_BadPassword:
            cout << "Bad password" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DisallowedUsername:
            cout << "Bad username" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DuplicateUsername:
            cout << "Duplicate username" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_DuplicateEmail:
            cout << "Duplicate email" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateAccount_Success:
            cout << "Account created" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateAccount_AccountUpdated:
            cout << "Account update" << endl;
            break;
         case PacketErrorReport::ErrorType_CreateFailed_UserCreateAccountPending:
            cout << "Account creation pending. Check your email." << endl;
            break;

         case PacketErrorReport::ErrorType_UserBadLogin:
            cout << "Login not valid... either user email is wrong or the password." << endl;
            break;

         case PacketErrorReport::ErrorType_Contact_Invitation_success:
         case PacketErrorReport::ErrorType_Contact_Invitation_ProblemFindingUser:
         case PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyInvited:
         case PacketErrorReport::ErrorType_Contact_Invitation_InviteeAlreadyFriend:
         case PacketErrorReport::ErrorType_Contact_Invitation_InvalidUser:
         case PacketErrorReport::ErrorType_Contact_Invitation_CannotInviteSelf:
         case PacketErrorReport::ErrorType_Contact_Invitation_Accepted:
         case PacketErrorReport::ErrorType_Contact_Invitation_BadInvitation:
            cout << "Contacts code: " << (int)errorReport->packetSubType  << endl;
            break;

         case PacketErrorReport::ErrorType_ChatNotCurrentlyAvailable:// reported at the gateway
         case PacketErrorReport::ErrorType_BadChatChannel:
         case PacketErrorReport::ErrorType_NoChatChannel:
         case PacketErrorReport::ErrorType_UserNotOnline:
         case PacketErrorReport::ErrorType_NotAMemberOfThatChatChannel:
         case PacketErrorReport::ErrorType_YouAreTheOnlyPersonInThatChatChannel:
         case PacketErrorReport::ErrorType_CannotAddUserToChannel_AlreadyExists:
         case PacketErrorReport::ErrorType_NoChatHistoryExistsOnSelectedChannel:
         case PacketErrorReport::ErrorType_NoChatHistoryExistsForThisUser:
            cout << "Chat code: " << (int)errorReport->packetSubType  << endl;
            break;
         }
         m_isCreatingAccount = false;
         for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
         {
            (*it)->OnError( errorReport->errorCode, errorReport->statusInfo );
         }
      }
      break;
      case PacketType_Login:
      {
         switch( packetIn->packetSubType )
         {
            case PacketLogin::LoginType_InformClientOfLoginStatus:
               {
                  PacketLoginToClient* login = static_cast<PacketLoginToClient*>( packetIn );
                  if( login->wasLoginSuccessful == true )
                  {
                     m_userName = login->userName;
                     m_uuid = login->uuid;
                     m_connectionId = login->connectionId;
                     m_lastLoggedOutTime = login->lastLogoutTime;
                     m_loginKey = login->loginKey;
                     m_isLoggedIn = true;
                  }
                  else
                  {
                     Disconnect();// server forces a logout.
                  }
                  NotifyClientLoginStatus( m_isLoggedIn );
                  
                  m_isLoggingIn = false;
               }
               break;
            case PacketLogin::LoginType_PacketLogoutToClient:
               {
                  PacketLogout* logout = static_cast<PacketLogout*>( packetIn );
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->UserLogout();
                  }
                  Disconnect();
                  m_isLoggingIn = false;
                  m_isLoggedIn = false;
               }
               break;
            case PacketLogin::LoginType_RequestListOfPurchases:
               {
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->ServerRequestsListOfUserPurchases();
                  }
               }
               break;
            case PacketLogin::LoginType_ListOfAggregatePurchases:
               {
                  PacketListOfUserAggregatePurchases* purchases = static_cast<PacketListOfUserAggregatePurchases*>( packetIn );
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {  
                     (*it)->ListOfAggregateUserPurchases( purchases->purchases, purchases->platformId );
                  }
               }
               break;
            case PacketLogin::LoginType_RequestUserProfileResponse:
               {
                  PacketRequestUserProfileResponse* profile = static_cast<PacketRequestUserProfileResponse*>( packetIn );
                  map< string, string > profileKeyValues;

                  const SerializedKeyValueVector< string >& kvVector = profile->profileKeyValues;
                  SerializedKeyValueVector< string >::const_KVIterator profileIt = kvVector.begin();
                  while( profileIt != kvVector.end() )
                  {
                     profileKeyValues.insert( pair< string, string >( profileIt->key, profileIt->value ) );
                     profileIt++;
                  }

                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {  
                     (*it)->UserProfileResponse( profile->userName, 
                                          profile->email, 
                                          profile->userUuid, 
                                          profile->lastLoginTime, 
                                          profile->loggedOutTime, 
                                          profile->adminLevel, 
                                          profile->isActive, 
                                          profile->showWinLossRecord, 
                                          profile->marketingOptOut, 
                                          profile->showGenderProfile );
                     (*it)->UserProfileResponse( profileKeyValues );
                  }
               }
               break;
            case PacketLogin::LoginType_RequestListOfProductsResponse:
               {
                  PacketRequestListOfProductsResponse* products = static_cast<PacketRequestListOfProductsResponse*>( packetIn );
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {  
                     (*it)->ListOfAvailableProducts( products->products, products->platformId );
                  }
               }
               break;
            case PacketLogin::LoginType_RequestOtherUserProfileResponse:
               {
                  PacketRequestOtherUserProfileResponse* profile = static_cast<PacketRequestOtherUserProfileResponse*>( packetIn );
                  string userName =       profile->basicProfile.find( "name" );
                  string userUuid =       profile->basicProfile.find( "uuid" );
                  bool showWinLoss =      boost::lexical_cast< bool >( profile->basicProfile.find( "show_win_loss_record" ) );
                  int timeZoneGMT =       boost::lexical_cast< int >( profile->basicProfile.find( "time_zone" ) );
                  string avatarIcon =     profile->basicProfile.find( "avatar_icon" );

                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  { 
                     (*it)->OtherUsersProfile( userName, userUuid, avatarIcon, showWinLoss, timeZoneGMT, profile->productsOwned );
                  }
               }
               break;
         }
         
      }
      break;
      case PacketType_UserInfo:
      {
         switch( packetIn->packetSubType )
         {
         case PacketUserInfo::InfoType_ChatChannelList:
            {
               PacketChatChannelList* channelList = static_cast<PacketChatChannelList*>( packetIn );

               cout << " chat channel list received " << channelList->channelList.size() << endl;
               const SerializedKeyValueVector< ChannelInfo >& kvVector = channelList->channelList;
               SerializedKeyValueVector< ChannelInfo >::const_KVIterator channelIt = kvVector.begin();
               while( channelIt != kvVector.end() )
               {
                  ChatChannel newChannel;
                  newChannel.uuid = channelIt->value.channelUuid;
                  newChannel.channelName = channelIt->value.channelName;
                  newChannel.gameProductId = channelIt->value.gameProduct;
                  newChannel.gameInstanceId = channelIt->value.gameId;
                  newChannel.channelDetails= channelIt->value.channelName;
                  //newChannel.userList = channelIt->value.userList;
                  if( AddChatChannel( newChannel ) )
                  {
                     for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                     {
                        (*it)->ChatChannelUpdate( channelIt->value.channelUuid );
                     }
                  }
                  channelIt++;
               }
            }
            break;
            // demographics, winloss, etc.
         }
      }
      break;
      case PacketType_Purchase:
         {
            switch( packetIn->packetSubType )
            {
            case PacketPurchase::PurchaseType_BuyResponse:
               {
                  PacketPurchase_BuyResponse* purchase = static_cast<PacketPurchase_BuyResponse*>( packetIn );
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->PurchaseSuccess( purchase->purchaseUuid, purchase->success );
                  }
               }
               break;
            case PacketPurchase::PurchaseType_RequestListOfSalesResponse:
               {
                  PacketPurchase_RequestListOfSalesResponse* purchase = static_cast<PacketPurchase_RequestListOfSalesResponse*>( packetIn );
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->ProductsForSale( purchase->thingsForSale );
                  }
               }
               break;
            }

         }
         break;
      case PacketType_Chat:
      {
         switch( packetIn->packetSubType )
         {
          case PacketChatToServer::ChatType_ChatToClient:
            {
               cout << "You received a message: " << endl;
               PacketChatToClient* chat = static_cast<PacketChatToClient*>( packetIn );
               cout << " *** from: " << chat->userName;
               cout << "     message: " << chat->message << endl;
               for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->ChatReceived( chat->message, chat->channelUuid, chat->userUuid, chat->timeStamp );
               }

               // add to some form of history?

               //m_friends = login->friendList.GetData();
               //DumpFriends();
            }
            break;
          case PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer:
             {
               cout << "You received a chat channel addition: " << endl;
               PacketChatUserAddedToChatChannelFromGameServer* chat = static_cast<PacketChatUserAddedToChatChannelFromGameServer*>( packetIn );
               if( m_callbacks.size() )
               {
                  bool found = false;
                  vector< ChatChannel >::iterator it = m_channels.begin();
                  while( it != m_channels.end() )
                  {
                     ChatChannel& channel = *it;
                     if( channel.uuid == chat->channelUuid )
                     {
                        found = true;
                        channel.userList = chat->userList;
                        for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                        {
                           (*it)->ChatChannelUpdate( channel.uuid );
                        }
                        break;
                     }
                  }
                  if( found == false )
                  {
                     ChatChannel newChannel;
                     newChannel.uuid = chat->channelUuid;
                     newChannel.channelName = chat->gameName;
                     newChannel.gameInstanceId = chat->gameId;
                     newChannel.channelDetails= chat->gameName;
                     newChannel.userList = chat->userList;
                     if( AddChatChannel( newChannel ) )
                     {
                        for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                        {
                           (*it)->ChatChannelUpdate( newChannel.uuid );
                        }
                     }
                  }
               }
             }
             break;
          case PacketChatToServer::ChatType_RequestHistoryResult:
            {
               PacketChatHistoryResult* history = static_cast<PacketChatHistoryResult*>( packetIn );
               int num = history->chat.size();

               
               /**/
               if( m_callbacks.size() == 0 || ( history->chatChannelUuid.size() == 0 && history->userUuid.size() == 0 ) )
               {
                  cout << "Chat items for this channel are [" << num << "] = {";

                  for( int i=0; i<num; i++ )
                  {
                     cout << history->chat[i].userName << " said " << history->chat[i].message;
                     if( i < num-1 )
                         cout << ", ";
                  }
                  cout << "}" << endl;
               }
               else
               {
                  list< ChatEntry > listOfChats;
                  for( int i=0; i<num; i++ )
                  {
                     listOfChats.push_back( history->chat[i] );
                  }
                  bool invokeChannelNotifier = false;

                  if( history->chatChannelUuid.size() )
                  {
                     invokeChannelNotifier  = true;
                  }
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     if( invokeChannelNotifier )
                     {
                        (*it)->ChatChannelHistory( history->chatChannelUuid, listOfChats );
                     }
                     else
                     {
                        (*it)->ChatP2PHistory( history->userUuid, listOfChats );
                     }
                  }
               }
            }
            break;
          case PacketChatToServer::ChatType_UserChatStatusChange:
            {
               cout << "char contacts received" << endl;
               PacketChatUserStatusChangeBase* packet = static_cast< PacketChatUserStatusChangeBase* >( packetIn );
              // cout << "num records: " << packet->m_>friends.size() << endl;
               cout << packet->userName << " online status = " << boolalpha << packet->statusChange << noboolalpha << endl;
               
               if( m_callbacks.size() )
               {
                  bool updated = false;
                  UserNameKeyValue::KVIterator  itFriends = m_friends.begin();
                  while( itFriends != m_friends.end() )
                  {
                     if( itFriends->key == packet->uuid )
                     {
                        itFriends->value.isOnline = packet->statusChange ? true : false; // 0=offline, everything else is some kind of online
                        updated = true;
                     }
                     itFriends++;
                  }
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->FriendOnlineStatusChanged( packet->uuid );
                  }
               }
             }
            break;
         }
      }
      break;
      case PacketType_Gameplay:
      {
         switch( packetIn->packetSubType )
         {
         case PacketGameToServer::GamePacketType_GameIdentification:
            {
               PacketGameIdentification* gameId = 
                  static_cast<PacketGameIdentification*>( packetIn );

               m_gameList.push_back( PacketGameIdentification( *gameId ) );
               if( gameId->gameProductId == m_gameProductId ) 
               {
                  m_selectedGame = gameId->gameId;
               }

               NotifyClientToBeginSendingRequests();
            }
            break;
         case PacketGameToServer::GamePacketType_RawGameData:
            {
               PacketGameplayRawData* data = 
                  static_cast<PacketGameplayRawData*>( packetIn );

               int dataType = data->dataType;
               RawDataAccumulator& rawDataBuffer = m_rawDataBuffer[ dataType ];
               rawDataBuffer.AddPacket( data );
               string hash = data->identifier;
               if( rawDataBuffer.GetRemainingSize() > 10*1024 )
               {
                  BoostThreadPerformance();
               }

               
               cout << "packet type: " << dataType << ", index: " << (int) data->index << ", size:" << (int) data->size;
               if( m_lastRawDataIndex - data->index > 1 )
               {
                  cout << "***";
               }
               m_lastRawDataIndex = data->index;
               cout << endl;

               if( rawDataBuffer.IsDone() )
               {
                  
                  if( dataType == PacketGameplayRawData::Game )
                  {
                     U8* buffer = NULL;
                     int size = 0;
                     rawDataBuffer.PrepPackage( buffer, size );
                     for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                     {
                        (*it)->GameData( size, buffer );
                     }
                     delete buffer;
                  }
                  else
                  {
                     AssetInfoExtended asset;
                     if( GetAsset( hash, asset ) )
                     {
                        rawDataBuffer.PrepPackage( asset );
                        UpdateAssetData( hash, asset );
                        for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                        {
                           (*it)->AssetDataAvailable( asset.category, hash );
                        }
                     }
                     else
                     {
                        cout << " unknown asset has arrived " << hash << endl;
                     }
                  }
                  
                  RestoreNormalThreadPerformance();
               }
               cleaner.Clear();
            }
            //return false;// this is important... we will delete the packets.
            break;
         
         case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
            {
               PacketRequestUserWinLossResponse* response = 
                     static_cast<PacketRequestUserWinLossResponse*>( packetIn );
               for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->UserWinLoss( response->userUuid, response->winLoss );
               }
               assert( 0 );// not finished, wrong user data
            }
            break;
         }
      }
      break;
      case PacketType_Asset:
      {
         switch( packetIn->packetSubType )
         {
            case PacketAsset::AssetType_GetListOfAssetCategoriesResponse:
            {
              /* PacketAsset_GetListOfAssetCategoriesResponse* categoryList = 
                  static_cast<PacketAsset_GetListOfAssetCategoriesResponse*>( packetIn );

               int num = categoryList->assetcategory.size();
               m_assets.clear();

               for( int i=0; i< num; i++ )
               {
                  const string& name = categoryList->assetcategory[i];
                  AssetMapIter it = m_assets.find( name );
                  if( it == m_assets.end() )
                  {
                     m_assets.insert( AssetMapPair( name, AssetCollection () ) );
                  }
               }*/
            }
            break;

            case PacketAsset::AssetType_GetListOfAssetsResponse:
            {
              /* PacketAsset_GetListOfAssetsResponse* assetList = 
                  static_cast<PacketAsset_GetListOfAssetsResponse*>( packetIn );

               string& category = assetList->assetCategory;
               assert( category.size() > 0 );

               AssetMapIter it = m_assets.find( category );
               if( it == m_assets.end() )
               {
                  m_assets.insert( AssetMapPair( category, AssetCollection () ) );
                  it = m_assets.find( category );
               }

               AssetCollection& collection = it->second;
               collection.clear();


               SerializedKeyValueVector< AssetInfo >::KeyValueVectorIterator updatedAssetIt = assetList->updatedAssets.begin();
               while ( updatedAssetIt != assetList->updatedAssets.end() )
               {
                  AssetInfoExtended extender( updatedAssetIt->value );
                  collection.push_back( extender );
                  updatedAssetIt ++;
               }

               for( list< UserNetworkEventNotifier* >::iterator callbackIt = m_callbacks.begin(); callbackIt != m_callbacks.end(); ++callbackIt )
               {
                  (*callbackIt)->AssetManifestAvalable( category );
               }*/
            }
            break;
          }
      }
      break;
      case PacketType_Tournament:
      {
         switch( packetIn->packetSubType )
         {
            case PacketTournament::TournamentType_RequestListOfTournamentsResponse:
            {
               PacketTournament_RequestListOfTournamentsResponse* response = 
                  static_cast<PacketTournament_RequestListOfTournamentsResponse*>( packetIn );
               m_availableTournaments.clear();

               SerializedKeyValueVector< TournamentInfo >::KeyValueVectorIterator it = response->tournaments.begin();
               while ( it != response->tournaments.end() )
               {
                  TournamentInfo ti = it->value;
                  m_availableTournaments.push_back( ti );
                  it++;
               }
               if( m_availableTournaments.size() )
               {
                  for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
                  {
                     (*it)->TournamentListAvalable();
                  }
               }
               else
               {
                  cout << "No tournaments available" << endl;
               }
            }
            break;
            case PacketTournament::TournamentType_UserRequestsEntryInTournamentResponse:
            {
               PacketTournament_UserRequestsEntryInTournamentResponse* response = 
                  static_cast<PacketTournament_UserRequestsEntryInTournamentResponse*>( packetIn );
              
               for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->TournamentPurchaseResult( response->tournamentUuid, response->result );
               }
            }
            break;
         }
      }
      break;
   }

   return true;
}

//------------------------------------------------------------------------
//------------------------------------------------------------------------

void     NetworkLayer2::NotifyClientLoginStatus( bool isLoggedIn )
{
   for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
   {
      (*it)->UserLogin( isLoggedIn );
   }

   NotifyClientToBeginSendingRequests();   
}

//------------------------------------------------------------------------

void     NetworkLayer2::NotifyClientToBeginSendingRequests()
{
   if( m_isLoggedIn == false )
      return;

   if( m_selectedGame )
   {
      for( list< UserNetworkEventNotifier2* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
      {
         (*it)->ReadyToStartSendingRequestsToGame();
      }
   }
}

//------------------------------------------------------------------------

bool     NetworkLayer2::AddChatChannel( const ChatChannel& channel )
{
   vector< ChatChannel >::iterator it =  m_channels.begin();
   while( it != m_channels.end() )
   {
      if( it->uuid == channel.uuid )
      {
         return false;
      }
      it++;
   }
   m_channels.push_back( channel );
   return true;
}

//------------------------------------------------------------------------

bool     NetworkLayer2::GetAsset( const string& hash, AssetInfoExtended& asset )
{
   vector< AssetInfoExtended >::iterator it = m_staticAssets.begin();
   while( it != m_staticAssets.end() )
   {
      if( it->assetHash == hash )
      {
         asset = *it;
         return true;
      }
      it ++;
   }

   it = m_dynamicAssets.begin();
   while( it != m_dynamicAssets.end() )
   {
      if( it->assetHash == hash )
      {
         asset = *it;
         return true;
      }
      it ++;
   }

   return false;
}

bool     NetworkLayer2::UpdateAssetData( const string& hash, AssetInfoExtended& asset )
{
   vector< AssetInfoExtended >::iterator it = m_staticAssets.begin();
   while( it != m_staticAssets.end() )
   {
      if( it->assetHash == hash )
      {
         it->MoveData( asset );
         return true;
      }
      it ++;
   }

   it = m_dynamicAssets.begin();
   while( it != m_dynamicAssets.end() )
   {
      if( it->assetHash == hash )
      {
         it->MoveData( asset );
         return true;
      }
      it ++;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////////

void  RawDataAccumulator:: AddPacket( PacketGameplayRawData * ptr )
{
   numBytes += ptr->size;
   int index = ptr->index;
   if( ptr->index > 1 )// need to keep them ordered
   {
      deque< PacketGameplayRawData* >::iterator it = packetsOfData.begin();
      while( it != packetsOfData.end() )
      {
         if( (*it)->index < index )
         {
            packetsOfData.insert( it, ptr );
            return;
         }
         it ++;
      }
   }

   packetsOfData.push_back( ptr );
}

bool  RawDataAccumulator:: IsDone()
{
   if( packetsOfData.size() < 1 )
      return false;

   const PacketGameplayRawData * ptr = packetsOfData.back();
   if( ptr->index > 1 )// allows for 1 or 0. 1 is proper.
      return false;

   return true;
}

void  RawDataAccumulator:: PrepPackage( U8* & data, int& size )
{
   PacketFactory factory;
   assert( IsDone() );

   size = numBytes;
   data = new U8[size+1];
   U8* workingPointer = data;

   PacketGameplayRawData* packet = NULL;
   while( packetsOfData.size() )
   {
      packet = packetsOfData.front();
      packetsOfData.pop_front();
      memcpy( workingPointer, packet->data, packet->size );
      workingPointer += packet->size;

      BasePacket* temp = static_cast< BasePacket* >( packet );
      factory.CleanupPacket( temp );
   }
   data[size] = 0;


   numBytes = 0;
}

void  RawDataAccumulator:: PrepPackage( AssetInfoExtended& asset )
{
   PacketFactory factory;
   assert( IsDone() );
   asset.ClearData();

   asset.size = numBytes;
   asset.data = new U8[ asset.size + 1];
   U8* workingPointer = asset.data;

   PacketGameplayRawData* packet = NULL;
   while( packetsOfData.size() )
   {
      packet = packetsOfData.front();
      packetsOfData.pop_front();
      memcpy( workingPointer, packet->data, packet->size );
      workingPointer += packet->size;

      BasePacket* temp = static_cast< BasePacket* >( packet );
      factory.CleanupPacket( temp );
   }
   asset.data[ asset.size ] = 0;


   numBytes = 0;
}

//-----------------------------------------------------------------------------

// here for reference only
/*void     NetworkLayer::DumpFriends()
{
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   
   string text = "friends[ ";
   text += boost::lexical_cast< string >( m_friends.size() );
   text += " ] = {";
   m_pyro->Log( text );

   KeyValueVectorIterator it = m_friends.begin();
   while( it != m_friends.end() )
   {
      KeyValueSerializer<string>& kv = *it++;
      m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
      m_pyro->Log( kv.value, false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro->Log( " : ", false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
      m_pyro->Log( kv.key );
   }
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   m_pyro->Log( "}" );
}
//-----------------------------------------------------------------------------

void     NetworkLayer::DumpListOfGames( const GameList& gameList )
{
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   
   string text = "games: ";

   text += "[ ";
   text += boost::lexical_cast< string >( gameList.size() );
   text += " ] = {";
   m_pyro->Log( text );

   GameList::const_iterator it = gameList.begin();
   while( it != gameList.end() )
   {
      const PacketGameIdentification& gameId = *it++;
      m_pyro->SetConsoleColor( Pyroraptor::ColorsUsername );
      m_pyro->Log( gameId.name, false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro->Log( ", ", false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
      m_pyro->Log( gameId.shortName );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
      m_pyro->Log( " : ", false );
      m_pyro->SetConsoleColor( Pyroraptor::ColorsResponseText );
      m_pyro->Log( gameId.gameId );
   }
   m_pyro->SetConsoleColor( Pyroraptor::ColorsNormal );
   m_pyro->Log( "}" );
}*/
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
