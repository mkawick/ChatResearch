// DiplodocusContact.cpp

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
using boost::format;

#include "../NetworkCommon/Utils/CommandLineParser.h"
#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/ContactPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "DiplodocusContact.h"
#include "UserLookupManager.h"

#include <iostream>
using namespace std;
#include <time.h>

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

DiplodocusContact::DiplodocusContact( const string& serverName, U32 serverId ): Diplodocus< KhaanContact >( serverName, serverId, 0,  ServerType_Contact ),
                                             StatTrackingConnections(),
                                             m_numSearches( 0 ),
                                             m_numInvitesSent( 0 ),
                                             m_numInvitesAccepted( 0 ),
                                             m_numInvitesRejected( 0 ),
                                             m_hasRequestedAdminSettings( false ),
                                             m_timestampExpireOldInvitations( false ),
                                             m_invitationManager( NULL ),
                                             m_invitationManagerNeedsUpdate( false ),
                                             m_userLookupNeedsUpdate( false )
{
   SetSleepTime( 45 );
   time_t currentTime;
   time( &currentTime );
   m_timestampDailyStatServerStatisics = ZeroOutHours( currentTime );
   m_timestampHourlyStatServerStatisics = ZeroOutMinutes( currentTime );

   m_timestampExpireOldInvitations = 0;

   m_secondsBetweenSendingInvitationAndExpiration = 60*60 * 24 * 7;// 7 days;
}

void     DiplodocusContact::Init()
{
   m_userLookup = new UserLookupManager;
   m_userLookup->Init();
   m_userLookup->SetDbIdentifier( 1 );

   InvitationManager::Set( this );
   InvitationManager::Set( m_userLookup );
   m_invitationManager = new InvitationManager( Invitation::InvitationType_Friend );
   m_invitationManager->SetDbIdentifier( 2 );
   m_invitationManager->Init();

   Diplodocus< KhaanContact >::Init();
}

//---------------------------------------------------------------

void     DiplodocusContact::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );

   InputChainType* localKhaan = static_cast< InputChainType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_serversNeedingUpdate.push_back( localKhaan->GetServerId() );
}


//---------------------------------------------------------------

void     DiplodocusContact::RequestAdminSettings()
{
   PacketDbQuery* dbQuery = new PacketDbQuery;
   dbQuery->id = 0;
   dbQuery->lookup = QueryType_AccountAdminSettings;
   dbQuery->query = "SELECT * FROM admin_contact";

   m_isWaitingForAdminSettings = true;

   AddQueryToOutput( dbQuery );
}

//---------------------------------------------------------------

void     DiplodocusContact::HandleAdminSettings( const PacketDbQueryResult* dbResult )
{
   KeyValueParser              enigma( dbResult->bucket );
   KeyValueParser::iterator    it = enigma.begin();

   U32 basicTimeUnit = 60 * 60;// one hour default value
   U32 numUnits = 1;

   const U32 oneMinute = 60;
   const U32 oneHour = oneMinute * 60;
   const U32 oneDay = oneHour * 24;   
   const U32 oneWeek = oneDay * 7;
   const U32 oneMonth = oneDay * 30;// one month +/-;
   
   while( it != enigma.end() )
   {
      KeyValueParser::row      row = *it++;
      string setting =         row[ TableKeyValue::Column_key ];
      string value =           row[ TableKeyValue::Column_value ];

      if( setting == "invitation.expry.num_units" )
      {
         if( value.size() && value != "NULL" )
         {
            numUnits = boost::lexical_cast< U32 >( value );
         }
      }
      else if ( setting == "invitation.expry.unit" )
      {
         if( value == "hour" )
         {
            basicTimeUnit = oneHour;
         }
         else if ( value == "minute" )
         {
            basicTimeUnit = oneMinute;
         }
         else if ( value == "day" )
         {
            basicTimeUnit = oneDay;// one day
         }
         else if ( value == "week" )
         {
            basicTimeUnit = oneWeek;// one week
         }
         else if ( value == "month" )
         {
            basicTimeUnit = oneMonth;
         }
      }
   }
   
   m_secondsBetweenSendingInvitationAndExpiration = basicTimeUnit * numUnits;   

   // time clamping
   if( m_secondsBetweenSendingInvitationAndExpiration < oneMinute )
      m_secondsBetweenSendingInvitationAndExpiration = 0;
   else if( m_secondsBetweenSendingInvitationAndExpiration > oneMonth )
      m_secondsBetweenSendingInvitationAndExpiration = oneMonth;

   m_isWaitingForAdminSettings = false;
}

