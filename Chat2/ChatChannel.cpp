#include "ChatChannel.h"

ChatChannel::ChatChannel(): recordId( 0 ), 
                           isActive( false ), 
                           maxPlayers( DefaultMaxNumPlayersInChatchannel ), 
                           gameType( 0 ), 
                           gameInstanceId( 0 ), 
                           gameTurn( 0 )
{
}
