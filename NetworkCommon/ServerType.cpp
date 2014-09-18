// ServerType.cpp

#include "ServerType.h"

const char* GetServerTypeName( ServerType type )
{
   switch( type )
   {
   case ServerType_Gateway:
      return "Gateway";
   case ServerType_GameInstance:
      return "Game";
   case ServerType_Account:
      return "Account";
   case ServerType_Database:
      return "DB";
   case ServerType_Log:
      return "Logging";
   case ServerType_Login:
      return "Login";
   case ServerType_Process:
      return "Process";
   case ServerType_Tournament:
      return "Tournament";
   case ServerType_General:
      return "General";
   case ServerType_Chat:
      return "Chat";
   case ServerType_Controller:
      return "Controller";
   case ServerType_Asset:
      return "Asset";
   case ServerType_Contact:
      return "Contact";
   case ServerType_Purchase:
      return "Purchase";
   case ServerType_LoadBalancer:
      return "Load balancer";
   case ServerType_Analytics:
      return "Analytics";
   case ServerType_Test:
      return "Test";
   case ServerType_Starter:
      return "starter";
   case ServerType_Notification:
      return "Notification";
   case ServerType_UserStats:
      return "UserStats";
   case ServerType_Count:
      return "Number of server... this is an error";
   }

   return "bug: bad reference value";
}