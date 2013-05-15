#include "TableWrapper.h"

const char* const TableUser::column_names[] = 
{
   "id",
   "name",
   "uuid",
   "birth_date",
   "last_login_timestamp",
   "last_logout_timestamp",
   ""
};

const char* const TableChatChannel::column_names[] = 
{
   "id",
   "name",
   "uuid",
   "is_active",
   ""
};
