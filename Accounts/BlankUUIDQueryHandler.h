// BlankUUIDQueryHandler.h

#pragma once

#include "../NetworkCommon/Database/QueryHandler.h"


///////////////////////////////////////////////////////////////////////////////////////////

// normally we only deal with user_temp_new_user
class BlankUUIDQueryHandler : public QueryHandler< Queryer* >
{
public:
   typedef QueryHandler< Queryer* > ParentType;

public:
   BlankUUIDQueryHandler( U32 id, Queryer* parent, string& query );
   bool     HandleResult( const PacketDbQueryResult* dbResult );

   //void     UpdateUuidForUser( const string& userId, bool updateCreateAccountTableToo, const string& columnId, const string additionalHashText = "" );
   void     UpdateUuidForTempUser( const string& recordId, const string additionalHashText = "" );

   string   GenerateUuid();

   void     Update( time_t currentTime );

private:
   BlankUUIDQueryHandler();

   void     GenerateAListOfAvailableUUIDS();
   

   bool     m_isServicingBlankUUID;

   list  < string > m_unusedUuids;
   int      m_numberPendingUuids;
};

///////////////////////////////////////////////////////////////////////////////////////////
