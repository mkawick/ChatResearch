// TableWrapper.h

#pragma once

#include "Enigmosaurus.h"

//////////////////////////////////////////////////////////////

class TableSingleColumn
{
public:
   enum Columns
   {
      Column_text,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableSingleColumn> SingleColumnTable;


//////////////////////////////////////////////////////////////

class TableUser
{
public:
   enum Columns
   {
      Column_id,
      Column_name,
      Column_name_match,
      Column_password_hash,
      Column_email,
      Column_user_gamekit_id,
      Column_user_gamekit_id_hash,
      Column_user_creation_date,
      Column_uuid,
      Column_last_login_time,
      Column_last_logout_time,
      Column_active,
      Column_language_id,
      Column_user_confirmation_date,
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

class TableUserJoinChatChannel
{
public:
   enum Columns
   {
      Column_name,
      Column_uuid,
      Column_id,
      Column_channel_uuid,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserJoinChatChannel> UserJoinChatChannelTable;

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
      Column_user_pw_hash,
      Column_user_name_match,
      Column_flagged_as_invalid,
      Column_flagged_auto_create,
      Column_time_last_confirmation_email_sent,
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
      Column_game_type, // summoner wars
      Column_game_instance_id, // game instance
      Column_date_created,
      Column_date_expired,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableChatChannel> ChatChannelTable;

//////////////////////////////////////////////////////////////
/*
class TableChatChannelWithNumChats
{
public:
   enum Columns
   {
      Column_id,
      Column_name,
      Column_uuid,
      Column_is_active,
      Column_max_num_members,
      Column_game_type, // summoner wars
      Column_game_instance_id, // game instance
      Column_date_created,
      Column_record_count,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableChatChannelWithNumChats> ChatChannelWithNumChatsTable;*/

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
      Column_user_uuid,
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
      Column_replaces,
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

class TableKeyValue
{
public:
   enum Columns
   {
      Column_key,
      Column_value,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableKeyValue> KeyValueParser;

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

//////////////////////////////////////////////////////////////

class TableUserProfile
{
public:
   enum Columns
   {
      Column_id,
      Column_address1,
      Column_address2,
      Column_city,
      Column_provence,
      Column_mail_code,
      Column_country,
      Column_marketing_opt_out,
      Column_screen_name,
      Column_gender,
      Column_mber_avatar,
      Column_home_phone,
      Column_alt_phone,
      Column_show_gender_profile,
      Column_admin_level,
      Column_show_win_loss_record,
      Column_time_zone,
      Column_account_create_product_id,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserProfile> UserProfileTable;

//////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////

class TableUserPlusProfile
{
public:
   enum Columns
   {
      Column_id,
      Column_name,
      Column_name_match,
      Column_password_hash,
      Column_email,
      Column_user_gamekit_id,
      Column_user_gamekit_id_hash,
      Column_user_creation_date,
      Column_uuid,
      Column_last_login_time,
      Column_last_logout_time,
      Column_active,
      Column_language_id,
      Column_user_confirmation_date,
      Column_id_from_profile,
      Column_address1,
      Column_address2,
      Column_city,
      Column_provence,
      Column_mail_code,
      Column_country,
      Column_marketing_opt_out,
      Column_screen_name,
      Column_gender,
      Column_mber_avatar,
      Column_home_phone,
      Column_alt_phone,
      Column_show_gender_profile,
      Column_admin_level,
      Column_show_win_loss_record,
      Column_time_zone,
      Column_account_create_product_id,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserPlusProfile> UserPlusProfileTable;

//////////////////////////////////////////////////////////////

class TableUserJoinPending
{
public:
   enum Columns
   {
      Column_id,
      Column_name,
      Column_name_match,
      Column_password_hash,
      Column_email,
      Column_user_gamekit_id,
      Column_user_gamekit_id_hash,
      Column_user_creation_date,
      Column_uuid,
      Column_last_login_time,
      Column_last_logout_time,
      Column_active,
      Column_language_id,
      Column_user_confirmation_date,
      Column_pending_id,
      Column_inviter_id,
      Column_invitee_id,
      Column_was_notified,
      Column_sent_date,
      Column_message,
      Column_pending_uuid,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserJoinPending> UserJoinPendingTable;

//////////////////////////////////////////////////////////////

class TableFriendPending
{
public:
   enum Columns
   {
      Column_id,
      Column_inviter_id,
      Column_invitee_id,
      Column_was_notified,
      Column_sent_date,
      Column_message,
      Column_pending_uuid,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableFriendPending> UserFriendPendingTable;

//////////////////////////////////////////////////////////////

class TableProduct
{
public:
   enum Columns
   {
      Column_id,
      Column_product_id,
      Column_uuid,
      Column_name,
      Column_filter_name,
      Column_begin_date,
      Column_product_type,
      Column_notes,
      Column_name_string,
      Column_icon_lookup,
      Column_end
   };
};

typedef Enigmosaurus <TableProduct> ProductTable;

//////////////////////////////////////////////////////////////

class TableUserProduct
{
public:
   enum Columns
   {
      Column_id,
      Column_user_uuid,
      Column_product_id,
      Column_purchase_date,
      Column_price_paid,
      Column_currency_type,
      Column_num_purchased,
      Column_admin_provided,
      Column_admin_notes,
      Column_retail_campaign,
      Column_end
   };
};

typedef Enigmosaurus <TableUserProduct> UserProductTable;

//////////////////////////////////////////////////////////////

class TableProductJoinUserProduct
{
public:
   enum Columns
   {
      Column_id,
      Column_product_id,
      Column_uuid,
      Column_name,
      Column_filter_name,
      Column_begin_date,
      Column_product_type,
      Column_notes,
      Column_id_user_join_product,
      Column_user_uuid,
      Column_product_id2,
      Column_purchase_date,
      Column_price_paid,
      Column_currency_type,
      Column_num_purchased,
      Column_admin_provided,
      Column_admin_notes,
      Column_retail_campaign,
      Column_end
   };
};

typedef Enigmosaurus <TableProductJoinUserProduct> ProductJoinUserProductTable;

//////////////////////////////////////////////////////////////

class TableUserOwnedProductSimple
{
public:
   enum Columns
   {
      Column_product_id,  // product_id, filter_name, product.uuid, num_purchased
      Column_product_name_string,
      Column_product_uuid,
      Column_quantity,
      Column_filter_name,
      Column_end
   };
};

typedef Enigmosaurus <TableUserOwnedProductSimple> UserOwnedProductSimpleTable;

//////////////////////////////////////////////////////////////

class TableUser_IdUUidDate
{
public:
   enum Columns
   {
      Column_id,
      Column_uuid,
      Column_date,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUser_IdUUidDate> User_IdUUidDateParser;

//////////////////////////////////////////////////////////////