
#include "DiplodocusTools.h"
#include "../Logging/server_log.h"

void  LogMessage_LoginPacket( const PacketPrepareForUserLogin* loginPacket )
{
   U32      connectionId =    loginPacket->connectionId;
   string   uuid =            loginPacket->uuid;
   string   userName =        loginPacket->userName;
   U32      gatewayId =       loginPacket->gatewayId;
   string   password =        loginPacket->password;

   LogMessage( LOG_PRIO_INFO, "Prep for login: connection (%d:%u), %s, %s, %s", connectionId, gatewayId, userName.c_str(), uuid.c_str(), password.c_str() );
}

void  LogMessage_LogoutPacket( const PacketPrepareForUserLogout* logoutPacket )
{
   U32      connectionId =    logoutPacket->connectionId;
   string   uuid =            logoutPacket->uuid;
   //string   userName =        logoutPacket->userName;
//  U32      gatewayId =       logoutPacket->gatewayId;
   //string   password =        logoutPacket->password;

   LogMessage( LOG_PRIO_INFO, "Prep for logout: connection (%d), %s,", connectionId, uuid.c_str() );

   //LogMessage( LOG_PRIO_INFO, "Prep for logout: %d, %s", logoutPacket->connectionId, logoutPacket->uuid.c_str() );
   
}