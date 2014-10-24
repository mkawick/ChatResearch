// GatewayCommon.h
#pragma once

#include <string>

typedef std::deque< U32 >               ConnectionIdQueue;

enum GatewayConstants
{
   MaximumInputBufferSize = 6 * 1024
};


////////////////////////////////////////////////////////////////////////////////////

struct ConnectionIdStorage
{
   ConnectionIdStorage( U32 _id, U16 _count ) : id( _id ), countIds( _count ){}
   U32 id;
   U16 countIds;
};


////////////////////////////////////////////////////////

struct QOS_ServiceChange
{
   U8       serverType;
   U8       gameId;
   U8       errorTypeMessageToSend;
   bool     forceUsersToDc;
   bool     isConnected;
   
   std::string    text;

   QOS_ServiceChange() : serverType( 0 ), gameId( 0 ), errorTypeMessageToSend( 0 ), forceUsersToDc( false ), isConnected( false ){}
   QOS_ServiceChange( const QOS_ServiceChange& qos ) : 
         serverType( qos.serverType ), 
         gameId( qos.gameId ), 
         errorTypeMessageToSend( qos.errorTypeMessageToSend ), 
         forceUsersToDc( qos.forceUsersToDc ), 
         isConnected( qos.isConnected ), 
         text( qos.text )
         {}
};

////////////////////////////////////////////////////////

class FruitadensGateway;

struct ServerStatus
{
   ServerType           type;
   U8                   gameId; //
   bool                 isConnected;
   bool                 isEnabled;
   FruitadensGateway*   fruity;
};

////////////////////////////////////////////////////////

struct ClientConnectionForGame
{
   ClientConnectionForGame( U32 connId, U8 _gameId ) : connectionId( connId ), gameId( _gameId ) {}
   U8                   gameId;
   U32                  connectionId;
};

////////////////////////////////////////////////////////////////////////////////////
