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
      Column_password_hash,
      Column_email,
      Column_user_gamekit_id,
      Column_user_gamekit_id_hash,
      Column_user_creation_date,
      Column_uuid,
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
      Column_id,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableSimpleUser> SimpleUserTable;

//////////////////////////////////////////////////////////////

class TableUserTempNewUser
{
public:
   enum Columns
   {
      Column_id,
      Column_name,
      Column_user_id,
      Column_email,
      Column_lookup_key,
      Column_game_id,
      Column_time_created,
      Column_was_email_sent,
      Column_language_id,
      Column_uuid,
      Column_gamekit_hash,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserTempNewUser> NewUsersTable;

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

class StringsTable
{
public:
   enum Columns
   {
      Column_id,
      Column_string,
      Column_category,
      Column_description,
      Column_english,
      Column_spanish,
      Column_french,
      Column_german,
      Column_italian,
      Column_portuguese,
      Column_russian,
      Column_japanese,
      Column_chinese,
      Column_end
   };
};

typedef Enigmosaurus <StringsTable> StringTableParser;

//////////////////////////////////////////////////////////////

class TableIndexOnly
{
public:
   enum Columns
   {
      Column_index,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableIndexOnly> IndexTableParser;

//////////////////////////////////////////////////////////////

class ConfigTable
{
public:
   enum Columns
   {
      Column_id,
      Column_key,
      Column_value,
      Column_category,
      Column_end
   };
};

typedef Enigmosaurus <ConfigTable> ConfigParser;

//////////////////////////////////////////////////////////////