//---------------------------------------------------------------

void     DiplodocusContact::HandleExipiredInvitations( const PacketDbQueryResult* dbResult )
{
   // tbd ?
}

//---------------------------------------------------------------

bool     DiplodocusContact::AddInputChainData( BasePacket* packet, U32 connectionId )
{
  /* packets handled

      PacketType_Contact

      invite friend -> send notification to chat server
      search friends
      clear invite -> send notification to chat server
      accept invite -> send notification to chat server
      remove friend -> send notification to chat server?
      get user profile info
      get friend profile info
      modify user profile  -> push to all friends? Probably not.*/

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
      PacketGatewayWrapper* wrapper = static_cast< PacketGatewayWrapper* >( packet );
      BasePacket* unwrappedPacket = wrapper->pPacket;
      U32 connectionIdToUse = wrapper->connectionId;
      connectionIdToUse = connectionIdToUse;

      if( unwrappedPacket->packetType == PacketType_Contact )
      {
         UserContactMapIterator found = m_users.find( connectionId );
         if( found == m_users.end() )
         {
            SendErrorToClient( connectionId, PacketErrorReport::ErrorType_Contact_BadLoginKey );
            return false;
         }

         PacketContact* packetContact = static_cast< PacketContact* >( unwrappedPacket );
         bool result = found->second.HandleRequestFromClient( packetContact );
         result = result;
        
         PacketCleaner cleaner( packet );
         return true;
      }
      else
      {
         assert( 0 );
      }

      delete unwrappedPacket;
   }
   return false;
}

//---------------------------------------------------------------

 UserContact* DiplodocusContact::GetUser( U32 userId )
{
   UserIdToContactMapIterator it = m_userLookupById.find( userId );
   if( it == m_userLookupById.end() )
      return NULL;

   UserContactMapIterator found = m_users.find( it->second );
   if( found == m_users.end() )
      return NULL;

   return &found->second;
}

//---------------------------------------------------------------

UserContact* DiplodocusContact::GetUserByUsername( const string& userName )
{
   UserContactMapIterator it = m_users.begin(); //find( it->second );
   while( it != m_users.end() )
   {
      if( it->second.GetUserInfo().userName == userName )
         return &it->second;
      it++;
   }

   return NULL;
}


//---------------------------------------------------------------

UserContact* DiplodocusContact::GetUserByUuid( const string& uuid )
{
   UserContactMapIterator it = m_users.begin(); //find( it->second );
   while( it != m_users.end() )
   {
      if( it->second.GetUserInfo().uuid == uuid )
         return &it->second;
      it++;
   }

   return NULL;
}


string      DiplodocusContact::GetUserUuidByConnectionId( U32 connectionId )
{
   UserContactMapIterator found = m_users.find( connectionId );
   if( found == m_users.end() )
   {
      return NULL;
   }
   return found->second.GetUserInfo().uuid;
}

void        DiplodocusContact::GetUserConnectionId( const string& uuid, U32& connectionId )
{
   connectionId = 0;

   UserContact* contact = GetUserByUuid( uuid );
   if( contact )
      connectionId = contact->GetUserInfo().connectionId;
}

string      DiplodocusContact::GetUserName( const string& uuid )
{
   const UserContact*   user = GetUserByUuid( uuid );
   if( user == NULL )
      return string();
   return user->GetUserInfo().userName;
}


//---------------------------------------------------------------


bool     DiplodocusContact::HandleCommandFromGateway( BasePacket* packet, U32 connectionId )
{
   assert( 0 ); // unfinished
   return false;
}


//---------------------------------------------------------------


bool  DiplodocusContact::HandlePacketFromOtherServer( BasePacket* packet, U32 connectionId )// not thread safe
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

      case PacketLogin::LoginType_UserUpdateProfile:
         UpdateUserProfile( static_cast< PacketUserUpdateProfile* >( unwrappedPacket ) );
         return true;
      }
   }
   else if( unwrappedPacket->packetType == PacketType_Contact )
   {
      return HandlePacketRequests( static_cast< PacketContact* >( packet ), connectionId );
   }

   return false;
}

//---------------------------------------------------------------

