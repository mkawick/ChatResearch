// BlankUUIDQueryHandler.h

#pragma once

#include "../NetworkCommon/Database/QueryHandler.h"


///////////////////////////////////////////////////////////////////////////////////////////

class BlankUUIDQueryHandler : public QueryHandler
{
public:
   BlankUUIDQueryHandler( int id, Queryer* parent, string& query );
   bool     HandleResult( const PacketDbQueryResult* dbResult );
   void     UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId );

private:
   BlankUUIDQueryHandler();
};

///////////////////////////////////////////////////////////////////////////////////////////
