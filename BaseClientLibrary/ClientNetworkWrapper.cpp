#include "../NetworkCommon/ServerConstants.h"

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning( disable:4996 )
#endif

#include "ClientNetworkWrapper.h"

#include "../NetworkCommon/Utils/Utils.h"
#include "../NetworkCommon/NetworkUtils.h"
#include "../NetworkCommon/Packets/CheatPacket.h"
#include "../NetworkCommon/Packets/UserStatsPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"
#include "../NetworkCommon/ChainedArchitecture/Thread.h"

#include <assert.h>
#include <iostream>
#include <boost/lexical_cast.hpp>
using namespace Mber;

#if PLATFORM == PLATFORM_WINDOWS
#pragma warning ( disable: 4996 )
#endif

///////////////////////////////////////////////////////////////////////////////////

struct GameData
{
   int size;
   U8* data;
};

///////////////////////////////////////////////////////////////////////////////////
//------------------------------------------------------------

ClientNetworkWrapper::ClientNetworkWrapper( U8 gameProductId, bool connectToAssetServer ): 
      m_normalSleepTime( 50 ),
      m_boostedSleepTime( 8 ),         
      m_selectedGame( 0 ),
      m_gameProductId( gameProductId ), 
      m_avatarId( 0 ),
      m_languageId( LanguageList_english ),
      m_showWinLossRecord( false ),
      m_marketingOptOut( false ),
      m_showGenderProfile( false ),
      m_displayOnlineStatusToOtherUsers( false ),
      m_blockContactInvitations( false ),
      m_blockGroupInvitations( false ),
      m_connectToAssetServer( connectToAssetServer ),
      m_isLoggingIn( false ),
      m_isLoggedIn( false ),
      m_isCreatingAccount( false ),
      m_printFunction( false ),
      m_connectionId( 0 ),       
      m_lastRawDataIndex( 0 ),
      m_loadBalancerPort( DefaultLoadBalancerPort ),
      m_wasCallbackForReadyToBeginSent( false ),
      m_requiresGatewayDiscovery( true ),
      m_readyToReconnectToGateway( false ),
      m_hasFinishedInitializing( true ),
      m_enabledMultithreadedProtections( false ),
      m_networkVersionOverride( 0 ),
      m_savedLoginInfo( NULL )
{
   PrintFunctionName( __FUNCTION__ );
   m_loadBalancerDns = "you.have.not.initialized.the.library.properly.com"; // just the default
   InitializeSockets();

   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      m_isThreadPerformanceBoosted[i] = false;
      m_timeWhenThreadPerformanceBoosted[i] = 0;
      m_fruitadens[i] = NULL;
      m_serverConnectionPort[i] = 0;
   }
}

ClientNetworkWrapper::~ClientNetworkWrapper()
{
   PrintFunctionName( __FUNCTION__ );
   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      delete m_fruitadens[i];
      m_fruitadens[i] = NULL;
   }

   Exit();
   ShutdownSockets();

   if( m_savedLoginInfo )
   {
      PacketCleaner cleaner( m_savedLoginInfo );
      m_savedLoginInfo = NULL;
   }
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool     ClientNetworkWrapper::IsConnected( bool isMainServer ) const
{
   // PrintFunctionName( __FUNCTION__ ); // too chatty
   if( m_hasFinishedInitializing == false )
      return false;

   if( isMainServer )
   {
      if( m_requiresGatewayDiscovery == true )
         return false;
      if( m_readyToReconnectToGateway == true )
         return false;
	  if( m_fruitadens[ ConnectionNames_Main ]->GetName() == "LoadBalancer" )
		  return false;

      bool isMainConnected = m_fruitadens[ ConnectionNames_Main ]->IsConnected();
	  //PrintFunctionName( __FUNCTION__ );
	  //cout << "Is main connected " << std::boolalpha << isMainConnected << std::noboolalpha << endl;
	  return isMainConnected;
   }
   else
   {
      if( m_connectToAssetServer == true )
         return m_fruitadens[ ConnectionNames_Asset ]->IsConnected();
      return false;
   }
}

//------------------------------------------------------------
//------------------------------------------------------------

void  ClientNetworkWrapper::EnableMultithreadedCallbackSystem()
{
   PrintFunctionName( __FUNCTION__ );
   m_enabledMultithreadedProtections = true;
}

void  ClientNetworkWrapper::CheckForReroutes( bool checkForReroutes )
{
   //m_checkForReroute = checkForReroutes;
}

//////////////////////////////////////////////////////////////////////////

void  ClientNetworkWrapper::CreateNetworkObjects()
{
   PrintFunctionName( __FUNCTION__ );
   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      if( m_fruitadens[i] != NULL )
         continue;

      m_fruitadens[i] = new Fruitadens( "Networking Layer", false );
      m_fruitadens[i]->SetSleepTime( m_normalSleepTime );
      m_fruitadens[i]->RegisterPacketHandlerInterface( this );
      m_serverConnectionPort[i] = 0;
      m_isThreadPerformanceBoosted[i] = false;
      m_timeWhenThreadPerformanceBoosted[i] = 0;
   }
}

//////////////////////////////////////////////////////////////////////////

void  ClientNetworkWrapper::Init( const char* serverDNS )
{
   PrintFunctionName( __FUNCTION__ );
   m_hasFinishedInitializing = true;
   CreateNetworkObjects();

   if( m_fruitadens[0]->IsSocketValid() || IsConnected() == true )
   {
      Disconnect();
   }

   if( m_networkVersionOverride )
   {
      for( int i=0; i< ConnectionNames_Num; i++ )
      {
         m_fruitadens[i]->SetNetworkVersionOverride( m_networkVersionOverride );
      }
   }

   
   m_wasCallbackForReadyToBeginSent = false;
   

   if( serverDNS && strlen( serverDNS ) > 5 )// arbitrary 5, but guaranteed to be a minimum requirement 
   {
      m_loadBalancerDns = serverDNS;
   }

   if( m_requiresGatewayDiscovery )
   {
      // we user this connection for talking to the load banacer temporily
      string tempName( "LoadBalancer" );
      m_fruitadens[ 0 ]->SetName( tempName );
      m_fruitadens[ 0 ]->Connect( m_loadBalancerDns.c_str(), m_loadBalancerPort );
      m_fruitadens[ 0 ]->RegisterPacketHandlerInterface( this );
   }
   else
   {
      ReconnectAfterTalkingToLoadBalancer();
   }
}

//////////////////////////////////////////////////////////////////////////

void     ClientNetworkWrapper::ReconnectAfterTalkingToLoadBalancer()
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      int mainIsPrepared = false;
      for( int i=0; i< ConnectionNames_Num; i++ )
      {
         if( m_connectToAssetServer == false && 
            i == ConnectionNames_Asset )
            continue;

         if( m_serverIpAddress[i].size() == 0 )
         {
            cout << "Missing server address entry.. cannot connect" << endl;
            continue;
         }
         if( i == ConnectionNames_Asset )
            cout << "Asset";
         else
            cout << "Main";

         cout << " Server address[" << i <<"] = " << m_serverIpAddress[i].c_str() << ":" << m_serverConnectionPort[i] << endl;
         m_fruitadens[i]->RegisterPacketHandlerInterface( this );
         if( i == ConnectionNames_Main && 
            m_serverIpAddress[i].size() > 0 )
         {
            string tempName( "MainGateway" );
            m_fruitadens[ ConnectionNames_Main ]->SetName( tempName );

            cout << "set m_readyToReconnectToGateway = false" << endl;
            m_readyToReconnectToGateway = false;
            mainIsPrepared = true;
         }
         else if( i == ConnectionNames_Asset && m_serverIpAddress[i].size() > 0 )
         {
            string tempName( "Asset" );
            m_fruitadens[ i ]->SetName( tempName );
         }

         cout << "New server name = " << m_fruitadens[i]->GetName() << endl;

         m_fruitadens[i]->Connect( m_serverIpAddress[i].c_str(), m_serverConnectionPort[i] );
      }

      
      if( mainIsPrepared == false )
      {
         cout << "m_serverIpAddress[main] is invalid" << endl;
      }
   }
}

void     ClientNetworkWrapper::Disconnect()
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == true )
   {
      for( int i=0; i< ConnectionNames_Num; i++ )
      {
         if( m_connectToAssetServer == false && 
            i == ConnectionNames_Asset )
            continue;

         m_fruitadens[ i ]->RegisterPacketHandlerInterface( NULL );
         m_fruitadens[ i ]->Disconnect();
      }     
   }
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::InitialConnectionCallback( const Fruitadens* connectionObject )
{ 
   PrintFunctionName( __FUNCTION__ );
   cout << "m_requiresGatewayDiscovery=" << boolalpha << m_requiresGatewayDiscovery << noboolalpha << endl;

   const string& name = connectionObject->GetName();
   cout << "This connection name = " << name << endl;

   bool isLoadBalancer = ( name == "LoadBalancer" );
   cout << "isLoadBalancer=" << boolalpha << isLoadBalancer << noboolalpha << endl;
   
   if( m_requiresGatewayDiscovery == true && // main stateful flag
      m_readyToReconnectToGateway == false &&
      isLoadBalancer == true ) // main thread does double duty here
   {
      PacketRerouteRequest* request = new PacketRerouteRequest;
      SerializePacketOut( request );
   }
   else
   {
      bool isMainGateway = ( name == "MainGateway" );
      cout << "isMainGateway=" << boolalpha << isMainGateway << noboolalpha << endl;
      if( isMainGateway )
      {
         PacketHello          hello;
         SerializePacketOut( &hello );

         if( m_savedLoginInfo != NULL )
         {
            SerializePacketOut( m_savedLoginInfo );
            cout << " InitialConnectionCallback sent login packet" << endl;
            PacketCleaner cleaner( m_savedLoginInfo );
            m_savedLoginInfo = NULL;
         }

         Notification( ClientSideNetworkCallback::NotificationType_HasBeenConnectedToGateway );
      }
      else
      {
         Notification( ClientSideNetworkCallback::NotificationType_AssetHasBeenConnectedToGateway );
      }
   }
   return false; 
}

bool     ClientNetworkWrapper::InitialDisconnectionCallback( const Fruitadens* connectionObject )
{ 
   PrintFunctionName( __FUNCTION__ );
   if( m_requiresGatewayDiscovery )
      return false;

   if( connectionObject->GetName() == "MainGateway" )
   {
      Notification( ClientSideNetworkCallback::NotificationType_HasBeenDisconnectedFromGateway );
   }
   else
   {
      Notification( ClientSideNetworkCallback::NotificationType_AssetHasBeenDisconnectedToGateway );
   }
   
   return false; 
}

//-----------------------------------------------------------------------------

void  ClientNetworkWrapper::Exit()
{
   PrintFunctionName( __FUNCTION__ );
   if( m_hasFinishedInitializing == false )
      return;

   Disconnect();

   m_hasFinishedInitializing = false;

   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      m_fruitadens[i]->Cleanup();
   }
   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      m_fruitadens[i] = NULL;
   }

   Threading::MutexLock    locker( m_notificationMutex );

   while( m_notifications.size() )
   {
      const QueuedNotification& qn = m_notifications.front();
      //SerializedKeyValueVector< string >::const_KVIterator keyValueIt = qn.genericKeyValuePairs.begin();
      PacketCleaner cleaner( qn.packet );

      delete [] qn.genericData;
      m_notifications.pop();
   }

   // clearing all values
   m_userName.clear(), m_attemptedUsername.clear();
   m_email.clear();
   m_uuid.clear();
   m_motto.clear();
   m_thisDeviceUuid.clear();
   m_selectedGame = 0;
   m_avatarId = 0;
   m_languageId = 0;

   m_showWinLossRecord = false;
   m_marketingOptOut = false;
   m_showGenderProfile = false;
   m_displayOnlineStatusToOtherUsers = false;
   m_blockContactInvitations = false;
   m_blockGroupInvitations = false;
   m_wasCallbackForReadyToBeginSent = false;
   //m_requiresGatewayDiscovery = true;
   m_enabledMultithreadedProtections = false;

   
   m_requiresGatewayDiscovery = true;
   m_readyToReconnectToGateway = false;
   for( int i=0; i< ConnectionNames_Num; i++ )
   {
      m_serverIpAddress[i].clear();
      m_serverConnectionPort[i] = 0;
   }
   

   m_invitationsReceived.clear();
   m_invitationsSent.clear();

   for( int i=0; i<Invitation::InvitationType_Num; i++ )
   {
      m_invitations[ i ].clear();
      m_invitationsToGroup[ i ].clear();
   }

   m_products.clear();
   m_devices.clear();
   m_contacts.clear();
   m_lastUserSearch.clear();
   m_channels.clear();
   m_gameList.clear();

   m_purchases.clear();
   m_otherUsersPurchases.clear();

   //m_gameProductId; // do not change this
   m_isLoggingIn = false;
   m_isLoggedIn = false;
   m_isCreatingAccount = false;
   m_connectionId = 0;
   m_lastLoggedOutTime.clear();
   m_lastRawDataIndex = 0;

   m_availableTournaments.clear();
   if( m_savedLoginInfo )
   {
      PacketCleaner cleaner( m_savedLoginInfo );
      m_savedLoginInfo = NULL;
   }
}

string   ClientNetworkWrapper::GenerateHash( const string& stringThatIWantHashed )
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

