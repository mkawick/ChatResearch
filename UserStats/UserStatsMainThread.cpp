#include "UserStatsMainThread.h"

#include <iostream>
#include <time.h>
#include <string>
using namespace std;


#include "../NetworkCommon/Packets/ServerToServerPacket.h"
#include "../NetworkCommon/Packets/PacketFactory.h"

#include "../NetworkCommon/Packets/DbPacket.h"
#include "../NetworkCommon/Packets/ChatPacket.h"
#include "../NetworkCommon/Packets/InvitationPacket.h"
#include "../NetworkCommon/Packets/LoginPacket.h"
#include "../NetworkCommon/Packets/AnalyticsPacket.h"

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
//using namespace boost;

UserStatsMainThread::UserStatsMainThread( const string& serverName, U32 serverId ): Diplodocus< KhaanServerToServer >( serverName, serverId, 0,  ServerType_UserStats )
{
   //time( &m_timestampStatsPrint );
   //m_timestampSelectPreferredGateway = m_timestampStatsPrint;
}

UserStatsMainThread::~UserStatsMainThread()
{
}

//---------------------------------------------------------------

void     UserStatsMainThread::ServerWasIdentified( IChainedInterface* khaan )
{
   BasePacket* packet = NULL;
   PackageForServerIdentification( m_serverName, m_localIpAddress, m_externalIpAddress, m_serverId, m_serverType, m_listeningPort, m_gameProductId, m_isGame, m_isControllerApp, true, m_gatewayType, &packet );
   ChainedType* localKhaan = static_cast< ChainedType* >( khaan );
   localKhaan->AddOutputChainData( packet, 0 );
   m_clientsNeedingUpdate.push_back( localKhaan->GetServerId() );
}

//---------------------------------------------------------------

int      UserStatsMainThread::MainLoop_InputProcessing()
{
   return 0;
}

int      UserStatsMainThread::MainLoop_OutputProcessing()
{
   //UpdateDbResults();
   
    UpdateAllConnections();

   time_t currentTime;
   time( &currentTime );

   int numClients = static_cast< int >( m_connectedClients.size() );
   UpdateConsoleWindow( m_timeOfLastTitleUpdate, m_uptime, m_numTotalConnections, numClients, m_listeningPort, m_serverName );

   
   UpdateDbResults();
   /*
   PeriodicWriteToDB();
   RemoveLoggedOutUsers();
   UpdateChatChannelManager();
   UpdateAllChatUsers();

   RunHourlyStats();
   RunDailyStats();
   StatTrackingConnections::SendStatsToStatServer( m_listOfOutputs, m_serverName, m_serverId, m_serverType );

   */
   return 0;
}


//---------------------------------------------------------------

bool     UserStatsMainThread::AddInputChainData( BasePacket* packet, U32 connectionId )
{
   U8 packetType = packet->packetType;

   switch( packetType )
   {
   case PacketType_GatewayInformation:
      {
         PacketCleaner cleaner( packet );
         //HandleCommandFromGateway( packet, connectionId );
         return true;
      }
   case PacketType_ServerJobWrapper:
      {
         PacketCleaner cleaner( packet );
         //HandlePacketFromOtherServer( packet, connectionId );
         return true;
      }
 
   case PacketType_DbQuery:
      {
      }
   case PacketType_GatewayWrapper:
      {
         PacketCleaner cleaner( packet );
         //HandlePacketFromClient( packet );
         return true;
      }
   }
   return false;
}

//---------------------------------------------------------------

bool   UserStatsMainThread::AddOutputChainData( BasePacket* packet, U32 connectionId ) 
{
   if( packet->packetType == PacketType_ServerJobWrapper )
   {
      //return HandlePacketToOtherServer( packet, connectionId );
      return false;
   }

   if( packet->packetType == PacketType_DbQuery )
   {
      // here we save the query

      Threading::MutexLock locker( m_mutex );
      if( packet->packetSubType == BasePacketDbQuery::QueryType_Result )
      {
         PacketDbQueryResult* result = static_cast< PacketDbQueryResult* >( packet );
         m_dbQueries.push_back( result );
         if( result->customData != NULL )
            cout << "AddOutputChainData: Non-null custom data " << endl;
      }
      return true;
      //return false;
   }

   
   return false;
}

//---------------------------------------------------------------

void     UserStatsMainThread::UpdateDbResults()
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

    /*  if( dbResult->serverLookup == m_chatRoomManager->GetDbIdentifier() ) //&& connectionId == ChatChannelManagerUniqueId )
      {
         if( m_chatRoomManager->HandleDbResult( dbResult ) == false )
         {
            factory.CleanupPacket( packet );
         }
         m_chatRoomManagerNeedsUpdate = true;
      }
      else if( dbResult->serverLookup == m_invitationManager->GetDbIdentifier() ) //&& connectionId == ChatChannelManagerUniqueId )
      {
         if( m_invitationManager->HandleDbResult( dbResult ) == false )
         {
            factory.CleanupPacket( packet );
         }
         m_invitationManagerNeedsUpdate = true;
      }
      else*/
      {
       /*  ChatUser* user = GetUser( connectionId );
         if( user )
         {
            user->HandleDbResult( dbResult );
         }
         else*/
         {
            factory.CleanupPacket( packet );
         }
      }
   }
}

//---------------------------------------------------------------

bool     UserStatsMainThread::SendMessageToClient( BasePacket* packet, U32 connectionId )
{
   if( packet->packetType == PacketType_GatewayWrapper )// this is already wrapped up and ready for the gateway... send it on.
   {
      m_inputChainListMutex.lock();
      ClientMap tempInputContainer = m_connectedClients;
      m_inputChainListMutex.unlock();

      ClientMapIterator itInputs = tempInputContainer.begin();
      if( itInputs != tempInputContainer.end() )// only one output currently supported.
      {
         KhaanServerToServer* khaan = static_cast< KhaanServerToServer* >( itInputs->second );
         khaan->AddOutputChainData( packet );
         m_clientsNeedingUpdate.push_back( khaan->GetChainedId() );
         itInputs++;
      }
      return true;
   }

 /*  if( packet->packetType == PacketType_DbQuery )
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
   }*/

   return false;
}

////////////////////////////////////////////////////////////////////////////////////////

bool     UserStatsMainThread::AddQueryToOutput( PacketDbQuery* dbQuery )
{
   PacketFactory factory;
   m_outputChainListMutex.lock();
   BaseOutputContainer tempOutputContainer = m_listOfOutputs;
   m_outputChainListMutex.unlock();

   ChainLinkIteratorType itOutputs = tempOutputContainer.begin();
   while( itOutputs != tempOutputContainer.end() )// only one output currently supported.
   {
      ChainType* outputPtr = static_cast< ChainType*> ( (*itOutputs).m_interface );
      ChainedInterface* interfacePtr = static_cast< ChainedInterface* >( outputPtr );
      if( interfacePtr->DoesNameMatch( "Deltadromeus" ) )
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
