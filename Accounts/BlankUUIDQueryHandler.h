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
   void     UpdateUuidForUser( const string& recordId, const string additionalHashText );

   void     UseUserTable( bool useUserTable = true ){ m_useUserTable = useUserTable; }

   string   GenerateUuid();

   void     Update( time_t currentTime );

private:
   BlankUUIDQueryHandler();

   void     GenerateAListOfAvailableUUIDS();
   

   bool     m_isServicingBlankUUID;
   bool     m_useUserTable;

   list  < string > m_unusedUuids;
   int      m_numberPendingUuids;
};

///////////////////////////////////////////////////////////////////////////////////////////
