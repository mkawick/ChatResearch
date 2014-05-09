#include "ChatRoom.h"

ChatRoom::ChatRoom(): recordId( 0 ), 
                           isActive( false ), 
                           maxPlayers( DefaultMaxNumPlayersInChatRoom ), 
                           gameType( 0 ), 
                           gameInstanceId( 0 ), 
                           gameTurn( 0 )
{
}
