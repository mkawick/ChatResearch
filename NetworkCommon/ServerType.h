#pragma once

enum  ServerType
{
   ServerType_Gateway,
   ServerType_GameInstance,
   ServerType_Account,
   ServerType_Database,
   ServerType_Log,
   ServerType_Login,
   ServerType_Process,
   ServerType_Tournament,
   ServerType_General,
   ServerType_Chat,
   ServerType_Controller,
   ServerType_Asset,
   ServerType_Contact,
   ServerType_Purchase,
   ServerType_LoadBalancer,
   ServerType_Analytics,
   ServerType_Test,
   ServerType_Starter,
   ServerType_Notification,
   ServerType_PlayerStats,
   ServerType_Count // << end of list
};

const char* GetServerTypeName( ServerType );

template < typename type >
struct Range
{
   type   low, high;
};

class BasePacket;
#include <deque>

typedef std::deque< BasePacket* >       PacketQueue;