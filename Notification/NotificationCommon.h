// NotificationCommon.h

#include "../NetworkCommon/Packets/PacketFactory.h"

//////////////////////////////////////////////////////////////

class TableUserDevice
{
public:
   enum Columns
   {
      Column_id,
      Column_user_uuid,
      Column_device_uuid,
      Column_device_id,
      Column_name,
      Column_icon_id,
      Column_platformId,
      Column_user_id,
      Column_is_enabled,
      Column_registered_date,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserDevice> UserDeviceTable;

//////////////////////////////////////////////////////////////

class TableUserDeviceNotifications
{
public:
   enum Columns
   {
      Column_id,
      Column_user_device_id,
      Column_game_type,
      Column_is_enabled,
      Column_time_last_changed,
      Column_device_id,
      Column_end
   };
   static const char* const column_names[];
};

typedef Enigmosaurus <TableUserDeviceNotifications> UserDeviceNotificationsTable;

//////////////////////////////////////////////////////////////
