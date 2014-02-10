#include "ChatChannel.h"

ChatChannel::ChatChannel(): recordId( 0 ), 
                           isActive( false ), 
                           maxPlayers( DefaultMaxNumPlayersInChatchannel ), 
                           gameType( 0 ), 
                           gameId( 0 ), 
                           gameTurn( 0 )
{
}
