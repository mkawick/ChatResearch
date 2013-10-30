// BlankUUIDQueryHandler.h

#pragma once

#include "../NetworkCommon/Database/QueryHandler.h"


///////////////////////////////////////////////////////////////////////////////////////////

// normally we only deal with user_temp_new_user
class BlankUUIDQueryHandler : public QueryHandler< Queryer* >
{
public:
   BlankUUIDQueryHandler( U32 id, Queryer* parent, string& query );
   bool     HandleResult( const PacketDbQueryResult* dbResult );

   //void     UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId, const string additionalHashText = "" );
   void     UpdateUuidForTempUser( const string& recordId, const string additionalHashText = "" );

   string   GenerateUuid( const string& userId, const string& additionalHashText );

   void     Update( time_t currentTime );

private:
   BlankUUIDQueryHandler();

   void     PrepQueryToLookupUuid( const string& userId, const string& additionalHashText );
   

   bool     m_isServicingBlankUUID;
};

///////////////////////////////////////////////////////////////////////////////////////////