// make sure to follow the model of the account server regarding queries. Look at CreateAccount
// 
bool     DiplodocusContact::AddOutputChainData( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )
   {
      if( m_connectionIdGateway == 0 )
         return false;

      Threading::MutexLock locker( m_mutex );

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         ChainedInterface* interfacePtr = static_cast< ChainedInterface* >( chainedInput.m_interface );
         if( interfacePtr->DoesNameMatch( "KhaanContact" ) )
         {
            KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( interfacePtr );
            if( khaan->GetServerId() == m_connectionIdGateway )
            {
               khaan->AddOutputChainData( packet );
               //khaan->Update();// the gateway may not have a proper connection id.

               m_serversNeedingUpdate.push_back( khaan->GetServerId() );
               return true;
            }
         }
      }
      return false;
   }

   if( packet->packetType == PacketType_DbQuery )
   {
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* dbResult = static_cast<PacketDbQueryResult*>( packet );

         Threading::MutexLock locker( m_mutex );
         m_dbQueries.push_back( dbResult );
         return true;
      }
   }

   return false;
}

//---------------------------------------------------------------

void  DiplodocusContact::UpdateDbResults()
{   
   PacketFactory factory;

   m_mutex.lock();
   deque< PacketDbQueryResult* > tempQueue = m_dbQueries;
   m_dbQueries.clear();
   m_mutex.unlock();

   deque< PacketDbQueryResult* >::iterator it = tempQueue.begin();
   while( it != tempQueue.end() )
   {
      PacketDbQueryResult* dbResult = *it++;
      if( dbResult->customData != NULL )
            cout << "UpdateDbResults: Non-null custom data " << endl;
      BasePacket* packet = static_cast<BasePacket*>( dbResult );

      U32 connectionId = dbResult->id;
      if( connectionId != 0 )
      {
         if( dbResult->serverLookup == m_userLookup->GetDbIdentifier() ) //&& connectionId == ChatChannelManagerUniqueId )
         {
            if( m_userLookup->HandleDbResult( dbResult ) == false )
            {
               factory.CleanupPacket( packet );
            }
            m_userLookupNeedsUpdate = true;
         }
         else if( dbResult->serverLookup == m_invitationManager->GetDbIdentifier() ) //&& connectionId == ChatChannelManagerUniqueId )
         {
            if( m_invitationManager->HandleDbResult( dbResult ) == false )
            {
               factory.CleanupPacket( packet );
            }
            m_invitationManagerNeedsUpdate = true;
         }
         else
         {
            UserContactMapIterator it = m_users.find( connectionId );
            if( it != m_users.end() )
            {
               cout << "Db query type: "<< dbResult->lookup << " handed off to UserContact #" << connectionId << endl;
               it->second.HandleDbQueryResult( dbResult );
            }
            else
            {
               cout << "ERROR: DB Result has user that cannot be found" << endl;
            }
         }
      }
      else
      {
         // local packet handling
         switch( dbResult->lookup )
         {
            case QueryType_AccountAdminSettings:
            {
               HandleAdminSettings( dbResult );
            }
            break;
            case QueryType_SelectExpiredInvitations:
            {
               HandleExipiredInvitations( dbResult );
            }
            break;
         }
      }

      PacketCleaner cleaner( packet );
   }
}

//-----------------------------------------------------------------------------------------

void     DiplodocusContact::TrackCountStats( StatTracking stat, float value, int sub_category )
{
   StatTrackingConnections::TrackCountStats( m_serverName, m_serverId, stat, value, sub_category );
}

//---------------------------------------------------------------

void     DiplodocusContact::RunHourlyStats()
{
   time_t currentTime;
   time( &currentTime );

   if( difftime( currentTime, m_timestampHourlyStatServerStatisics ) >= timeoutHourlyStatisics ) 
   {
      m_timestampHourlyStatServerStatisics = ZeroOutMinutes( currentTime );
   }
}

//---------------------------------------------------------------

void     DiplodocusContact::RunDailyStats()
{
   time_t currentTime;
   time( &currentTime );
   if( difftime( currentTime, m_timestampDailyStatServerStatisics ) >= timeoutDailyStatisics ) 
   {
      m_timestampDailyStatServerStatisics = ZeroOutHours( currentTime );

      TrackCountStats( StatTracking_ContactNumberSearchesForUserPerformed, static_cast< float >( m_numSearches ), 0 );
      TrackCountStats( StatTracking_ContactNumInvitesSentPerDay, static_cast< float >( m_numInvitesSent ), 0 );
      TrackCountStats( StatTracking_ContactAcceptedInvitesPerDay, static_cast< float >( m_numInvitesAccepted ), 0 );
      TrackCountStats( StatTracking_ContactRejectedInvitesPerDay, static_cast< float >( m_numInvitesRejected ), 0 );
      ClearStats();
   }
}