void     ClientNetworkWrapper::UpdateNotifications()
{
   // PrintFunctionName( __FUNCTION__ ); // too chatty
   if( m_callbacks.size() == 0 )
   {
      return;
   }
   Update();

   m_notificationMutex.lock();
   std::queue< QueuedNotification > localCopy = m_notifications;
   while( m_notifications.size() ) // clear
   {
      m_notifications.pop();
   }
   m_notificationMutex.unlock();
   
   while( localCopy.size() )
   {
      const QueuedNotification& qn = localCopy.front();
      SerializedKeyValueVector< string >::const_KVIterator keyValueIt = qn.genericKeyValuePairs.begin();
      PacketCleaner cleaner( qn.packet );
      
      for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
      {
         ClientSideNetworkCallback* notify = (*it);
         keyValueIt = qn.genericKeyValuePairs.begin(); // in case we have multiple interfaces

         switch( qn.eventId )
         {
         case ClientSideNetworkCallback::NotificationType_AreWeUsingCorrectNetworkVersion:
            {
               U32 serverVersion = boost::lexical_cast< U32 >( qn.intValue );
               notify->AreWeUsingCorrectNetworkVersion( GlobalNetworkProtocolVersion == serverVersion );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_FriendsUpdate:
            {
               notify->FriendsUpdate();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_FriendOnlineStatusChanged:
            {
               notify->FriendOnlineStatusChanged( keyValueIt->key );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_InvitationsReceivedUpdate:
            {
               notify->InvitationsReceivedUpdate();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_InvitationsSentUpdate:  
            {
               notify->InvitationsSentUpdate();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_InvitationReceived:
            {
               string uuid = keyValueIt->value; ++keyValueIt;
               const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
               SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
               while (it != kvVector.end() )
               {
                  if( it->key == uuid )
                  {
                     notify->InvitationReceived( it->value );
                  }
                  it ++;
               }
            }
            break;
         case ClientSideNetworkCallback::NotificationType_InvitationAccepted:
            {
               string from = keyValueIt->value; ++keyValueIt;
               string to = keyValueIt->value; ++keyValueIt;
               string accepted = keyValueIt->value; ++keyValueIt;

               notify->InvitationAccepted( from, to, accepted == "1" );
            }
            break;

         case ClientSideNetworkCallback::NotificationType_GenericInvitationsUpdated:
            {
               U32 invitationType = boost::lexical_cast< U32 >( qn.intValue );
               notify->GenericInvitationsUpdated( invitationType );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_GroupInvitationsUpdated:
            {
               U32 invitationType = boost::lexical_cast< U32 >( qn.intValue );
               notify->GroupInvitationsUpdated( invitationType );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_GenericInvitationRejected:
            {
               notify->GenericInvitationRejected();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_SearchResults:
            {
               notify->SearchForUserResultsAvailable();
            }
            break;

         case ClientSideNetworkCallback::NotificationType_TournamentListAvalable:
            {
               notify->TournamentListAvalable();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_TournamentPurchaseResult:
            {
               //SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
               // string from = keyValueIt->value; ++keyValueIt;
               //string to = keyValueIt->value; ++keyValueIt;
               string tournamentUuid = keyValueIt->value; ++keyValueIt;
               int result = boost::lexical_cast< U32 >( keyValueIt->value );
               notify->TournamentPurchaseResult( tournamentUuid, result );
            }
            break;

         case ClientSideNetworkCallback::NotificationType_OnError:
            {
               U32 code = boost::lexical_cast< U16>( qn.genericKeyValuePairs.find( "code" ) );
               U16 status = boost::lexical_cast< U16>( qn.genericKeyValuePairs.find( "status" ) );
               string text = qn.genericKeyValuePairs.find( "text" );
               notify->OnError( code, status, text.c_str() );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_QOS_ServiceChange:
            {
               Packet_QOS_ReportToClient* qosReport = static_cast< Packet_QOS_ReportToClient * > ( qn.packet );

               notify->QosChange( qosReport->errorText, qosReport->errorState, qosReport->param1, qosReport->param2 );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_UserLogin:
            {
               bool isLoggedIn = qn.intValue ? true: false; 
               notify->UserLogin( isLoggedIn );

               NotifyClientToBeginSendingRequests(); 
            }
            break;
         case ClientSideNetworkCallback::NotificationType_UserLogout:
            {
               notify->UserLogout();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ServerRequestsListOfUserPurchases:
            {
               notify->ServerRequestsListOfUserPurchases();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ListOfAggregateUserPurchases:
            {
               string userUuid = keyValueIt->value; 
               notify->ListOfAggregateUserPurchases( userUuid );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_UserProfileResponse:
            {
               bool notifyThatSelfUpdated = qn.intValue ? true: false; 
               PacketRequestUserProfileResponse* profile = static_cast<PacketRequestUserProfileResponse*>( qn.packet );
               
               notify->UserProfileResponse();
               if( notifyThatSelfUpdated )
                  notify->SelfProfileUpdate( true );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ListOfAvailableProducts:
            {
               notify->ListOfAvailableProducts();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_OtherUsersProfile:
            {
               map< string, string > profileKeyValues;
               while( keyValueIt != qn.genericKeyValuePairs.end() )
               {
                  profileKeyValues.insert( pair< string, string >( keyValueIt->key, keyValueIt->value ) );
                  keyValueIt++;
               }
               notify->OtherUsersProfile( profileKeyValues );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_SelfProfileUpdate:
            {
               bool success = qn.intValue ? true: false; 
               notify->SelfProfileUpdate( success );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_HasBeenConnectedToGateway:
            { 
               notify->HasBeenConnectedToGateway();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_HasBeenDisconnectedFromGateway:
            {
               notify->HasBeenDisconnectedFromGateway();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_AssetHasBeenConnectedToGateway:
            { 
               notify->HasBeenConnectedToAssetGateway();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_AssetHasBeenDisconnectedToGateway:
            {
               notify->HasBeenDisconnectedFromAssetGateway();
            }
            break;
            
         case ClientSideNetworkCallback::NotificationType_ChatListUpdate:
            {
               notify->ChatListUpdate();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ChatChannelUpdate:
            {
               notify->ChatChannelUpdate( keyValueIt->key );
            }
            break;

         case ClientSideNetworkCallback::NotificationType_ChatReceived:
            {
               string message = qn.genericKeyValuePairs.find( "message" );
               string channelUuid = qn.genericKeyValuePairs.find( "channelUuid" );
               string userUuid = qn.genericKeyValuePairs.find( "userUuid" );
               string timeStamp = qn.genericKeyValuePairs.find( "timeStamp" );
               U32 id = boost::lexical_cast< U32 >( qn.genericKeyValuePairs.find( "tempId" ) );
               notify->ChatReceived( message, channelUuid, userUuid, timeStamp, id );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ChatChannelHistory:
         case ClientSideNetworkCallback::NotificationType_ChatP2PHistory:
            {
               PacketChatHistoryResult* packetChatHistoryResult = 
                  static_cast<PacketChatHistoryResult*>( qn.packet );
               list< ChatEntry > listOfChats;
               int num = packetChatHistoryResult->chat.size();
               for( int i=0; i<num; i++ )
               {
                  listOfChats.push_back( packetChatHistoryResult->chat[i] );
               }
               bool invokeChannelNotifier = false;

               if( packetChatHistoryResult->chatChannelUuid.size() )
               {
                  notify->ChatChannelHistory( packetChatHistoryResult->chatChannelUuid, listOfChats );
               }
               else
               {
                  notify->ChatP2PHistory( packetChatHistoryResult->userUuid, listOfChats );
               }
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ChatHistoryMissedSinceLastLoginComposite:
            {
               PacketChatMissedHistoryResult* packetChatMissedHistoryResult = 
                  static_cast< PacketChatMissedHistoryResult* >( qn.packet );
               SerializedVector< MissedChatChannelEntry >& optimizedDataAccessHistory = packetChatMissedHistoryResult->history;
               int num = packetChatMissedHistoryResult->history.size();

               list< MissedChatChannelEntry > listOfChats;
               for( int i=0; i<num; i++ )
               {
                  listOfChats.push_back( optimizedDataAccessHistory[i] );
               }
               notify->ChatHistoryMissedSinceLastLoginComposite( listOfChats );
            }
            break;

         case ClientSideNetworkCallback::NotificationType_ReadyToStartSendingRequestsToGame:
            {
               notify->ReadyToStartSendingRequestsToGame();
            }
            break;

         case ClientSideNetworkCallback::NotificationType_NewChatChannelAdded:
            {
               string name = qn.genericKeyValuePairs.find( "name" );
               string uuid = qn.genericKeyValuePairs.find( "uuid" );
               string successString = qn.genericKeyValuePairs.find( "success" );

               bool success = false;
               if( successString == "1" )
                  success = true;
               notify->NewChatChannelAdded( name, uuid, success );
            }
            break;
   
         case ClientSideNetworkCallback::NotificationType_ChatChannelDeleted:
            {
               string uuid = qn.genericKeyValuePairs.find( "uuid" );
               string successString = qn.genericKeyValuePairs.find( "success" );

               bool success = false;
               if( successString == "1" )
                  success = true;
               notify->ChatChannelDeleted( uuid, success );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ChatChannel_UserAdded:
            {
               string channelName = qn.genericKeyValuePairs.find( "channelName" );
               string channelUuid = qn.genericKeyValuePairs.find( "channelUuid" );
               string userName = qn.genericKeyValuePairs.find( "userName" );
               string userUuid = qn.genericKeyValuePairs.find( "userUuid" );

               notify->ChatChannel_UserAdded( channelName, channelUuid, userName, userUuid );
            }
            break;

         case ClientSideNetworkCallback::NotificationType_ChatChannel_UserRemoved:
            {
               string channelUuid = qn.genericKeyValuePairs.find( "channelUuid" );
               string userUuid = qn.genericKeyValuePairs.find( "userUuid" );
               string successString = qn.genericKeyValuePairs.find( "success" );

               bool success = false;
               if( successString == "1" )
                  success = true;
               
               notify->ChatChannel_UserRemoved( channelUuid, userUuid, success );
            }
            break;

         case ClientSideNetworkCallback::NotificationType_ChatChannelMembers:
            {
               PacketChatListAllMembersInChatChannelResponse* packetChatHistoryResult = 
                  static_cast<PacketChatListAllMembersInChatChannelResponse*>( qn.packet );

               ChatChannel channel;
               GetChannel( packetChatHistoryResult->chatChannelUuid, channel );

               notify->ChatChannelMembers( channel.channelName,  packetChatHistoryResult->chatChannelUuid, packetChatHistoryResult->userList );
            }
            break;
               
         case ClientSideNetworkCallback::NotificationType_AssetCategoriesLoaded:
            {
               notify->AssetCategoriesLoaded();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_AssetManifestAvailable:
            {
               notify->AssetManifestAvailable( keyValueIt->key );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_GameData:
            {
               if( qn.genericData )
               {
                  notify->GameData( qn.intValue, qn.genericData );
               }
            }
            break;
         case ClientSideNetworkCallback::NotificationType_AssetDataAvailable:
            {
               string hash = qn.genericKeyValuePairs.find( "hash" );
               string category = qn.genericKeyValuePairs.find( "category" );

               notify->AssetDataAvailable( category, hash );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ListOfDevicesUpdated:
            {
               notify->ListOfDevicesUpdated();
            }
            break;
         case ClientSideNetworkCallback::NotificationType_DeviceIDUpdated:
            {
               notify->UpdatedDeviceID();
            }
               break;
         case ClientSideNetworkCallback::NotificationType_DeviceRemoved:
            {
               string channelUuid = qn.genericKeyValuePairs.find( "channelUuid" );
               string successString = qn.genericKeyValuePairs.find( "success" );

               bool success = false;
               if( successString == "1" )
                  success = true;

               notify->DeviceRemoved( channelUuid, success );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_ProductsForSale:
            {                  
               PacketPurchase_RequestListOfSalesResponse* purchase = 
                  static_cast<PacketPurchase_RequestListOfSalesResponse*>( qn.packet );

               notify->ProductsForSale( purchase->thingsForSale );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_PurchaseSuccess:
            {                  
               PacketPurchase_BuyResponse* purchase = 
                  static_cast<PacketPurchase_BuyResponse*>( qn.packet );

               notify->PurchaseSuccess( purchase->purchaseUuid, purchase->success );
            }
            break;

         case  ClientSideNetworkCallback::NotificationType_PurchaseReceiptResponse:
            {
               PacketPurchase_ValidatePurchaseReceiptResponse* purchase = 
                  static_cast<PacketPurchase_ValidatePurchaseReceiptResponse*>( qn.packet );
               notify->PurchaseReceiptResponse( purchase->transactionId, purchase->errorCode );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_UserStats:
            {
               PacketUserStats_RequestListOfUserStatsResponse* response = 
                  static_cast<PacketUserStats_RequestListOfUserStatsResponse*>( qn.packet );
               notify->UserStats( response->userUuid, response->stats, response->whichGame );
            }
            break;
         case ClientSideNetworkCallback::NotificationType_UserProfileStats:
            {
               PacketUserStats_RequestUserProfileStatsResponse* response =
                  static_cast<PacketUserStats_RequestUserProfileStatsResponse*>( qn.packet );
               notify->UserProfileStats( response->profileUserId, response->gameType, response->userProfileStats );
            }
            break;

         }
      }

      delete [] qn.genericData;
      localCopy.pop();
                  
   }

   CleanupLingeringMemory();
}

void     ClientNetworkWrapper::CleanupLingeringMemory()
{
   //PrintFunctionName( __FUNCTION__ );// too chatty
   vector< string > categories;
   GetAssetCategories( categories );

   AssetMapIter it = m_assets.begin();
   while( it != m_assets.end() )
   {
      AssetCollection& collection = it->second;
      it++;
      AssetCollection::iterator itCollection = collection.begin();
      while ( itCollection != collection.end() )
      {
         AssetInfoExtended& asset = *itCollection++;
         if( asset.NeedsClearing() == true )
         {
            cout << "memory cleanup for name: " << asset.assetName << endl;
            asset.ClearTempData();
         }
      }

   }
}

//------------------------------------------------------------

void     ClientNetworkWrapper::InheritedUpdate()  // depricated... remove me
{
   SendNotifications();
}

//------------------------------------------------------------

void     ClientNetworkWrapper::SendNotifications()
{
   if( m_enabledMultithreadedProtections == true )
   {
      return;
   }

   UpdateNotifications();
}

//------------------------------------------------------------
//------------------------------------------------------------

void     ClientNetworkWrapper::Notification( int type )
{
   Threading::MutexLock    locker( m_notificationMutex );
   m_notifications.push( QueuedNotification( type ) );
}

void     ClientNetworkWrapper::Notification( int type, U8* genericData, int size )
{
   QueuedNotification temp( type );
   temp.genericData = genericData;
   temp.intValue = size;
   Threading::MutexLock    locker( m_notificationMutex );
   m_notifications.push( temp );
}

void     ClientNetworkWrapper::Notification( int type, BasePacket* meta )
{
   Threading::MutexLock    locker( m_notificationMutex );
   m_notifications.push( QueuedNotification( type, meta ) );
}

void     ClientNetworkWrapper::Notification( int type, int data, int meta )
{
   QueuedNotification qn( type );
   //qn.genericKeyValuePairs.insert( boost::lexical_cast<string>( data ), boost::lexical_cast<string>( meta ) );
   qn.intValue = data;
   qn.intValue2 = meta;

   Threading::MutexLock    locker( m_notificationMutex );
   m_notifications.push( qn );
}

void     ClientNetworkWrapper::Notification( int type, const string& data )
{
   QueuedNotification qn( type );
   qn.genericKeyValuePairs.insert( data, data );

   Threading::MutexLock   locker( m_notificationMutex );
   m_notifications.push( qn );
}
   
void     ClientNetworkWrapper::Notification( int type, const string& data, const string& data2 )
{
   QueuedNotification qn( type );
   qn.genericKeyValuePairs.insert( data, data2 );

   Threading::MutexLock   locker( m_notificationMutex );
   m_notifications.push( qn );
}

void     ClientNetworkWrapper::Notification( int type, SerializedKeyValueVector< string >& strings )
{
   QueuedNotification qn( type );
   qn.genericKeyValuePairs = strings;

   Threading::MutexLock   locker( m_notificationMutex );
   m_notifications.push( qn );
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

bool  ClientNetworkWrapper::RequestLogin( const string& userName, const string& password, const string& languageCode )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      //Init( NULL );
      ReconnectAfterTalkingToLoadBalancer();
   }
   if( m_savedLoginInfo )
   {
      PacketCleaner cleaner( m_savedLoginInfo );
      m_savedLoginInfo = NULL;
   }

   cout << " RequestLogin : " << userName << endl;

   PacketLogin* login = new PacketLogin;
   login->loginKey = "deadbeef";// currently unused
   login->uuid = userName;
   login->userName = userName;
   login->loginKey = password;
   login->languageCode = languageCode;
   login->password = boost::lexical_cast< string >( CreatePasswordHash( password.c_str() ) );
   m_attemptedUsername = userName;

   m_isLoggingIn = true;

   if( m_requiresGatewayDiscovery == false &&
      SerializePacketOut( login ) == true )
   {
      PacketCleaner cleaner( login );
      cout << " RequestLogin send packet" << endl;
   }
   else
   {
      m_savedLoginInfo = login;
      cout << " RequestLogin saved packet to send later" << endl;
   }

   return true;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestAccountCreate( const string& userName, const string& userEmail, const string& password, int languageId, const string& deviceId, const string& gkHash )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      //Init();
      ReconnectAfterTalkingToLoadBalancer();
   }
   assert( m_isLoggedIn == false );

   PacketCreateAccount* createAccount = new PacketCreateAccount;
   createAccount->userName = userName;
   createAccount->userEmail = userEmail;
   createAccount->password = boost::lexical_cast< string >( CreatePasswordHash( password.c_str() ) );
   createAccount->deviceId = deviceId;
   createAccount->deviceAccountId = "";
   if( gkHash.size() != 0 )
   {
      createAccount->deviceAccountId = boost::lexical_cast< string >( CreatePasswordHash( gkHash.c_str() ) );
   }
   createAccount->languageId = languageId;

   m_attemptedUsername = userName;

   m_isCreatingAccount = true;
   //SerializePacketOut( &createAccount );

   if( m_requiresGatewayDiscovery == false &&
      SerializePacketOut( createAccount ) == true )
   {
      PacketCleaner cleaner( createAccount );
      cout << " RequestAccountCreate send packet" << endl;
   }
   else
   {
      m_savedLoginInfo = createAccount;
      cout << " RequestAccountCreate saved packet to send later" << endl;
   }

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestLogout() const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketLogout logout;
   SerializePacketOut( &logout );

   return true;
}

bool ClientNetworkWrapper::ForceLogout()
{
   PrintFunctionName( __FUNCTION__ );
   m_isLoggingIn = false;
   m_isLoggedIn = false;
   
   if( IsConnected() == false )
   {
      return false;
   }
   PacketLogout logout;
   SerializePacketOut( &logout );
   
   Disconnect();
   
   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestProfile( const string userName )//if empty, profile for currently logged in user is used
{
   PrintFunctionName( __FUNCTION__ );
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

bool  ClientNetworkWrapper::RequestUserStats( const string& useruuid, U8 gameId )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketUserStats_RequestListOfUserStats request;
   request.userUuid = useruuid;
   request.whichGame = gameId;

   SerializePacketOut( &request );

   return true;
}

//-----------------------------------------------------------------------------
bool ClientNetworkWrapper::RequestUserProfileStats( unsigned int profileUserId, U8 gameType )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   
   PacketUserStats_RequestUserProfileStats request;
   request.gameType = gameType;
   request.profileUserId = profileUserId;
   
   SerializePacketOut( &request );
   
   return true;
}

bool ClientNetworkWrapper::RequestGameProfile( unsigned int profileUserId, unsigned int requestUserId, U8 gameType  )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   
   PacketUserStats_RequestGameProfile request;
   request.gameType = gameType;
   request.profileUserId = profileUserId;
   request.requestUserId = requestUserId;
   
   SerializePacketOut( &request );
   
   return true;
}

//-----------------------------------------------------------------------------


bool ClientNetworkWrapper::RequestGamePlayerFactionStats( unsigned int userId, U8 gameType )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   
   PacketUserStats_RequestGameFactionStats request;
   request.gameType = gameType;
   request.userId = userId;
   
   SerializePacketOut( &request );
   
   return true;
}

//-----------------------------------------------------------------------------

bool ClientNetworkWrapper::RequestGameGlobalFactionStats( U8 gameType )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   
   PacketUserStats_RequestGameFactionStats request;
   request.gameType = gameType;
   request.userId = 0;
   
   SerializePacketOut( &request );
   
   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::ThrottleConnection( U8 level )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketLoginDebugThrottleUserPackets throttle;
   throttle.level = level;
   SerializePacketOut( &throttle );

   return true;
}
//-----------------------------------------------------------------------------

void  ClientNetworkWrapper::FillProfileChangeRequest( PacketUpdateSelfProfile& profile ) const 
{
   PrintFunctionName( __FUNCTION__ );
   profile.userName = m_userName;
   profile.email = m_email;
   //profile.

   profile.languageId = m_languageId;
   profile.avatarIconId = m_avatarId;
   profile.motto = m_motto;
   profile.showWinLossRecord = m_showWinLossRecord;
   profile.marketingOptOut = m_marketingOptOut;
   profile.showGenderProfile = m_showGenderProfile;
   profile.displayOnlineStatusToOtherUsers = m_displayOnlineStatusToOtherUsers;
   profile.blockContactInvitations = m_blockContactInvitations;
   profile.blockGroupInvitations = m_blockGroupInvitations;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestChangeAvatarId( int newId ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketUpdateSelfProfile profile;
   FillProfileChangeRequest( profile );

   profile.avatarIconId = newId;

   SerializePacketOut( &profile );

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestOtherUserInGameProfile( const string& userName, bool fullProfile ) // friends, games list, etc
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false || userName.size() == 0 )
   {
      return false;
   }
   PacketRequestOtherUserProfile request;
   request.userName = userName;
   request.fullProfile = fullProfile;

   SerializePacketOut( &request );

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestChatChannelList( )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketChatChannelListRequest request;
   SerializePacketOut( &request );
   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::AdminUpdateUserProfile( const string userName, const string& email, const string& userUuid, int adminLevel, int languageId, bool isActive, bool showWinLossRecord, bool marketingOptOut, bool showGenderProfile )
{
   PrintFunctionName( __FUNCTION__ );
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
   /*update.showWinLossRecord =    showWinLossRecord;
   update.marketingOptOut =      marketingOptOut;
   update.showGenderProfile =    showGenderProfile;*/

   SerializePacketOut( &update );

   return true;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::UpdateOwnProfile( const string& userName, const string& email, const string& motto )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }

   PacketUpdateSelfProfile update;
   FillProfileChangeRequest( update );
   update.userName =             userName;
   update.email =                email;
   update.motto =                motto;

   SerializePacketOut( &update );

   return true;
}

bool     ClientNetworkWrapper::SetLanguage( int languageId )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   if( languageId < LanguageList_english || languageId >= LanguageList_count )
      return false;

   PacketUpdateSelfProfile update;
   FillProfileChangeRequest( update );
   update.languageId =           languageId;
   SerializePacketOut( &update );

   return true;
}
bool     ClientNetworkWrapper::SetMarketingOptOut ( bool marketingOptOut )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }

   PacketUpdateSelfProfile update;
   FillProfileChangeRequest( update );
   update.marketingOptOut =           marketingOptOut;
   SerializePacketOut( &update );

   return true;
}

bool     ClientNetworkWrapper::SetShowWinLossRecord( bool winLossShow )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }

   PacketUpdateSelfProfile update;
   FillProfileChangeRequest( update );
   update.showWinLossRecord =           winLossShow;
   SerializePacketOut( &update );

   return true;
}

bool     ClientNetworkWrapper::SetShowGenderProfile( bool showGenderProfile )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }

   PacketUpdateSelfProfile update;
   FillProfileChangeRequest( update );
   update.showGenderProfile =           showGenderProfile;
   SerializePacketOut( &update );

   return true;
}

bool     ClientNetworkWrapper::SetDisplayOnlineStatusToOtherUsers( bool display )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }

   PacketUpdateSelfProfile update;
   FillProfileChangeRequest( update );
   update.displayOnlineStatusToOtherUsers =           display;
   SerializePacketOut( &update );

   return true;
}

bool     ClientNetworkWrapper::SetBlockContactInvitations( bool block )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }

   PacketUpdateSelfProfile update;
   FillProfileChangeRequest( update );
   update.blockContactInvitations =           block;
   SerializePacketOut( &update );

   return true;
}

bool     ClientNetworkWrapper::SetBlockGroupInvitations( bool block )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }

   PacketUpdateSelfProfile update;
   FillProfileChangeRequest( update );
   update.blockGroupInvitations =           block;
   SerializePacketOut( &update );

   PacketChat_UserProfileChange chatProfile;
   chatProfile.blockChannelInvites = block;
   SerializePacketOut( &chatProfile );

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestListOfContacts() const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketContact_GetListOfContacts friends;
   SerializePacketOut( &friends );

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::GetAvailableProduct( int index, ProductBriefPacketed& product ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( index < 0 || index >= m_products.size() )
   {
      //product.quantity = -1;
      product.vendorUuid.clear();
      return false;
   }

   product = m_products[index];
   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::FindProductByVendorUuid( const string& vendorUuid, ProductBriefPacketed& product ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( vendorUuid.size() == 0 )
   {
      product.vendorUuid.clear();
      product.uuid.clear();
      return false;
   }


   int num = m_products.size();
   for( int i=0; i<num; i++ )
   {
      const ProductBriefPacketed& pb = m_products[i];
      if( pb.vendorUuid == vendorUuid )
      {
         product = pb;
         return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::FindProduct( const string& uuid, ProductBriefPacketed& product ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( uuid.size() == 0 )
   {
      product.vendorUuid.clear();
      product.uuid.clear();
      return false;
   }


   int num = m_products.size();
   for( int i=0; i<num; i++ )
   {
      const ProductBriefPacketed& pb = m_products[i];
      if( pb.uuid == uuid )
      {
         product = pb;
         return true;
      }
   }

   return false;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestListOfAppStoreProducts() const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketRequestListOfProducts products;
   SerializePacketOut( &products );

   return true;
}

//-----------------------------------------------------------------------------
/*
bool  ClientNetworkWrapper::RequestListOfProducts() const
{
   if( IsConnected() == false )
   {
      return false;
   }
   PacketRequestListOfProducts products;
   SerializePacketOut( &products );

   return true;
}
*/
//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestListOfMberProducts() const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketPurchase_RequestListOfSales products;
   SerializePacketOut( &products );

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestListOfPurchases( const string userUuid ) const
{
   PrintFunctionName( __FUNCTION__ );
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

bool  ClientNetworkWrapper::MakePurchase( const string& exchangeUuid ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketPurchase_Buy purchase;
   purchase.purchaseUuid = exchangeUuid;
   SerializePacketOut( &purchase );

   return true;
}

bool  ClientNetworkWrapper::VerifyVendorPurchase( const string& purchaseItemId, int quantity, const string& transactionId, const string& receipt, int platformId ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketPurchase_ValidatePurchaseReceipt receiptPacket;
   receiptPacket.purchaseItemId = purchaseItemId;
   receiptPacket.quantity = quantity;
   receiptPacket.transactionId = transactionId;
   receiptPacket.receipt = receipt;
   receiptPacket.platformId = platformId;

   SerializePacketOut( &receiptPacket );

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::UserHasMadePurchase( const string& vendorProductUuid, const string& receipt, int platformId ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
 /*  PacketPurchase_Buy purchase;
   purchase.purchaseUuid = exchangeUuid;
   SerializePacketOut( &purchase );*/

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::RequestListOfTournaments()
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketTournament_RequestListOfTournaments tournaments;
   SerializePacketOut( &tournaments );// only requests tournaments for the current game

   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::GetPurchase( int index, PurchaseEntry& purchase ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( index < 0 || index >= m_purchases.size() )
   {
      purchase.quantity = -1;
      //purchase.name = "";
      return false;
   }

   purchase.productUuid = m_purchases[index].productUuid;
   purchase.quantity = m_purchases[index].quantity;
   return true;

}

bool  ClientNetworkWrapper::GetPurchaseOtherUser( int index, PurchaseEntry& purchase ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( index < 0 || index >= m_otherUsersPurchases.size() )
   {
      purchase.quantity = -1;
      //purchase.name = "";
      return false;
   }

   //purchase = m_otherUsersPurchases[index];
   purchase.productUuid = m_otherUsersPurchases[index].productUuid;
   purchase.quantity = m_otherUsersPurchases[index].quantity;
   return true;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::GetPurchasesByType( vector< PurchaseEntry >& purchases, GameProductType productType )
{
   PrintFunctionName( __FUNCTION__ );
   int num = m_purchases.size();
   for( int i=0; i<num; i++ )
   {
      const string& uuid = m_purchases[i].productUuid;
      ProductBriefPacketed brief;
      if( FindProduct( uuid, brief ) == true )
      {
         if( brief.productType == productType )
         {
            PurchaseEntry pe;
            pe.productUuid = m_purchases[i].productUuid;
            pe.quantity = m_purchases[i].quantity;
            purchases.push_back( pe );
         }
      }
   }
   if( purchases.size() )
      return true;

   return false;
}

//-----------------------------------------------------------------------------

bool  ClientNetworkWrapper::PurchaseEntryIntoTournament( const string& tournamentUuid, const vector<PurchaseServerDebitItem>& listOfDebitItems, const string& customDeck )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }

   PacketTournament_UserRequestsEntryInTournament entry;
   entry.tournamentUuid = tournamentUuid;
   entry.customDeck = customDeck;

   int num = listOfDebitItems.size();
   for( int i=0; i<num; i++ )
   {
      entry.itemsToSpend.push_back( listOfDebitItems[i] );
   }

   SerializePacketOut( &entry );

   return true;
}

//-----------------------------------------------------------------------------
  
bool  ClientNetworkWrapper::RequestChatChannelHistory( const string& channelUuid, int numRecords, int startingIndex, const char* startingTimestamp ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketChatHistoryRequest history;
   history.chatChannelUuid = channelUuid;
   history.numRecords = numRecords;
   history.startingIndex = startingIndex;
   if( startingTimestamp != NULL )
   {
      history.startingTimestamp = startingTimestamp;
   }
   SerializePacketOut( &history );
   return true;
}

//-----------------------------------------------------------------------------
  
bool  ClientNetworkWrapper::RequestChatP2PHistory( const string& userUuid, int numRecords, int startingIndex, const char* startingTimestamp ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return false;
   }
   PacketChatHistoryRequest history;
   history.userUuid = userUuid;
   history.numRecords = numRecords;
   history.startingIndex = startingIndex;
   if( startingTimestamp != NULL )
   {
      history.startingTimestamp = startingTimestamp;
   }
   SerializePacketOut( &history );
   return true;
}

//-----------------------------------------------------------------------------
  
void     ClientNetworkWrapper::AddNotationToContact( const string& uuid, bool isFavorite, const string& message )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsConnected() == false )
   {
      return;
   }

   UserNameKeyValue::KVIterator  itFriends;
   bool found = false;
   {
      Threading::MutexLock    locker( m_notificationMutex );

      itFriends = m_contacts.begin();
      while( itFriends != m_contacts.end() )
      {
         if( itFriends->key == uuid )
         {
            itFriends->value.markedAsFavorite = isFavorite;
            itFriends->value.notesAboutThisUser = message;
            found = true;
            break;
         }
         itFriends++;
      }
   }

   if( found )
   {
      PacketContact_SetNotationOnUser notation;
      notation.uuid = uuid;
      //notation.friendInfo = itFriends->value;
      notation.friendInfo.markedAsFavorite = isFavorite;
      notation.friendInfo.notesAboutThisUser = message;
      SerializePacketOut( &notation );
   }
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

string   ClientNetworkWrapper::FindContact( const string& name ) const 
{
   PrintFunctionName( __FUNCTION__ );
   UserNameKeyValue::const_KVIterator  itFriends = m_contacts.begin();
   while( itFriends != m_contacts.end() )
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

string   ClientNetworkWrapper::FindContactFromUuid( const string& uuid ) const 
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );
   UserNameKeyValue::const_KVIterator  itFriends = m_contacts.begin();
   while( itFriends != m_contacts.end() )
   {
      if( itFriends->key == uuid )
      {
         return itFriends->value.userName;
      }
      itFriends++;
   }
   if( uuid == m_uuid )// this method is commonly used as a lookup.. let's make it simple to use
      return m_userName;

   return string();
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetContact( int index, BasicUser& user )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );
   if( index < 0 || index >= m_contacts.size() )
   {
      user.userName.clear();
      user.UUID.clear();
      return false;
   }

   int i = 0;
   UserNameKeyValue::const_KVIterator  itFriends = m_contacts.begin();
   while( itFriends != m_contacts.end() )
   {
      if( i == index )
      {
         user = itFriends->value;
         return true;
      }
      i ++;
      itFriends++;
   }

   return false;
}


//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetContact( const string& uuid, BasicUser& user )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

   UserNameKeyValue::const_KVIterator  itFriends = m_contacts.begin();
   while( itFriends != m_contacts.end() )
   {
      if( itFriends->key == uuid )
      {
         user = itFriends->value;
         return true;
      }
      itFriends++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::IsContact( const string& userUuid )
{
   PrintFunctionName( __FUNCTION__ );
   if( userUuid.size() < 2 )
   {
      return false;
   }

   Threading::MutexLock    locker( m_notificationMutex );

   UserNameKeyValue::const_KVIterator  itFriends = m_contacts.begin();
   while( itFriends != m_contacts.end() )
   {
      if( itFriends->key == userUuid )
      {
         return true;
      }
      itFriends++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::IsContactByName( const string& userName )
{
   PrintFunctionName( __FUNCTION__ );

   Threading::MutexLock    locker( m_notificationMutex );
   if( userName.size() < 2 )
   {
      return false;
   }

   UserNameKeyValue::const_KVIterator  itFriends = m_contacts.begin();
   while( itFriends != m_contacts.end() )
   {
      if( itFriends->value.userName == userName )
      {
         return true;
      }
      itFriends++;
   }
   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetUserSearchResult( int index, BasicUser& user )
{
   PrintFunctionName( __FUNCTION__ );
   
   Threading::MutexLock    locker( m_notificationMutex );

   if( index < 0 || index >= m_lastUserSearch.size() )
   {
      user.userName = "";
      user.UUID = "";
      return false;
   }

   int i = 0;
   UserNameKeyValue::const_KVIterator  itFound = m_lastUserSearch.begin();
   while( itFound != m_lastUserSearch.end() )
   {
      if( i == index )
      {
         user = itFound->value;
         return true;
      }
      i ++;
      itFound++;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetChannel( const string& channelUuid, ChatChannel& channel ) const
{
   /*ChatChannel tempChannel;
   bool    result = FindChannel( channelUuid, tempChannel );
   //channel = NULL;
   if( result == true )
   {
      channel = tempChannel;
      return true;
   }

   return false;*/

   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

   vector< ChatChannel >::const_iterator itChannels = m_channels.begin();
   while( itChannels != m_channels.end() )
   {
      if( channelUuid == itChannels->uuid )
      {
         channel = *itChannels;
         return true;
      }

      itChannels++;
   }

   return false;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool    ClientNetworkWrapper::GetChannel( int index, ChatChannel& channel ) const
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

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

vector< ChatChannel >::iterator    ClientNetworkWrapper::GetChannel( const string& channelUuid )
{
   // Threading::MutexLock    locker( m_notificationMutex ); // protected

   PrintFunctionName( __FUNCTION__ );
   vector< ChatChannel >::iterator itChannels = m_channels.begin();
   while( itChannels != m_channels.end() )
   {
      if( channelUuid == itChannels->uuid )
      {
         return itChannels;
      }
      itChannels++;
   }

   return m_channels.end();
}

bool     ClientNetworkWrapper::RemoveChannel( const string& channelUuid )
{
   PrintFunctionName( __FUNCTION__ );
   vector< ChatChannel >::iterator itChannels = m_channels.begin();
   while( itChannels != m_channels.end() )
   {
      if( channelUuid == itChannels->uuid )
      {
         m_channels.erase (itChannels);
         return true;
      }

      itChannels++;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::IsGameChannel( const string& channelUuid ) const
{
   PrintFunctionName( __FUNCTION__ );
   ChatChannel channel;
   if( GetChannel( channelUuid, channel ) == false )
      return false;

   if( channel.gameProductId != 0 )
      return true;

   return false;
}

//-----------------------------------------------------------------------------

int      ClientNetworkWrapper::GetAssetCategories( vector< string >& categories ) const
{
   //PrintFunctionName( __FUNCTION__ );// chatty
   Threading::MutexLock    locker( m_notificationMutex );

   AssetMapConstIter it = m_assets.begin();
   while( it != m_assets.end() )
   {
      categories.push_back( it->first );
      it++;
   }

   return categories.size();
}

//-----------------------------------------------------------------------------

int      ClientNetworkWrapper::GetNumAssets( const string& category )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

   AssetMapConstIter it = m_assets.find( category );
   if( it == m_assets.end() )
      return 0;
   return it->second.size();
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetAssetInfo( const string& category, int index, AssetInfoExtended& assetInfo )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

   AssetMapConstIter it = m_assets.find( category );
   if( it == m_assets.end() )
      return false;
   
   if( index < 0 || index >= static_cast< int >( it->second.size() ) )
      return false;
   assetInfo = it->second[ index ];
   return true;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetAvatarAsset( U32 index, AssetInfoExtended& assetInfo )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

   AssetMapConstIter it = m_assets.find( "avatar" );
   if( it == m_assets.end() )
      return false;

   string assetName = boost::lexical_cast< string >( index );
   const string assetHash = GenerateHash( assetName );

   const vector< AssetInfoExtended >& arrayOfAssets = it->second;

   vector< AssetInfoExtended >::const_iterator itAssets = arrayOfAssets.begin();
   while( itAssets != arrayOfAssets.end() )
   {
      if( itAssets->assetHash == assetHash )
      {
         assetInfo = *itAssets;
         return true;
      }
      itAssets++;
   }

   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper:: ClearAssetInfo( const string& category, const string& hash )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

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

bool    ClientNetworkWrapper::GetTournamentInfo( int index, TournamentInfo& tournamentInfo )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

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
/*
bool     ClientNetworkWrapper::RequestListOfGames() const
{
   PacketRequestListOfGames listOfGames;
   SerializePacketOut( &listOfGames );

   return true;
}*/

//-----------------------------------------------------------------------------
/*
bool     ClientNetworkWrapper::RequestListOfUsersLoggedIntoGame( ) const
{
   assert( 0 );// not finished
   return true;
}*/

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestUserWinLossRecord( const string& userName ) const
{
   PrintFunctionName( __FUNCTION__ );
   PacketRequestUserWinLoss packet;
   packet.userUuid;
   packet.gameUuid;
   assert( 0 );// not finished
   return true;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestListOfInvitationsSentForContacts() const
{
   PrintFunctionName( __FUNCTION__ );
   PacketContact_GetListOfInvitationsSent    invitationRequest;
   return SerializePacketOut( &invitationRequest );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestListOfInvitationsReceivedForContacts() const
{
   PrintFunctionName( __FUNCTION__ );
   PacketContact_GetListOfInvitations    invitationRequest;
   return SerializePacketOut( &invitationRequest );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::AcceptInvitationForContacts( const string& uuid ) const
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->key == uuid )
      {
         PacketContact_AcceptInvite invitationAccepted;
         invitationAccepted.invitationUuid = uuid;
        
         //return SerializePacketOut( &invitationAccepted );
         bool  result = SerializePacketOut( &invitationAccepted );
//         RequestListOfInvitationsReceived();
         return result;
      }
      it++;
   }
   return false;
}
/*
//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::AcceptInvitationFromUsername( const string& userName ) const
{
   Threading::MutexLock    locker( m_notificationMutex );

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
}*/

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::DeclineInvitationForContacts( const string& uuid, string message ) const
{
   PrintFunctionName( __FUNCTION__ );
   PacketContact_DeclineInvitation invitationDeclined;
   invitationDeclined.invitationUuid = uuid;
   invitationDeclined.message = message;

   bool  result = SerializePacketOut( &invitationDeclined );
//   RequestListOfInvitationsReceived();
   return result;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RemoveSentInvitationForContacts( const string& uuid ) const
{
   PrintFunctionName( __FUNCTION__ );
   bool found = false;
   Threading::MutexLock    locker( m_notificationMutex );

   const SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsSent;
   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->key == uuid )
      {
         found = true;
         break;
      }
      it++;
   }
   if( found == false )
      return false;


   PacketContact_RemoveInvitation invitation;
   invitation.invitationUuid = uuid;

   bool  result = SerializePacketOut( &invitation );
//   RequestListOfInvitationsSent();
   return result;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestListOfInvitations( Invitation::InvitationType type ) const
{
   PrintFunctionName( __FUNCTION__ );
   PacketInvitation_GetListOfInvitations invitationRequest;
   invitationRequest.invitationType = type;
   
   return SerializePacketOut( &invitationRequest );
}

bool     ClientNetworkWrapper::AcceptInvitation( const string& uuid, Invitation::InvitationType type ) const
{
   PrintFunctionName( __FUNCTION__ );
   PacketInvitation_AcceptInvitation accept;
   //todo... validate that this is a valid invitation
   accept.invitationUuid = uuid;
   accept.invitationType = type;

   return SerializePacketOut( &accept );
}

bool     ClientNetworkWrapper::DeclineInvitation( const string& uuid, string message, Invitation::InvitationType type ) const
{
   PrintFunctionName( __FUNCTION__ );
   PacketInvitation_RejectInvitation decline;
   //todo... validate that this is a valid invitation
   decline.invitationUuid = uuid;
   decline.invitationType = type;

   return SerializePacketOut( &decline );
}

bool     ClientNetworkWrapper::RemoveSentInvitation( const string& uuid, Invitation::InvitationType type ) const
{
   PrintFunctionName( __FUNCTION__ );
   PacketInvitation_CancelInvitation cancel;
   cancel.invitationUuid = uuid;
   cancel.invitationType = type;

   return SerializePacketOut( &cancel );
}

bool     ClientNetworkWrapper::RequestListOfMembersInGroup( const string& groupUuid, Invitation::InvitationType type ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( type == Invitation::InvitationType_ChatRoom  )
   {
      PacketChatListAllMembersInChatChannel  request;
      request.chatChannelUuid = groupUuid;
      //request. = type;
      
      return SerializePacketOut( &request );
   }
   else
   {
      return false;
   }
}

bool     ClientNetworkWrapper::RequestListOfInvitationsForGroup( const string& groupUuid, Invitation::InvitationType type ) const
{
   PrintFunctionName( __FUNCTION__ );
   PacketInvitation_GetListOfInvitationsForGroup   invitationRequest;
   invitationRequest.groupUuid = groupUuid;
   invitationRequest.invitationType = type;

   return SerializePacketOut( &invitationRequest );
}

bool     ClientNetworkWrapper::InviteUserToChatChannel( const string& channelUuid, const string& userUuid, const string& message )
{
   PrintFunctionName( __FUNCTION__ );
   PacketInvitation_InviteUser invite;
   invite.userUuid = userUuid;
   invite.inviteGroup = channelUuid;
   invite.message = message;
   invite.invitationType = Invitation::InvitationType_ChatRoom;

   return SerializePacketOut( &invite );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetListOfInvitationsSentForContacts( list< InvitationInfo >& listOfInvites )
{
   PrintFunctionName( __FUNCTION__ );
   listOfInvites.clear();
   Threading::MutexLock    locker( m_notificationMutex );

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

bool     ClientNetworkWrapper::GetListOfInvitationsReceivedForContacts( list< InvitationInfo >& listOfInvites )
{
   PrintFunctionName( __FUNCTION__ );
   listOfInvites.clear();
   Threading::MutexLock    locker( m_notificationMutex );

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

bool     ClientNetworkWrapper::GetListOfInvitationsSent( list< Invitation >& listOfInvites, Invitation::InvitationType type )
{
   PrintFunctionName( __FUNCTION__ );
   assert( type > 0 && type < Invitation::InvitationType_Num );
   listOfInvites.clear();
   Threading::MutexLock    locker( m_notificationMutex );

   list< Invitation >& tempInvitationList = m_invitations[ type ];
   list< Invitation >::iterator it = tempInvitationList.begin();
   while( it != tempInvitationList.end() )
   {
      if( it->inviterUuid == m_uuid )
         listOfInvites.push_back( *it );
      it++;
   }
   if( listOfInvites.size() > 0 )
      return true;
   
   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetListOfInvitationsReceived( list< Invitation >& listOfInvites, Invitation::InvitationType type )
{
   PrintFunctionName( __FUNCTION__ );
   assert( type > 0 && type < Invitation::InvitationType_Num );
   listOfInvites.clear();
   Threading::MutexLock    locker( m_notificationMutex );

   list< Invitation >& tempInvitationList = m_invitations[ type ];
   list< Invitation >::iterator it = tempInvitationList.begin();
   while( it != tempInvitationList.end() )
   {
      if( it->inviteeUuid == m_uuid )
         listOfInvites.push_back( *it );
      it++;
   }
   if( listOfInvites.size() > 0 )
      return true;
   
   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetListOfGroupInvitations( list< Invitation >& listOfInvites, Invitation::InvitationType type )
{
   PrintFunctionName( __FUNCTION__ );
   assert( type > 0 && type < Invitation::InvitationType_Num );

   listOfInvites.clear();
   Threading::MutexLock    locker( m_notificationMutex );

   list< Invitation >& tempInvitationList = m_invitationsToGroup[ type ];
   list< Invitation >::iterator it = tempInvitationList.begin();
   while( it != tempInvitationList.end() )
   {
      listOfInvites.push_back( *it );
      it++;
   }
   if( listOfInvites.size() > 0 )
      return true;
   
   return false;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RegisterCallbackInterface( ClientSideNetworkCallback* _callbacks ) 
{
   PrintFunctionName( __FUNCTION__ );
   for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
   {
      if( *it == _callbacks )
         return false;
   }
   
   m_callbacks.push_back( _callbacks );  

   _callbacks->network = this;
   return true;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::NeedsProcessingTime() const 
{ 
   // PrintFunctionName( __FUNCTION__ ); // too chatty
   if( IsConnected() == true || m_notifications.size() > 0 ) 
      return true; 

   if( m_requiresGatewayDiscovery == false ) /// we aren't connected but we have the info needed to connect
   {
      for( int i=0; i< ConnectionNames_Num; i++ )
      {
         if( m_serverIpAddress[i].size() )
            return true;
      }
   }
   return false; 
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::SendRawPacket( const char* buffer, int length ) const
{
   PrintFunctionName( __FUNCTION__ );
   PacketGameplayRawData raw;
   memcpy( raw.data, buffer, length );
   raw.size = length;

   return SerializePacketOut( &raw );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::SendSearchForUsers( const string& searchString, int numRequested, int offset ) const // min 2 char
{
   PrintFunctionName( __FUNCTION__ );
   if( searchString.size() < 2 )
      return false;
   if( numRequested < 1 || numRequested > 25 )
      return false;

   if( offset < 0 )
      return false;

   PacketContact_SearchForUser search;
   search.searchString = searchString;
   search.limit = numRequested;
   search.offset = offset;

   return SerializePacketOut( &search );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::InviteUserToBeContact( const string& uuid, const string& userName, const string& message )
{
   PrintFunctionName( __FUNCTION__ );
   if( uuid.size() < 2 && userName.size() < 2 )
      return false;

   PacketContact_InviteContact invitation;
   invitation.userName = userName;
   invitation.uuid = uuid;
   invitation.message = message;

   bool result = SerializePacketOut( &invitation );
//   RequestListOfInvitationsSent();
   return result;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RemoveContact( const string& contactUuid, const string message )
{
   PrintFunctionName( __FUNCTION__ );
   if( IsContact( contactUuid ) == false )
      return false;

   PacketContact_ContactRemove removal;
   removal.contactUuid = contactUuid;
   removal.message = message;

   return SerializePacketOut( &removal );
}

//-----------------------------------------------------------------------------

bool	   ClientNetworkWrapper::SendP2PTextMessage( const string& message, const string& destinationUserUuid )
{
   PrintFunctionName( __FUNCTION__ );
   PacketChatToServer chat;
   chat.message = message;
   chat.userUuid = destinationUserUuid;
   // chat.channelUuid; // not used
   return SerializePacketOut( &chat );
}

//-----------------------------------------------------------------------------

bool	   ClientNetworkWrapper::SendChannelTextMessage( const string& message, const string& chatChannelUuid, U32 gameTurn )
{
   PrintFunctionName( __FUNCTION__ );
   PacketChatToServer chat;
   chat.message = message;
   chat.gameTurn = gameTurn;
   // chat.userUuid = m_uuid; // not used
   chat.channelUuid = chatChannelUuid;
   return SerializePacketOut( &chat );
}
//-----------------------------------------------------------------------------

bool	   ClientNetworkWrapper::CreateNewChatChannel( const string& channelName )
{
   PrintFunctionName( __FUNCTION__ );
   PacketChatCreateChatChannel chat;
   chat.name = channelName;
   return SerializePacketOut( &chat );
}

//-----------------------------------------------------------------------------

bool	   ClientNetworkWrapper::RenameChannel( const string& channelUuid, const string& newName )
{
   PrintFunctionName( __FUNCTION__ );
   ChatChannel channel;
   if( GetChannel( channelUuid, channel ) == false )
      return false;

   PacketChatRenameChannel rename;
   rename.channelUuid = channelUuid;
   rename.newName = newName;

   return SerializePacketOut( &rename );
}

//-----------------------------------------------------------------------------

bool	   ClientNetworkWrapper::AddUserToChannel( const string& userUuid, const string& channelUuid ) // PacketChatAddUserToChatChannel
{
   PrintFunctionName( __FUNCTION__ );
   ChatChannel channel;
   if( GetChannel( channelUuid, channel ) == false )
      return false;

   PacketChatAddUserToChatChannel addUser;
   addUser.chatChannelUuid = channelUuid;
   addUser.userUuid = userUuid;

   return SerializePacketOut( &addUser );
}

//-----------------------------------------------------------------------------
/*
bool	   ClientNetworkWrapper::DeleteChannel( const string& channelUuid ) // PacketChatDeleteChatChannel
{
}*/

//-----------------------------------------------------------------------------

bool	   ClientNetworkWrapper::LeaveChannel( const string& channelUuid )
{
   PrintFunctionName( __FUNCTION__ );
   ChatChannel channel;
   if( GetChannel( channelUuid, channel ) == false )
      return false;

   PacketChatRemoveUserFromChatChannel leave;
   leave.userUuid = m_uuid;
   leave.chatChannelUuid = channelUuid;

   return SerializePacketOut( &leave );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestListOfAssetCategories()
{
   PrintFunctionName( __FUNCTION__ );
   PacketAsset_GetListOfAssetCategories assetCategoryRequest;
   assetCategoryRequest.uuid = m_uuid;
   assetCategoryRequest.loginKey = m_loginKey;
   return SerializePacketOut( &assetCategoryRequest );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestListOfAssets( const string& category, int platformId )
{
   PrintFunctionName( __FUNCTION__ );
   PacketAsset_GetListOfAssets assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.platformId = platformId;
   assetRequest.assetCategory = category;

   return SerializePacketOut( &assetRequest );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestAssetByHash( const string& assetHash )
{
   PrintFunctionName( __FUNCTION__ );
   AssetInfoExtended asset;
   if( GetAsset( assetHash, asset ) == false )
      return false;

   PacketAsset_RequestAsset assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.assetHash = assetHash;

   return SerializePacketOut( &assetRequest );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestAssetByName( const string& assetName )
{
   PrintFunctionName( __FUNCTION__ );
   AssetInfoExtended asset;
   const string assetHash = GenerateHash( assetName );
   if( GetAsset( assetHash, asset ) == false )
      return false;

   PacketAsset_RequestAsset assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.assetHash = assetHash ;

   return SerializePacketOut( &assetRequest );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestAvatarById( U32 id )
{
   PrintFunctionName( __FUNCTION__ );
   AssetInfoExtended asset;
   const string assetHash = GenerateHash( boost::lexical_cast< string >( id ) );
   if( GetAsset( assetHash, asset ) == false )
      return false;

   if( asset.category != "avatar" )
      return false;

   PacketAsset_RequestAsset assetRequest;
   assetRequest.uuid = m_uuid;
   assetRequest.loginKey = m_loginKey;
   assetRequest.assetHash = assetHash ;

   return SerializePacketOut( &assetRequest );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RegisterDevice( const string& playdekUuid, const string& deviceName, PlatformType platformId, const string& vendorProvidedDeviceId )
{
   PrintFunctionName( __FUNCTION__ );
   PacketNotification_RegisterDevice registration;
   registration.deviceName = deviceName;
   registration.deviceId = vendorProvidedDeviceId;
   registration.platformId = (int) platformId;
   registration.assignedUuid = playdekUuid;

   return SerializePacketOut( &registration );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::RequestListOfDevicesForThisGame( int platformId )
{
   PrintFunctionName( __FUNCTION__ );
   PacketNotification_RequestListOfDevices request;
   request.platformId = platformId;

   return SerializePacketOut( &request );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::ChangeDevice(  const string& deviceUuid, const string& deviceNewName, const string& audioFileToPlay, bool isEnabled, int iconId, int repeatFrequencyInHours )
{
   PrintFunctionName( __FUNCTION__ );
   PacketNotification_UpdateDevice update;
   update.deviceUuid = deviceUuid;//m_thisDeviceUuid;
   update.deviceName = deviceNewName;
   update.isEnabled = isEnabled;
   update.audioFile = audioFileToPlay;
   update.repeatFrequencyInHours = repeatFrequencyInHours;
   update.iconId = iconId;
   update.gameType = m_gameProductId;

   return SerializePacketOut( &update );
}

//-----------------------------------------------------------------------------

bool    ClientNetworkWrapper::GetDevice( int index, RegisteredDevice& device ) const
{   
   PrintFunctionName( __FUNCTION__ );
   if( index < 0 || index >= static_cast< int >( m_devices.size() ) )
      return false;
   device = m_devices[ index ];
   return true;
}

//-----------------------------------------------------------------------------

bool    ClientNetworkWrapper::RemoveDevice( const string& deviceUuid ) const
{
   PrintFunctionName( __FUNCTION__ );
   if( deviceUuid.size() < 1 )
      return false;

   PacketNotification_RemoveDevice  remove;
   remove.deviceUuid = deviceUuid;

   return SerializePacketOut( &remove );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::SendPurchases( const vector< RegisteredProduct >& purchases, int platformId )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

   PacketListOfUserAggregatePurchases packet;
   packet.platformId = platformId;

   vector< RegisteredProduct >::const_iterator it = purchases.begin();
   while( it != purchases.end() )
   {
      const RegisteredProduct& rp = *it++;
      
      PurchaseEntryExtended pe;
      pe.productUuid = rp.id;
      pe.name = rp.title;
      pe.quantity = rp.quantity;
      //pe.parentUuid = rp.parentUuid;
      //pe.productUuid = rp.;
      //pe.number_price = rp.number_price;
      //pe.price = rp.price;
      //pe.date = GetDateInUTC();

      packet.purchases.push_back( pe );
   }

   return SerializePacketOut( &packet );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::GiveProduct( const string& userUuid, const string& productUuid, int quantity, const string& notes, int platformId )
{
   PrintFunctionName( __FUNCTION__ );
   PacketAddPurchaseEntry purchaseEntry;
   purchaseEntry.userName = userUuid;
   purchaseEntry.userEmail = userUuid;
   purchaseEntry.userUuid = userUuid;
   purchaseEntry.productUuid = productUuid;
   purchaseEntry.quantity = quantity;

   purchaseEntry.adminNotes  = notes;
   purchaseEntry.platformId = platformId;

   return SerializePacketOut( &purchaseEntry );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::SendCheat( const string& cheatText )
{
   PrintFunctionName( __FUNCTION__ );
   PacketCheat cheat;
   cheat.cheat = cheatText;
   cheat.whichServer = ServerType_Login;// needs improvement
   return SerializePacketOut( &cheat );
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::SerializePacketOut( BasePacket* packet ) const 
{
   packet->gameInstanceId = m_selectedGame;
   packet->gameProductId = m_gameProductId;

   //m_beginTime = GetCurrentMilliseconds();

   U8 type = packet->packetType;
   if( type == PacketType_Asset ) 
   {
      if( m_connectToAssetServer == true && 
         m_fruitadens[ ConnectionNames_Asset ]->IsConnected() == true )
         return m_fruitadens[ ConnectionNames_Asset ]->SerializePacketOut( packet );
      else 
         return false;
   }
   else
   {
      // for initial callbacks, we may not be connected
      //assert( m_fruitadens[ ConnectionNames_Main ]->IsConnected() );
      // TODO.. branch on asset requests.. otherwise, main
      return m_fruitadens[ ConnectionNames_Main ]->SerializePacketOut( packet );
   }
}


//-----------------------------------------------------------------------------

void      ClientNetworkWrapper::Update()
{
   if( IsConnected() == false )
   {
      if( m_requiresGatewayDiscovery == true )
         return;
   }

   if( m_readyToReconnectToGateway == true )
   {
      ReconnectAfterTalkingToLoadBalancer();
      return;
   }

   ExpireThreadPerformanceBoost();

   //return Fruitadens::MainLoop_InputProcessing();
}

void     ClientNetworkWrapper::BoostThreadPerformance( ConnectionNames whichConnection )
{
   PrintFunctionName( __FUNCTION__ );
   assert( whichConnection < ConnectionNames_Num );
   m_fruitadens[ whichConnection ]->SetSleepTime( m_boostedSleepTime );
   m_fruitadens[ whichConnection ]->SetPriority( Threading::CAbstractThread:: ePriorityHigh );
   m_isThreadPerformanceBoosted[ whichConnection ] = true;
   time( &m_timeWhenThreadPerformanceBoosted[ whichConnection ] );
}

void     ClientNetworkWrapper::RestoreNormalThreadPerformance( ConnectionNames whichConnection )
{
   PrintFunctionName( __FUNCTION__ );
   assert( whichConnection < ConnectionNames_Num );
   m_fruitadens[ whichConnection ]->SetSleepTime( m_normalSleepTime );
   m_isThreadPerformanceBoosted[ whichConnection ] = false;
   m_fruitadens[ whichConnection ]->SetPriority( Threading::CAbstractThread:: ePriorityNormal );
}

void     ClientNetworkWrapper::ExpireThreadPerformanceBoost( ConnectionNames whichConnection )
{
   //PrintFunctionName( __FUNCTION__ );// chatty
   assert( whichConnection < ConnectionNames_Num );
   if( m_isThreadPerformanceBoosted[ whichConnection ] == true )
   {
      time_t currentTime;// expire after 5 seconds.
      time( &currentTime );
      if( difftime( currentTime, m_timeWhenThreadPerformanceBoosted[ whichConnection ] ) > 5.0 )
      {
         RestoreNormalThreadPerformance( whichConnection );
      }
   }
}

//-----------------------------------------------------------------------------

void     ClientNetworkWrapper::LoadBalancedNewAddress( const PacketRerouteRequestResponse* response )
{
   PrintFunctionName( __FUNCTION__ );
   if( m_requiresGatewayDiscovery == false )
      return;

   cout << "BasePacket_RerouteRequestResponse:" << endl;
   cout << "Num server locations: " << response->locations.size() << endl;
   

   bool     isReconnectNecessary = false;
   int num = response->locations.size();
   for( int i=0; i< num; i++ )
   {
      const PacketRerouteRequestResponse::Address& address = response->locations[i];
      if( address.whichLocationId == PacketRerouteRequestResponse::LocationId_Gateway )
      {
         if( m_serverIpAddress[ ConnectionNames_Main ] != address.address ||
            m_serverConnectionPort[ ConnectionNames_Main ] != address.port )
         {
            isReconnectNecessary = true;
         }
         m_serverIpAddress[ ConnectionNames_Main ] = address.address;
         m_serverConnectionPort[ ConnectionNames_Main ] = address.port;
         cout << "Main server address: " << address.address << endl;
         cout << "Main server port: " << address.port << endl;
      }
      else if( address.whichLocationId == PacketRerouteRequestResponse::LocationId_Asset )
      {
         m_serverIpAddress[ ConnectionNames_Asset ] = address.address;
         m_serverConnectionPort[ ConnectionNames_Asset ] = address.port;
         cout << "Asset server address: " << address.address << endl;
         cout << "Asset server port: " << address.port << endl;
      }
      else
      {
         assert(0);// fix this. You've probably added a new category and need to add the structure here for this.
      }
   }

   //if( isReconnectNecessary )
   {
      Disconnect(); // << slight danger here... disconnecting when we are servicing a previous request
   }

   // when setting up the reroute, we must wait to clear this flag. Other threads will attempt to 
   // reconnect, interfere in various ways, etc. This blocks a lot of bad behavior.
   m_requiresGatewayDiscovery = false;// clear this flag
   m_readyToReconnectToGateway = true;
}

//-----------------------------------------------------------------------------

bool     ClientNetworkWrapper::HandlePacketReceived( BasePacket* packetIn )
{
   //PrintFunctionName( __FUNCTION__ );// duplicitous
   PacketCleaner cleaner( packetIn );

   cout << "Packet type " << (U32) (packetIn->packetType) << ":" << (U32) (packetIn->packetSubType) << endl;
   if( packetIn->packetType == 9 )
   {
      cout << "Type 9 met" << endl;
   }

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

               Notification( ClientSideNetworkCallback::NotificationType_AreWeUsingCorrectNetworkVersion, (int)packet->versionNumber );
            }
            break;
         case BasePacket::BasePacket_RerouteRequestResponse:
            {
               PacketRerouteRequestResponse* unwrappedPacket = static_cast< PacketRerouteRequestResponse * > ( packetIn );

               LoadBalancedNewAddress( unwrappedPacket );
            }
            break;
         case BasePacket::BasePacket_QOS:
            {
               Packet_QOS_ReportToClient* qosReport = static_cast< Packet_QOS_ReportToClient * > ( packetIn );

               //LoadBalancedNewAddress( unwrappedPacket );
             /*  cout << "Server connection state has changed" << endl;
               cout << " error text: " << qosReport->errorText << endl;
               cout << " error errorState: " << qosReport->errorState << endl;
               cout << " error param1: " << qosReport->param1 << endl;
               cout << " error param2: " << qosReport->param2 << endl;*/

               Notification( ClientSideNetworkCallback::NotificationType_QOS_ServiceChange, qosReport );
               cleaner.Clear();// do not cleanup this packe
            }
            break;
         }
      }
      break;
      case PacketType_Contact:
      {
         switch( packetIn->packetSubType )
         {
         case PacketContact::ContactType_GetListOfContactsResponse:
            {
               PacketContact_GetListOfContactsResponse* packet = static_cast< PacketContact_GetListOfContactsResponse* >( packetIn );
               HandleListOfContacts( packet );
               Notification( ClientSideNetworkCallback::NotificationType_FriendsUpdate );
            }
            break;
         case PacketContact::ContactType_UserOnlineStatusChange:
            {
               PacketContact_FriendOnlineStatusChange* packet = static_cast< PacketContact_FriendOnlineStatusChange* >( packetIn );
               HandleUserOnlineStatusChange( packet );               
               Notification( ClientSideNetworkCallback::NotificationType_FriendOnlineStatusChanged, packet->uuid );
            }
            break;
         case PacketContact::ContactType_GetListOfInvitationsResponse:
            {
               PacketContact_GetListOfInvitationsResponse* packet = static_cast< PacketContact_GetListOfInvitationsResponse* >( packetIn );
               HandleListOfReceivedInvitations( packet );               
               Notification( ClientSideNetworkCallback::NotificationType_InvitationsReceivedUpdate );
               // RequestListOfInvitationsReceived();
            }
            break;
         case PacketContact::ContactType_GetListOfInvitationsSentResponse:
            {
               cout << "contacts received" << endl;
               PacketContact_GetListOfInvitationsSentResponse* packet = static_cast< PacketContact_GetListOfInvitationsSentResponse* >( packetIn );
               cout << "Num invites sent: " << packet->invitations.size() << endl;

               HandleListOfSentInvitations( packet );
               Notification( ClientSideNetworkCallback::NotificationType_InvitationsSentUpdate );
               //RequestListOfInvitationsSent();
            }
            break;
         case PacketContact::ContactType_InviteReceived:
            {
               
               PacketContact_InviteReceivedNotification* packet = static_cast< PacketContact_InviteReceivedNotification* >( packetIn );
               HandleInvitationReceived( packet );
               
               Notification( ClientSideNetworkCallback::NotificationType_InvitationReceived, packet->info.uuid );
            }
            break;
         case PacketContact::ContactType_InvitationAccepted:
            {
               PacketContact_InvitationAccepted* packet = static_cast< PacketContact_InvitationAccepted* >( packetIn );
               HandleInvitationAccepted( packet );
               SerializedKeyValueVector< string > strings;
               strings.insert( "from", packet->fromUsername );
               strings.insert( "to", packet->toUsername );
               string accepted = "0";
               if( packet->wasAccepted )
                  accepted = "1";
               strings.insert( "accepted", accepted );

               Notification( ClientSideNetworkCallback::NotificationType_InvitationAccepted, strings );
            }
            break;
         case PacketContact::ContactType_SearchResults:
            {
               PacketContact_SearchForUserResult* packet = static_cast< PacketContact_SearchForUserResult* >( packetIn );
               HandleSearchForUserResults( packet );
               Notification( ClientSideNetworkCallback::NotificationType_SearchResults );
            }
            break;
         }
         
      }
      break;
      case PacketType_ErrorReport:
      {
         PacketErrorReport* errorReport = static_cast<PacketErrorReport*>( packetIn );
         m_isCreatingAccount = false;
         SerializedKeyValueVector< string > strings;
         strings.insert( "code", boost::lexical_cast<string>( errorReport->errorCode ) );
         strings.insert( "status", boost::lexical_cast<string>( errorReport->statusInfo ) );
         strings.insert( "text", errorReport->text );
         Notification( ClientSideNetworkCallback::NotificationType_OnError, strings );
         
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
                     m_uuid = login->uuid.c_str();
                     //m_email = login->email;
                     m_connectionId = login->connectionId;
                     m_lastLoggedOutTime = login->lastLogoutTime;
                     m_loginKey = login->loginKey;
                     m_isLoggedIn = true;
                  }
                  else
                  {
                     Disconnect();// server forces a logout.
                     m_isLoggedIn = false;
                  }
                  Notification( ClientSideNetworkCallback::NotificationType_UserLogin, m_isLoggedIn );
                  
                  m_isLoggingIn = false;
               }
               break;
            case PacketLogin::LoginType_PacketLogoutToClient:
               {
                  PacketLogout* logout = static_cast<PacketLogout*>( packetIn );
                  Notification( ClientSideNetworkCallback::NotificationType_UserLogout );
                 
                  Disconnect();
                  m_isLoggingIn = false;
                  m_isLoggedIn = false;
               }
               break;
            case PacketLogin::LoginType_RequestListOfPurchases:
               {
                  Notification( ClientSideNetworkCallback::NotificationType_ServerRequestsListOfUserPurchases );
               }
               break;
            case PacketLogin::LoginType_ListOfAggregatePurchases:
               {
                  PacketListOfUserAggregatePurchases* packet = static_cast<PacketListOfUserAggregatePurchases*>( packetIn );
                  HandleListOfAggregatePurchases( packet );
                  Notification( ClientSideNetworkCallback::NotificationType_ListOfAggregateUserPurchases, packet->userUuid );
               }
               break;
            case PacketLogin::LoginType_RequestUserProfileResponse:
               {
                  PacketRequestUserProfileResponse* profile = static_cast<PacketRequestUserProfileResponse*>( packetIn );
                  bool notifyThatSelfUpdated = false;

                  if( profile->userUuid == m_uuid )
                  {
                     notifyThatSelfUpdated = true;
                     m_userName = profile->userName;
                     m_email = profile->email;

                     m_languageId = profile->languageId;
                     m_avatarId = profile->iconId;
                     m_motto = profile->motto;
                     m_showWinLossRecord = profile->showWinLossRecord;
                     m_marketingOptOut = profile->marketingOptOut;
                     m_showGenderProfile = profile->showGenderProfile;
                     m_displayOnlineStatusToOtherUsers = profile->displayOnlineStatusToOtherUsers;
                     m_blockContactInvitations = profile->blockContactInvitations;
                     m_blockGroupInvitations = profile->blockGroupInvitations;
                  }

                  Notification( ClientSideNetworkCallback::NotificationType_UserProfileResponse, notifyThatSelfUpdated );
               }
               break;
            case PacketLogin::LoginType_RequestListOfProductsResponse:
               {
                  PacketRequestListOfProductsResponse* packet = static_cast<PacketRequestListOfProductsResponse*>( packetIn );
                  HandleListOfProducts( packet );
               }
               break;
            case PacketLogin::LoginType_RequestOtherUserProfileResponse:
               {
                  PacketRequestOtherUserProfileResponse* profile = static_cast<PacketRequestOtherUserProfileResponse*>( packetIn );
                  Notification( ClientSideNetworkCallback::NotificationType_OtherUsersProfile, profile->basicProfile );
               }
               break;
            case PacketLogin::LoginType_UpdateSelfProfileResponse:
               {
                  PacketUpdateSelfProfileResponse* profile = static_cast<PacketUpdateSelfProfileResponse*>( packetIn );
                  if( profile->success == true )
                  {
                     m_avatarId = profile->avatarIconId;
                  }
                  Notification( ClientSideNetworkCallback::NotificationType_SelfProfileUpdate, profile->success );
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
               HandleChatChannelUpdate( packetIn );
            }
            break;
            // demographics, winloss, etc.
         }
      }
      break;
      case PacketType_Purchase:
         {
            //assert( 0 ); // disabled
            switch( packetIn->packetSubType )
            {
               
            case PacketPurchase::PurchaseType_BuyResponse:
               {
                  PacketPurchase_BuyResponse* purchase = static_cast<PacketPurchase_BuyResponse*>( packetIn );
                  Notification( ClientSideNetworkCallback::NotificationType_PurchaseSuccess, purchase );
                  cleaner.Clear();// do not cleanup this packet
               }
               break;
            case PacketPurchase::PurchaseType_RequestListOfSalesResponse:
               {
                  PacketPurchase_RequestListOfSalesResponse* purchase = static_cast<PacketPurchase_RequestListOfSalesResponse*>( packetIn );
                  Notification( ClientSideNetworkCallback::NotificationType_ProductsForSale, purchase );
                  cleaner.Clear();// do not cleanup this packet
               }
               break;
            case PacketPurchase::PurchaseType_ValidatePurchaseReceiptResponse:
               {
                  PacketPurchase_ValidatePurchaseReceiptResponse* purchase = static_cast<PacketPurchase_ValidatePurchaseReceiptResponse*>( packetIn );
                  Notification( ClientSideNetworkCallback::NotificationType_PurchaseReceiptResponse, purchase );
                  cleaner.Clear();// do not cleanup this packet
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
               cout << "     message: " << chat->message.c_str() << endl;

               SerializedKeyValueVector< string > strings;
               strings.insert( "message", chat->message );
               strings.insert( "channelUuid", chat->channelUuid );
               strings.insert( "userUuid", chat->userUuid );
               strings.insert( "timeStamp", chat->timeStamp );
               strings.insert( "tempId", boost::lexical_cast< string >( chat->userTempId ) );
               Notification( ClientSideNetworkCallback::NotificationType_ChatReceived, strings );
            }
            break;
          case PacketChatToServer::ChatType_UserAddedToChatChannelFromGameServer:
             {
               PacketChatUserAddedToChatChannelFromGameServer* chat = static_cast<PacketChatUserAddedToChatChannelFromGameServer*>( packetIn );
               HandleBeingAddedByServerToChatChannel( chat );
               Notification( ClientSideNetworkCallback::NotificationType_ChatChannelUpdate, chat->channelUuid );
               Notification( ClientSideNetworkCallback::NotificationType_ChatListUpdate );
             }
             break;          
          case PacketChatToServer::ChatType_RequestHistoryResult:
            {
               PacketChatHistoryResult* history = static_cast<PacketChatHistoryResult*>( packetIn );
               if( history->chatChannelUuid.size() )
               {
                  Notification( ClientSideNetworkCallback::NotificationType_ChatChannelHistory, history );
               }
               else
               {
                  Notification( ClientSideNetworkCallback::NotificationType_ChatP2PHistory, history );
               }
               cleaner.Clear();// do not cleanup this packet
            }
            break;
         case PacketChatToServer:: ChatType_RequestHistorySinceLastLoginResponse:
            {
               PacketChatMissedHistoryResult* history = static_cast< PacketChatMissedHistoryResult* >( packetIn );
               Notification( ClientSideNetworkCallback::NotificationType_ChatHistoryMissedSinceLastLoginComposite, history );
               cleaner.Clear();// do not cleanup this packet
            }
            break;
         case PacketChatToServer::ChatType_UserChatStatusChange:
            {
               PacketChatUserStatusChangeBase* packet = static_cast< PacketChatUserStatusChangeBase* >( packetIn );
               HandleUserChatStatusChange( packet );
              
               Notification( ClientSideNetworkCallback::NotificationType_FriendOnlineStatusChanged, packet->uuid );
             }
            break;
         case PacketChatToServer::ChatType_CreateChatChannelResponse:
            {
               PacketChatCreateChatChannelResponse* chat = static_cast< PacketChatCreateChatChannelResponse* >( packetIn );

               SerializedKeyValueVector< string > strings;
               strings.insert( "name", chat->name );
               strings.insert( "uuid", chat->uuid );
               string success = "0";
               if( chat->successfullyCreated )
                  success = "1";
               strings.insert( "success", success );

               Notification( ClientSideNetworkCallback::NotificationType_NewChatChannelAdded, strings );
            }
            break;
         case PacketChatToServer::ChatType_DeleteChatChannelResponse:
            {
               PacketChatDeleteChatChannelResponse* chat = static_cast< PacketChatDeleteChatChannelResponse* >( packetIn );
               SerializedKeyValueVector< string > strings;
               strings.insert( "uuid", chat->uuid );
               string success = "0";
               if( chat->successfullyDeleted )
                  success = "1";
               strings.insert( "success", success );

               Notification( ClientSideNetworkCallback::NotificationType_ChatChannelDeleted, strings );
            }
            break;
         case PacketChatToServer::ChatType_AddUserToChatChannelResponse:
            {
               PacketChatAddUserToChatChannelResponse* packet = static_cast< PacketChatAddUserToChatChannelResponse* >( packetIn );
               HandleAddUserToChatChannel( packet );

               SerializedKeyValueVector< string > strings;
               strings.insert( "channelName", packet->channelName );
               strings.insert( "channelUuid", packet->channelUuid );
               strings.insert( "userName", packet->userName );
               strings.insert( "userUuid", packet->userUuid );
               Notification( ClientSideNetworkCallback::NotificationType_ChatChannel_UserAdded, strings );
             }
            break;
         case PacketChatToServer::ChatType_RemoveUserFromChatChannelResponse:
            {
               PacketChatRemoveUserFromChatChannelResponse* packet = static_cast< PacketChatRemoveUserFromChatChannelResponse* >( packetIn );
               HandleRemoveUserFromChatChannel( packet );

               SerializedKeyValueVector< string > strings;
               strings.insert( "channelUuid", packet->chatChannelUuid );
               strings.insert( "userUuid", packet->userUuid );
               string success = "0";
               if( packet->success )
                  success = "1";
               strings.insert( "success", success );

               Notification( ClientSideNetworkCallback::NotificationType_ChatChannel_UserRemoved, strings );

            }
            break;
         case PacketChatToServer::ChatType_ListAllMembersInChatChannelResponse:
            {
               PacketChatListAllMembersInChatChannelResponse* packet = static_cast< PacketChatListAllMembersInChatChannelResponse* >( packetIn );

               Notification( ClientSideNetworkCallback::NotificationType_ChatChannelMembers, packet );

               cleaner.Clear();
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
               PacketGameIdentification* gameId = static_cast<PacketGameIdentification*>( packetIn );

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
               PacketGameplayRawData* data = static_cast<PacketGameplayRawData*>( packetIn );

               HandleData( data );
               cleaner.Clear();
            }
            //return false;// this is important... we will delete the packets.
            break;
         
         case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
            {
              /* PacketRequestUserWinLossResponse* response = 
                     static_cast<PacketRequestUserWinLossResponse*>( packetIn );
               for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->UserWinLoss( response->userUuid, response->winLoss );
               }*/
               assert( 0 );// not finished, wrong user data
            }
            break;
         }
      }
      break;
      case PacketType_Invitation:
      {
         switch( packetIn->packetSubType )
         {
         case PacketInvitation::InvitationType_GetListOfInvitationsResponse:
            {
               PacketInvitation_GetListOfInvitationsResponse* response = static_cast<PacketInvitation_GetListOfInvitationsResponse*>( packetIn );
               const SerializedKeyValueVector< Invitation >& invitations = response->invitationList;

               int whichGroup = response->invitationType;
               assert( whichGroup >0 && whichGroup < Invitation::InvitationType_Num );
               U16 firstIndex = invitations.GetFirstIndex();
               U16 totalCount = invitations.GetTotalCount();
               U16 numChannelsInCurrentList = invitations.size();
               if( invitations.GetFirstIndex() == 0 )
               {
                  m_invitations[ whichGroup ].clear();
               }
               
               SerializedKeyValueVector< Invitation >::const_KVIterator it = invitations.begin();
               while (it != invitations.end() )
               {
                  const Invitation& invite = it->value;
                  cout << "Invitation: " << invite.invitationUuid << endl;
                  cout << "  inviter : " << invite.inviterName << endl;
                  cout << "  invitee : " << invite.inviteeName << endl;
                  it++;
                  m_invitations[ whichGroup ].push_back( invite );
               }
               if( firstIndex + numChannelsInCurrentList >= totalCount) // We are at the end of the list
               {
                  Notification( ClientSideNetworkCallback::NotificationType_GenericInvitationsUpdated, whichGroup );
               }
            }
            break;
         case PacketInvitation::InvitationType_GetListOfInvitationsForGroupResponse:
            {
               PacketInvitation_GetListOfInvitationsForGroupResponse* response = static_cast<PacketInvitation_GetListOfInvitationsForGroupResponse*>( packetIn );
               const SerializedKeyValueVector< Invitation >& invitations = response->invitationList;

               int whichGroup = response->invitationType;
               assert( whichGroup >0 && whichGroup < Invitation::InvitationType_Num );

               U16 firstIndex = invitations.GetFirstIndex();
               U16 totalCount = invitations.GetTotalCount();
               U16 numChannelsInCurrentList = invitations.size();
               if( invitations.GetFirstIndex() == 0 )
               {
                  m_invitationsToGroup[ whichGroup ].clear();
               }
               
               SerializedKeyValueVector< Invitation >::const_KVIterator it = invitations.begin();
               while (it != invitations.end() )
               {
                  const Invitation& invite = it->value;
                  cout << "Invitation: " << invite.invitationUuid << endl;
                  cout << "  inviter : " << invite.inviterName << endl;
                  cout << "  invitee : " << invite.inviteeName << endl;
                  it++;
                  m_invitationsToGroup[ whichGroup ].push_back( invite );
               }
               if( firstIndex + numChannelsInCurrentList >= totalCount) // We are at the end of the list
               {
                  Notification( ClientSideNetworkCallback::NotificationType_GroupInvitationsUpdated, whichGroup );
               }

            }
            break;
         case PacketInvitation::InvitationType_RejectInvitationResponse:
            {
               PacketInvitation_RejectInvitationResponse* response = static_cast<PacketInvitation_RejectInvitationResponse*>( packetIn );
               cout << "Invitation rejection complete" << endl;

               Notification( ClientSideNetworkCallback::NotificationType_GenericInvitationRejected );
            }
            break;
         case PacketGameToServer::GamePacketType_RawGameData:
            {
               PacketGameplayRawData* data = static_cast<PacketGameplayRawData*>( packetIn );

               HandleData( data );
               cleaner.Clear();
            }
            //return false;// this is important... we will delete the packets.
            break;
         
         case PacketGameToServer::GamePacketType_RequestUserWinLossResponse:
            {
              /* PacketRequestUserWinLossResponse* response = 
                     static_cast<PacketRequestUserWinLossResponse*>( packetIn );
               for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
               {
                  (*it)->UserWinLoss( response->userUuid, response->winLoss );
               }*/
               assert( 0 );// not finished, wrong user data
            }
            break;
         }
      }
      break;
      case PacketType_UserStats:
      {
         switch( packetIn->packetSubType )
         {
         case PacketUserStats::UserStatsType_RequestListOfUserStatsResponse:
            {
               PacketUserStats_RequestListOfUserStatsResponse* response = static_cast<PacketUserStats_RequestListOfUserStatsResponse*>( packetIn );
               Notification( ClientSideNetworkCallback::NotificationType_UserStats, response );
               cleaner.Clear();
            }
            break;
         case PacketUserStats::UserStatsType_RequestUserProfileStatsResponse:
            {
               PacketUserStats_RequestUserProfileStatsResponse* response = static_cast<PacketUserStats_RequestUserProfileStatsResponse*>( packetIn );
               Notification( ClientSideNetworkCallback::NotificationType_UserProfileStats, response );
               printf( "PacketUserStats_RequestUserProfileStatsResponse\n" );
               printf( "userId = %u\n", response->profileUserId );
               printf( "gameType = %u\n", response->gameType );
               
               printf( "wins_2p = %s\n", response->userProfileStats.find("wins_2p").data() );
               cleaner.Clear();
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
               PacketAsset_GetListOfAssetCategoriesResponse* packet = static_cast<PacketAsset_GetListOfAssetCategoriesResponse*>( packetIn );
               HandleListOfAssetCategoriesUpdate( packet );
               Notification( ClientSideNetworkCallback::NotificationType_AssetCategoriesLoaded );
            }
            break;

            case PacketAsset::AssetType_GetListOfAssetsResponse:
            {
               PacketAsset_GetListOfAssetsResponse* packet = 
                  static_cast<PacketAsset_GetListOfAssetsResponse*>( packetIn );

               HandleListOfAssets( packet );
               const string& category = packet->assetCategory;
               Notification( ClientSideNetworkCallback::NotificationType_AssetManifestAvailable, category );
            }
            break;
          }
      }
      break;
      case PacketType_Notification:
      {
         switch( packetIn->packetSubType )
         {
            case PacketNotification::NotificationType_RegisterDeviceResponse:
            {
               PacketNotification_RegisterDeviceResponse* response = 
                  static_cast<PacketNotification_RegisterDeviceResponse*>( packetIn );
               m_thisDeviceUuid = response->deviceUuid.c_str();
               
               Notification( ClientSideNetworkCallback::NotificationType_DeviceIDUpdated );
            }
            break;
            case PacketNotification::NotificationType_RequestListOfDevicesResponse:
            {
               PacketNotification_RequestListOfDevicesResponse* packet = 
                  static_cast<PacketNotification_RequestListOfDevicesResponse*>( packetIn );

               HandleListOfDevices( packet );

               Notification( ClientSideNetworkCallback::NotificationType_ListOfDevicesUpdated );
            }
            break;
            case PacketNotification::NotificationType_RemoveDeviceResponse:
            {
               PacketNotification_RemoveDeviceResponse* response =  
                  static_cast<PacketNotification_RemoveDeviceResponse*>( packetIn );

               SerializedKeyValueVector< string > strings;
               strings.insert( "channelUuid", response->deviceUuid );
               strings.insert( "success", boost::lexical_cast< string >( response->success ? 1:0 ) );
               if( response->success == true )
               {
                  Threading::MutexLock    locker( m_notificationMutex );
                  for( int i = 0; i<m_devices.size(); i++ )
                  {
                     if( m_devices[ i ].uuid == response->deviceUuid )
                     {
                        m_devices.remove( i );
                        break;
                     }
                  }
               }

               Notification( ClientSideNetworkCallback::NotificationType_DeviceRemoved, strings );
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

               Threading::MutexLock    locker( m_notificationMutex );
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
                  Notification( ClientSideNetworkCallback::NotificationType_TournamentListAvalable );
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
              
               SerializedKeyValueVector< string > strings;
               strings.insert( "tournamentUuid", response->tournamentUuid );
               strings.insert( "success", boost::lexical_cast< string >( response->result ) );

               Notification( ClientSideNetworkCallback::NotificationType_TournamentPurchaseResult, strings );

               if( response->result == PacketErrorReport::ErrorType_Purchase_Success )
               {

               }
            }
            break;
         }
      }
      break;
   }

   return true;
}

void     ClientNetworkWrapper::HandleListOfContacts( const PacketContact_GetListOfContactsResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleListOfContacts : num records: " << packet->friends.size() << endl;

   m_contacts.clear();

   Threading::MutexLock    locker( m_notificationMutex );

   SerializedKeyValueVector< FriendInfo >::const_KVIterator it = packet->friends.begin();
   while( it != packet->friends.end() )
   {
      BasicUser bu;
      const FriendInfo& fi = it->value;
      bu.isOnline = fi.isOnline;
      bu.userName = fi.userName;
      bu.UUID = it->key;
      bu.avatarId = fi.avatarId;
      bu.notesAboutThisUser = fi.notesAboutThisUser.c_str();
      bu.motto = fi.motto.c_str();
      bu.markedAsFavorite = fi.markedAsFavorite;

      m_contacts.insert( it->key, bu );
      it++;
   }
}

void     ClientNetworkWrapper::HandleUserOnlineStatusChange( const PacketContact_FriendOnlineStatusChange* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleUserOnlineStatusChange: " << packet->friendInfo.userName << " online status = " << boolalpha << packet->friendInfo.isOnline << noboolalpha << endl;
   Threading::MutexLock    locker( m_notificationMutex );

   bool updated = false;
   UserNameKeyValue::KVIterator  itFriends = m_contacts.begin();
   while( itFriends != m_contacts.end() )
   {
      if( itFriends->key == packet->uuid )
      {
         itFriends->value.userName = packet->friendInfo.userName;
         itFriends->value.isOnline = packet->friendInfo.isOnline;
         itFriends->value.avatarId = packet->friendInfo.avatarId;
         updated = true;
      }
      itFriends++;
   }
}

void     ClientNetworkWrapper::HandleListOfReceivedInvitations( const PacketContact_GetListOfInvitationsResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleListOfReceivedInvitations: Num invites received: " << packet->invitations.size() << endl;
   Threading::MutexLock    locker( m_notificationMutex );

   m_invitationsReceived.clear();
   const SerializedKeyValueVector< InvitationInfo >& kvVector = packet->invitations;
   m_invitationsReceived = kvVector;// expensive copy

   SerializedKeyValueVector< InvitationInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      const InvitationInfo& invite = it->value;
      cout << " **invite from: " << invite.inviterName << endl;
      cout << "   invite to: " << invite.inviteeName << endl;
      cout << "   message: " << invite.message << endl;
      it++;
   }
}

void     ClientNetworkWrapper::HandleListOfSentInvitations( const PacketContact_GetListOfInvitationsSentResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleListOfSentInvitations: Num invites sent: " << packet->invitations.size() << endl;
   Threading::MutexLock    locker( m_notificationMutex );

   m_invitationsSent.clear();
   const SerializedKeyValueVector< InvitationInfo >& kvVector = packet->invitations;
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
}

void     ClientNetworkWrapper::HandleInvitationReceived( const PacketContact_InviteReceivedNotification* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleInvitationReceived: new invite received" << endl;

   cout << "   From " << packet->info.inviterName << endl;
   cout << "   message: " << packet->info.message << endl;

   Threading::MutexLock    locker( m_notificationMutex );
   m_invitationsReceived.insert( packet->info.uuid, packet->info );
}

void     ClientNetworkWrapper::HandleInvitationAccepted( const PacketContact_InvitationAccepted* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleInvitationAccepted: invite accepted" << endl;
   cout << "From " << packet->fromUsername << endl;
   cout << "To " << packet->toUsername << endl;
   cout << "Was accepted " << packet->wasAccepted << endl;

   Threading::MutexLock    locker( m_notificationMutex );

   if( packet->fromUsername == m_userName )
   {
      RemoveInvitationFromSent( packet->invitationUuid.c_str() );
   }
   else
   {
      RemoveInvitationFromReceived( packet->invitationUuid.c_str() );
   }
}

void     ClientNetworkWrapper::HandleSearchForUserResults( const PacketContact_SearchForUserResult* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleSearchForUserResults: search for user results received" << endl;
   cout << "Num found: " << packet->found.size() << endl;

   Threading::MutexLock    locker( m_notificationMutex );

   m_lastUserSearch.clear();
   const SerializedKeyValueVector< FriendInfo >& kvVector = packet->found;
   SerializedKeyValueVector< FriendInfo >::const_KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      const FriendInfo& friendly = it->value; // friend is a keyword

      BasicUser bu;
      bu.userName = friendly.userName;
      bu.isOnline = friendly.isOnline;
      bu.UUID = it->key;
      m_lastUserSearch.insert( it->key, bu );
      cout << " **found user: " << friendly.userName << endl;
      cout << "   uuid: " << bu.UUID << endl;
      it++;
   }
}

void     ClientNetworkWrapper::HandleListOfAggregatePurchases( const PacketListOfUserAggregatePurchases* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleListOfAggregatePurchases: " << endl;
   cout << " Num found: " << packet->purchases.size() << endl;
   cout << " user: " << packet->userUuid << endl;
   Threading::MutexLock    locker( m_notificationMutex );

   if( m_uuid.size() == 0 )
      m_uuid = packet->userUuid; // correcting for occassional order of packets arriving.
   if( packet->userUuid == m_uuid )
   {
      m_purchases = packet->purchases;
   }
   else
   {
      m_otherUsersPurchases = packet->purchases;
      m_otherUserPurchaseUuid = packet->userUuid;
   }
}

void     ClientNetworkWrapper::HandleListOfProducts( const PacketRequestListOfProductsResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleListOfProducts: " << endl;
   cout << " Num found: " << packet->products.size() << endl;
   ///Threading::MutexLock    locker( m_notificationMutex );

   U16 firstIndex = packet->products.GetFirstIndex();
   U16 totalCount = packet->products.GetTotalCount();
   U16 numProdctsInCurrentList = packet->products.size();
   m_notificationMutex.lock();
   if( packet->products.GetFirstIndex() == 0 )
   {
      m_products.clear();
   }
   
   int numToAdd = packet->products.size();
   for( int i=0; i<numToAdd; i++ )
   {
      m_products.push_back( packet->products[i] );
   }
   m_notificationMutex.unlock();
   if( firstIndex + numProdctsInCurrentList >= totalCount) // We are at the end of the list
   {
      Notification( ClientSideNetworkCallback::NotificationType_ListOfAvailableProducts );
   }

   //m_products = packet->products;
}

void     ClientNetworkWrapper::HandleBeingAddedByServerToChatChannel( const PacketChatUserAddedToChatChannelFromGameServer* chat )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleBeingAddedByServerToChatChannel: You received a chat channel addition: " << endl;

   Threading::MutexLock    locker( m_notificationMutex );
               
   bool found = false;
   vector< ChatChannel >::iterator it = m_channels.begin();
   while( it != m_channels.end() )
   {
      ChatChannel& channel = *it;
      if( channel.uuid == chat->channelUuid )
      {
         found = true;
         break;
      }
   }
   if( found == false )
   {
      ChatChannel newChannel;
      newChannel.uuid = chat->channelUuid.c_str();
      newChannel.channelName = chat->gameName;
      newChannel.gameInstanceId = chat->gameId;
      newChannel.channelDetails= chat->gameName;
      newChannel.userList = chat->userList;
      AddChatChannel( newChannel );
   }
}

void     ClientNetworkWrapper::HandleUserChatStatusChange( const PacketChatUserStatusChangeBase* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleUserChatStatusChange: user status changed" << endl;
   cout << packet->userName << " online status = " << boolalpha << packet->statusChange << noboolalpha << endl;
   
   Threading::MutexLock    locker( m_notificationMutex );

   bool updated = false;
   UserNameKeyValue::KVIterator  itFriends = m_contacts.begin();
   while( itFriends != m_contacts.end() )
   {
      if( itFriends->key == packet->uuid )
      {
         itFriends->value.isOnline = packet->statusChange ? true : false; // 0=offline, everything else is some kind of online
         updated = true;
      }
      itFriends++;
   }
}

void     ClientNetworkWrapper::HandleAddUserToChatChannel( const PacketChatAddUserToChatChannelResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleAddUserToChatChannel: char contacts received" << endl;

   Threading::MutexLock    locker( m_notificationMutex );

   if( packet->success == true )
   {
      AddUserToChatChannel( packet->channelUuid.c_str(), packet->userUuid.c_str(), packet->userName );
   }
}

void     ClientNetworkWrapper::HandleRemoveUserFromChatChannel( const PacketChatRemoveUserFromChatChannelResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleRemoveUserFromChatChannel: channel uuid: " << packet->chatChannelUuid.c_str() << " , userUuid = " << packet->userUuid  << endl;
             
   Threading::MutexLock    locker( m_notificationMutex );

   if( packet->success == true )
   {
      RemoveUserfromChatChannel( packet->chatChannelUuid.c_str(), packet->userUuid.c_str() );
   }
}

void     ClientNetworkWrapper::HandleListOfAssetCategoriesUpdate( const PacketAsset_GetListOfAssetCategoriesResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   int num = packet->assetcategory.size();
   cout << " HandleListOfAssetCategoriesUpdate: num categories: " << num << endl;
   Threading::MutexLock    locker( m_notificationMutex );
   
   m_assets.clear();

   for( int i=0; i< num; i++ )
   {
      const string& name = packet->assetcategory[i];
      AssetMapIter it = m_assets.find( name );
      if( it == m_assets.end() )
      {
         m_assets.insert( AssetMapPair( name, AssetCollection () ) );
      }
   }
}

void     ClientNetworkWrapper::HandleListOfAssets( const PacketAsset_GetListOfAssetsResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   const string& category = packet->assetCategory;
   assert( category.size() > 0 );

   cout << " HandleListOfAssets: category: " << category << endl;

   Threading::MutexLock    locker( m_notificationMutex );

   AssetMapIter it = m_assets.find( category );
   if( it == m_assets.end() )
   {
      m_assets.insert( AssetMapPair( category, AssetCollection () ) );
      it = m_assets.find( category );
   }

   AssetCollection& collection = it->second;
   collection.clear();

   SerializedKeyValueVector< AssetInfo >::const_KVIterator updatedAssetIt = packet->updatedAssets.begin();
   while ( updatedAssetIt != packet->updatedAssets.end() )
   {
      AssetInfoExtended extender( updatedAssetIt->value );
      collection.push_back( extender );
      updatedAssetIt ++;
   }
}


void     ClientNetworkWrapper::HandleListOfDevices( const PacketNotification_RequestListOfDevicesResponse* packet )
{
   PrintFunctionName( __FUNCTION__ );
   cout << " HandleListOfDevices: num = " << packet->devices.size() << endl;
   Threading::MutexLock    locker( m_notificationMutex );
   m_devices = packet->devices;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void     ClientNetworkWrapper::HandleAssetData( PacketGameplayRawData* data )
{
   PrintFunctionName( __FUNCTION__ );
   const string hash = data->identifier;
   AssetInfoExtended* asset = GetAsset( hash );
   bool  wasBoosted = false;
   if( asset )
   {
      asset->AddAssetData( data );
      if( asset->IsAssetFullyLoaded() )
      {
         //UpdateAssetData( hash, asset );
         SerializedKeyValueVector< string > strings;
         strings.insert( "hash", hash );
         strings.insert( "category", asset->category );

         Notification( ClientSideNetworkCallback::NotificationType_AssetDataAvailable, strings );
      }
      else
      {
         if( asset->GetRemainingSize() > 10*1024 )
         {
            BoostThreadPerformance();
            wasBoosted = true;
         }
      }      
   }
   else
   {
      cout << " unknown asset has arrived " << hash << endl;
   }

   if( wasBoosted == false )
   {
      RestoreNormalThreadPerformance();
   }
}

//------------------------------------------------------------------------

void     ClientNetworkWrapper::HandleData( PacketGameplayRawData* data )
{
   PrintFunctionName( __FUNCTION__ );
   //const string& hash = data->identifier;
   int dataType = data->dataType;
   cout << "packet type: " << dataType << ", index: " << (int) data->index << ", size:" << (int) data->size;
   if( m_lastRawDataIndex - data->index > 1 )
   {
      cout << "***";
   }
   m_lastRawDataIndex = data->index;
   cout << endl;

   if( dataType == PacketGameplayRawData::Asset )
   {
      HandleAssetData( data );
      return;
   }

   //RawDataAccumulator& rawDataBuffer = m_rawDataBuffer;
   m_rawDataBuffer.AddPacket( data );
   if( m_rawDataBuffer.GetRemainingSize() > 10*1024 )
   {
      BoostThreadPerformance();
   }
   

   if( m_rawDataBuffer.IsDone() )
   {
      //GameData* store = new GameData();
      U8* ptr;
      int size;
      m_rawDataBuffer.PrepPackage( ptr, size );         

      Notification( ClientSideNetworkCallback::NotificationType_GameData, ptr, size );

      RestoreNormalThreadPerformance();
   }
}

void     ClientNetworkWrapper::HandleChatChannelUpdate( BasePacket* packetIn )
{
   PrintFunctionName( __FUNCTION__ );
   PacketChatChannelList* packetChannels = static_cast<PacketChatChannelList*>( packetIn );

   cout << " chat channel list received " << packetChannels->channelList.size() << endl;
   const SerializedKeyValueVector< ChannelInfoFullList >& channels = packetChannels->channelList;

   U16 firstIndex = channels.GetFirstIndex();
   U16 totalCount = channels.GetTotalCount();
   U16 numChannelsInCurrentList = channels.size();

   if( firstIndex == 0 )
   {
      m_channels.clear();
   }

   // create scope
   {
      Threading::MutexLock    locker( m_notificationMutex );

      SerializedKeyValueVector< ChannelInfoFullList >::const_KVIterator channelIt = channels.begin();
      while( channelIt != channels.end() )
      {
         ChatChannel newChannel;
         newChannel.uuid = channelIt->value.channelUuid.c_str();
         newChannel.channelName = channelIt->value.channelName;
         newChannel.gameProductId = channelIt->value.gameProduct;
         newChannel.gameInstanceId = channelIt->value.gameId;
         newChannel.channelDetails = channelIt->value.channelName;
         newChannel.userList = channelIt->value.userList;// super expensive
         AddChatChannel( newChannel );
         channelIt++;
      }
   }

   if( firstIndex + numChannelsInCurrentList >= totalCount) // We are at the end of the list
   {
      Notification( ClientSideNetworkCallback::NotificationType_ChatListUpdate );
   }
}

//////////////////////////////////////////////////////////////////////////

//------------------------------------------------------------------------

//------------------------------------------------------------------------

void     ClientNetworkWrapper::NotifyClientToBeginSendingRequests()
{
   PrintFunctionName( __FUNCTION__ );
   if( m_isLoggedIn == false )
      return;

   if( m_selectedGame && m_wasCallbackForReadyToBeginSent == false )
   {
      Notification( ClientSideNetworkCallback::NotificationType_ReadyToStartSendingRequestsToGame );
      m_wasCallbackForReadyToBeginSent = true;
   }
}

//------------------------------------------------------------------------

bool     ClientNetworkWrapper::AddChatChannel( const ChatChannel& channel )
{
   PrintFunctionName( __FUNCTION__ );
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

bool     ClientNetworkWrapper::AddUserToChatChannel( const string& channelUuid, const string& userUuid, const string& userName )
{
   PrintFunctionName( __FUNCTION__ );
   vector< ChatChannel >::iterator channelIt = GetChannel( channelUuid );
   if( channelIt == m_channels.end() )
      return false;

   string name = userName;
   if( userName.length() == 0 )
   {
      name = FindContactFromUuid( userUuid );
   }
   if( name.length() == 0 )
   {
      return false;
   }

   ChatChannel& channel = *channelIt;
   SerializedKeyValueVector< string >::KVIterator it = channel.userList.begin();
   while( it != channel.userList.end() )
   {
      if( it->key == userUuid )
      {
         return false;
      }
      it++;
   }

   channel.userList.insert( userUuid, name );
   return true;
}

bool     ClientNetworkWrapper::RemoveUserfromChatChannel( const string& channelUuid, const string& userUuid )
{
   PrintFunctionName( __FUNCTION__ );
   vector< ChatChannel >::iterator channelIt = GetChannel( channelUuid );
   if( channelIt == m_channels.end() )
      return false;

   ChatChannel& channel = *channelIt;

   SerializedKeyValueVector< string >::KVIterator it = channel.userList.begin();
   while( it != channel.userList.end() )
   {
      if( it->key == userUuid )
      {
         channel.userList.erase( it );
         if( userUuid == m_uuid )// we removed ourselves
         {
            RemoveChannel( channelUuid );
         }
         return true;
      }
      it++;
   }
   return false;
}

bool     ClientNetworkWrapper::RemoveInvitationFromSent( const string& uuid )
{
   PrintFunctionName( __FUNCTION__ );
   SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsSent;
   SerializedKeyValueVector< InvitationInfo >::KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->key == uuid )
      {
         kvVector.erase( it );
         return true;
      }
      it++;
   }
   return false;
}

bool     ClientNetworkWrapper::RemoveInvitationFromReceived( const string& uuid )
{
   PrintFunctionName( __FUNCTION__ );
   SerializedKeyValueVector< InvitationInfo >& kvVector = m_invitationsReceived;
   SerializedKeyValueVector< InvitationInfo >::KVIterator it = kvVector.begin();
   while (it != kvVector.end() )
   {
      if( it->key == uuid )
      {
         kvVector.erase( it );
         return true;
      }
      it++;
   }
   return false;
}

//------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetAsset( const string& hash, AssetInfoExtended& asset )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

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

AssetInfoExtended* ClientNetworkWrapper::GetAsset( const string& hash )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

   AssetMapIter it = m_assets.begin();//find( category );
   while( it != m_assets.end() )
   {
      vector< AssetInfoExtended >& arrayOfAssets = it->second;
      it++;

      vector< AssetInfoExtended >::iterator itAssets = arrayOfAssets.begin();
      while( itAssets != arrayOfAssets.end() )
      {
         if( itAssets->assetHash == hash )
         {
            return &(*itAssets);
            
         }
         itAssets++;
      }
   }

   return NULL;
}

//------------------------------------------------------------------------

bool     ClientNetworkWrapper::GetAsset( const string& category, const string& hash, AssetInfoExtended& asset )
{
   PrintFunctionName( __FUNCTION__ );
   Threading::MutexLock    locker( m_notificationMutex );

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

bool     ClientNetworkWrapper::UpdateAssetData( const string& hash, AssetInfoExtended& asset )
{
   PrintFunctionName( __FUNCTION__ );
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

bool  NetworkLayerExtended::HandlePacketReceived( BasePacket* packetIn )
{
   bool wasHandled = false;
   switch( packetIn->packetType )
   {
      case PacketType_Asset:
      {
         switch( packetIn->packetSubType )
         {
         case PacketAsset::AssetType_EchoToClient:
            for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
            {
               ClientSideNetworkCallbackExtended* ptr = static_cast<ClientSideNetworkCallbackExtended*>( *it );
               ptr->AssetEcho();
            }
            wasHandled = true;
            break;
         }
      }
      break;
      case PacketType_Chat:
      {
         switch( packetIn->packetSubType )
         {
         case PacketChatToServer::ChatType_EchoToClient:
            for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
            {
               ClientSideNetworkCallbackExtended* ptr = static_cast<ClientSideNetworkCallbackExtended*>( *it );
               ptr->ChatEcho();
            }
            wasHandled = true;
            break;
         }
      }
      break;
      case PacketType_Contact:
      {
         switch( packetIn->packetSubType )
         {
         case PacketContact::ContactType_EchoToClient:
            for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
            {
               ClientSideNetworkCallbackExtended* ptr = static_cast<ClientSideNetworkCallbackExtended*>( *it );
               ptr->ContactEcho();
            }
            wasHandled = true;
            break;
         }
      }
      break;
      case PacketType_Login:
      {
         switch( packetIn->packetSubType )
         {
         case PacketLogin::LoginType_EchoToClient:
            for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
            {
               ClientSideNetworkCallbackExtended* ptr = static_cast<ClientSideNetworkCallbackExtended*>( *it );
               ptr->LoginEcho();
            }
            wasHandled = true;
            break;
         }
      }
      break;
      case PacketType_Gameplay:
      {
         switch( packetIn->packetSubType )
         {
         case PacketGameToServer::GamePacketType_EchoToServer:
            for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
            {
               ClientSideNetworkCallbackExtended* ptr = static_cast<ClientSideNetworkCallbackExtended*>( *it );
               ptr->GameEcho();
            }
            wasHandled = true;
            break;
         }
      }
      break;
      case PacketType_Purchase:
      {
         switch( packetIn->packetSubType )
         {
         case PacketPurchase::PurchaseType_EchoToClient:
            for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
            {
               ClientSideNetworkCallbackExtended* ptr = static_cast<ClientSideNetworkCallbackExtended*>( *it );
               ptr->PurchaseEcho();
            }
            wasHandled = true;
            break;
         }
      }
      break;
   }
   if( wasHandled == true )
   {
      PacketCleaner cleaner( packetIn );
      return true;
   }

   return ClientNetworkWrapper::HandlePacketReceived( packetIn );
}

//------------------------------------------------------------

void  NetworkLayerExtended::StartTime()
{
   for( list< ClientSideNetworkCallback* >::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it )
   {
      ClientSideNetworkCallbackExtended* ptr = static_cast<ClientSideNetworkCallbackExtended*>( *it );
      ptr->SetStartTime();
   }
}

void  NetworkLayerExtended::SendAssetEcho()
{
   PrintFunctionName( __FUNCTION__ );
   PacketAsset_EchoToServer packet;
   packet.uuid = m_uuid;
   packet.loginKey = m_loginKey;
   SerializePacketOut( &packet );
   StartTime();
}

void  NetworkLayerExtended::SendContactEcho()
{
   PrintFunctionName( __FUNCTION__ );
   PacketContact_EchoToServer packet;
   SerializePacketOut( &packet );
   StartTime();
}

void  NetworkLayerExtended::SendChatEcho()
{
   PrintFunctionName( __FUNCTION__ );
   PacketChat_EchoToServer packet;
   SerializePacketOut( &packet );
   StartTime();
}

void  NetworkLayerExtended::SendLoginEcho()
{
   PrintFunctionName( __FUNCTION__ );
   PacketLogin_EchoToServer packet;
   SerializePacketOut( &packet );
   StartTime();
}

void  NetworkLayerExtended::SendGameEcho()
{
   PrintFunctionName( __FUNCTION__ );
   PacketGame_EchoToServer packet;
   SerializePacketOut( &packet );
   StartTime();
}

void  NetworkLayerExtended::SendPurchaseEcho()
{
   PrintFunctionName( __FUNCTION__ );
   PacketPurchase_EchoToServer packet;
   SerializePacketOut( &packet );
   StartTime();
}

void  NetworkLayerExtended::SendNotification( U8 type, string additionalText )
{
   PrintFunctionName( __FUNCTION__ );
   PacketGame_Notification packet;
   packet.userUuid = m_uuid;
   packet.notificationType = type;
   packet.additionalText = additionalText;
   SerializePacketOut( &packet );
}

void  NetworkLayerExtended::SendGameTest()
{
   PrintFunctionName( __FUNCTION__ );
   PacketGame_TestHook packet;
   SerializePacketOut( &packet );
}

void  NetworkLayerExtended::SetNetworkVersionOverride( U8 ver )
{
   m_networkVersionOverride = ver;
}

//------------------------------------------------------------


void     ClientSideNetworkCallbackExtended::SetStartTime()
{
   m_timeSent = GetCurrentMilliseconds();
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

bool  RawDataAccumulator:: IsDone() const
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
   assert( IsDone() );
   asset.ClearData();

   asset.size = numBytes;
   asset.data = new U8[ asset.size + 1];
   U8* workingPointer = asset.data;

   deque< PacketGameplayRawData* >::iterator it = packetsOfData.begin();
   while( it != packetsOfData.end() )
   {
      PacketGameplayRawData* packet = *it++;
      memcpy( workingPointer, packet->data, packet->size );
      workingPointer += packet->size;
   }
   asset.data[ asset.size ] = 0;

   numBytes = 0;
   isReadyToBeCleared = true;
}

void  RawDataAccumulator:: ClearData()
{
   PacketFactory factory;
   while( packetsOfData.size() )
   {
      PacketGameplayRawData* packet = packetsOfData.front();
      packetsOfData.pop_front();

      BasePacket* temp = static_cast< BasePacket* >( packet );
      factory.CleanupPacket( temp );
   }
   isReadyToBeCleared = false;
}

bool  RawDataAccumulator:: NeedsClearing() const
{
   return isReadyToBeCleared && IsDone();
}

///////////////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
