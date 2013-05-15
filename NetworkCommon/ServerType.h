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
   ServerType_Test
};

template < typename type >
struct Range
{
   type   low, high;
};

class BasePacket;
#include <deque>

typedef std::deque< BasePacket* >       PacketQueue;