//---------------------------------------------------------------

void     DiplodocusContact::ExpireOldInvitations()
{
   if( m_hasRequestedAdminSettings == false )
   {
      RequestAdminSettings();
      m_hasRequestedAdminSettings = true;
      return;
   }
   else if( m_isWaitingForAdminSettings == true )
   {
      return;
   }

   // disabled
   if( m_secondsBetweenSendingInvitationAndExpiration == 0 )
      return;

   time_t currentTime;
   time( &currentTime );
   double pastSeconds = difftime( currentTime, m_timestampExpireOldInvitations );
   if( pastSeconds >= m_secondsBetweenSendingInvitationAndExpiration ) 
   {
      m_timestampExpireOldInvitations = currentTime;

      U32 seconds = static_cast< U32 >( pastSeconds );

      //SELECT * FROM playdek.friend_pending where sent_date < ( NOW() - interval 1000000 second );

      string whereClause = "FROM playdek.friend_pending WHERE sent_date < ( UTC_TIMESTAMP() - interval ";
      whereClause += boost::lexical_cast< string >( seconds );
      whereClause += " second )";

      string selectQuery = "SELECT * " + whereClause; // we can grab these and possibly inform connected users.
      string deleteQuery = "DELETE " + whereClause;

      //------------------------------

      PacketDbQuery* dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_SelectExpiredInvitations;
      dbQuery->query = selectQuery;
      dbQuery->isFireAndForget = false;

      AddQueryToOutput( dbQuery );
      

      dbQuery = new PacketDbQuery;
      dbQuery->id = 0;
      dbQuery->lookup = QueryType_DeleteExpiredInvitations;
      dbQuery->query = deleteQuery;
      dbQuery->isFireAndForget = true;

      AddQueryToOutput( dbQuery );
   }
}

//---------------------------------------------------------------

int      DiplodocusContact::CallbackFunction()
{
   UpdateDbResults();

   while( m_serversNeedingUpdate.size() )
   {
      U32 serverId = m_serversNeedingUpdate.front();
      m_serversNeedingUpdate.pop_front();

      Threading::MutexLock locker( m_mutex );
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
         ChainedInterface* interfacePtr = static_cast< ChainedInterface* >( chainedInput.m_interface );
         if( interfacePtr->DoesNameMatch( "KhaanContact" ) )
         {
            KhaanContact* khaan = static_cast< KhaanContact* >( interfacePtr );
            if( khaan->GetServerId() == serverId )
            {
               if( khaan->Update() == false )
               {
                  m_serversNeedingUpdate.push_back( serverId );
               }
            }
         }
      }
   }
   UpdateAllConnections();

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   RunHourlyStats();
   RunDailyStats();
   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   ExpireOldInvitations();   

   return 1;
}

//---------------------------------------------------------------

void     DiplodocusContact::UpdateAllConnections()
{
   m_mutex.lock();
   UserContactMapIterator it = m_users.begin();
   while( it != m_users.end() )
   {
      UserContact& contact = it->second;
      UserContactMapIterator temp = it++;
      if( contact.IsLoggedOut() )
      {
         if( contact.SecondsExpiredSinceLoggedOut() > SecondsBeforeRemovingLoggedOutUser )
         {
            m_userLookupById.erase( contact.GetUserInfo().id );
            
            m_users.erase( temp );
         }
      }
      else 
      {
         contact.Update();
      }
   }
   m_mutex.unlock();

   Parent::UpdateAllConnections();
}

//---------------------------------------------------------------

bool     DiplodocusContact::ConnectUser( PacketPrepareForUserLogin* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UserContactMapIterator it = m_users.find( connectionId );// don't do anything if this user is already logged in.
   if( it != m_users.end() )
      return false;

   bool found = false;
   // if the user is already here but relogged, simply 
   m_mutex.lock();
      it = m_users.begin();
      while( it != m_users.end() )
      {
         if( it->second.GetUserInfo().uuid == loginPacket->uuid ) 
         {
            found = true;
            U32 id = it->second.GetUserInfo().id;
            UserIdToContactMapIterator itIdToContact = m_userLookupById.find( id );
            if( itIdToContact != m_userLookupById.end() )
            {
               itIdToContact->second = connectionId;
            }
            it->second.SetConnectionId( connectionId );
            it->second.FinishLoginBySendingUserFriendsAndInvitations();
            
            m_users.insert( UserContactPair( connectionId, it->second ) );
            m_users.erase( it );
            break;
         }
         it++;
      }
   m_mutex.unlock();

   if( found == false )
   {

      UserInfo ui;
      ui.userName =        loginPacket->userName;
      ui.uuid =            loginPacket->uuid.c_str();
      ui.apple_id = "";
      ui.connectionId =    connectionId;
      ui.gameProductId =   loginPacket->gameProductId;
      ui.active =          loginPacket->active;
      ui.email =           loginPacket->email;
      ui.passwordHash =    loginPacket->password;
      ui.id =              loginPacket->userId;

      UserContact user( ui, connectionId );
      user.SetServer( this );

      m_mutex.lock();
      m_users.insert( UserContactPair( connectionId, user ) );
      m_userLookupById.insert( UserIdToContactPair( ui.id, connectionId ) );
      m_mutex.unlock();
   }
   return true;
}

