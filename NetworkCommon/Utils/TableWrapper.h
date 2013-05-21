// TableWrapper.h

#pragma once

#include "Enigmosaurus.h"

//////////////////////////////////////////////////////////////

class TableUser
{
public:
   enum Columns
   {
      Column_id,
      Column_name,
      Column_uuid,
      Column_birth_date,
      Column_last_login_time,
      Column_last_logout_time,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUser> UserTable;

//////////////////////////////////////////////////////////////

class TableSimpleUser
{
public:
   enum Columns
   {
      Column_name,
      Column_uuid,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableSimpleUser> SimpleUserTable;

//////////////////////////////////////////////////////////////

class TableChatChannel
{
public:
   enum Columns
   {
      Column_id,
      Column_name,
      Column_uuid,
      Column_is_active,
      Column_max_num_members,
      Column_is_server_created,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableChatChannel> ChatChannelTable;

//////////////////////////////////////////////////////////////

class TableChat
{
public:
   enum Columns
   {
      Column_id,
      Column_text,
      Column_user_id_sender,
      Column_user_id_recipient,
      Column_chat_channel_id,
      Column_timestamp,
      Column_game_turn,
      Column_game_instance_id,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableChat> ChatTable;

//////////////////////////////////////////////////////////////

class SimpleChat
{
public:
   enum Columns
   {
      Column_text,
      Column_user_name,
      
      Column_game_turn,
      Column_timestamp,
      Column_end
   };
};


typedef Enigmosaurus <SimpleChat> SimpleChatTable;

//////////////////////////////////////////////////////////////

class SimpleGame
{
public:
   enum Columns
   {
      Column_uuid,
      Column_name,
      Column_end
   };
};

typedef Enigmosaurus <SimpleGame> SimpleGameTable;

//////////////////////////////////////////////////////////////