//---------------------------------------------------------------

bool     DiplodocusContact::DisconnectUser( PacketPrepareForUserLogout* loginPacket )
{
   U32 connectionId = loginPacket->connectionId;

   UserContactMapIterator it = m_users.find( connectionId );
   if( it == m_users.end() )
      return false;

   it->second.NeedsUpdate();
   it->second.UserLoggedOut();

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusContact::UpdateUserProfile( const PacketUserUpdateProfile* profile )
{
   U32 connectionId = profile->connectionId;

   UserContactMapIterator it = m_users.find( connectionId );
   if( it == m_users.end() )
      return false;

   it->second.UpdateProfile( profile );

   return true;
}

//---------------------------------------------------------------

bool     DiplodocusContact::HandlePacketRequests( PacketContact* packet, U32 connectionId )
{
   UserContactMapIterator it = m_users.find( connectionId );
   if( it == m_users.end() )
      return false;

   bool wasHandled = it->second.HandleRequestFromClient( packet );

   if( wasHandled )
      return true;

   switch( packet->packetSubType )
   {
   case PacketContact::ContactType_GetListOfContacts:
      break;

 /*  case PacketGameToServer::GamePacketType_ListOfGames:
      IsUserAllowedToUseThisProduct( static_cast< PacketListOfGames* >( unwrappedPacket ) );
      delete unwrappedPacket;
      return true;*/
   }

   return false;
}

//---------------------------------------------------------------

bool     DiplodocusContact::AddQueryToOutput( PacketDbQuery* packet )
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

   delete packet;/// normally, we'd leave this up to the invoker to cleanup. 
   return false;
}

//---------------------------------------------------------------

void     DiplodocusContact::ClearStats()
{
   m_numSearches = 0;
   m_numInvitesSent = 0;
   m_numInvitesAccepted = 0;
   m_numInvitesRejected = 0;
}

//---------------------------------------------------------------

//---------------------------------------------------------------

bool     DiplodocusContact::SendMessageToClient( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )// this is already wrapped up and ready for the gateway... send it on.
   {
      Threading::MutexLock locker( m_mutex );

      ClientMapIterator itInputs = m_connectedClients.begin();
      if( itInputs != m_connectedClients.end() )// only one output currently supported.
      {
         KhaanContact* khaan = static_cast< KhaanContact* >( itInputs->second );
         khaan->AddOutputChainData( packet );
         m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
         itInputs++;
      }
      return true;
   }

   if( packet->packetType == PacketType_DbQuery )
   {
      Threading::MutexLock locker( m_outputChainListMutex );
      // we don't do much interpretation here, we simply pass output data onto our output, which should be the DB or other servers.
      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
      {
         ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
         if( outputPtr->AddInputChainData( packet, m_chainId ) == true )
         {
            break;
         }
         itOutputs++;
      }
      return true;
   }

   return false;
}


bool     DiplodocusContact::AddQueryToOutput( PacketDbQuery* dbQuery, U32 connectionId )
{
   PacketFactory factory;
   dbQuery->id = connectionId;

   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      if( outputPtr->AddInputChainData( dbQuery, m_chainId ) == true )
      {
         return true;
      }
      itOutputs++;
   }

   BasePacket* deleteMe = static_cast< BasePacket*>( dbQuery );

   factory.CleanupPacket( deleteMe );
   return false;
}

bool     DiplodocusContact::SendErrorToClient( U32 connectionId, PacketErrorReport::ErrorType error )
{
   return Diplodocus< KhaanContact >:: SendErrorToClient( connectionId, error, 0 );
}

//---------------------------------------------------------------

//
///////////////////////////////////////////////////////////////////
