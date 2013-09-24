// BlankUserProfileHandler.h

#pragma once

#include "../NetworkCommon/Database/QueryHandler.h"


///////////////////////////////////////////////////////////////////////////////////////////

class BlankUserProfileHandler : public QueryHandler
{
public:
   BlankUserProfileHandler( U32 id, Queryer* parent, string& query );
   bool     HandleResult( const PacketDbQueryResult* dbResult );
   void     CreateBlankProfile( const string& user_id );

   void     Update( time_t currentTime );

private:
   BlankUserProfileHandler();
   bool     m_isServicingBlankUUID;
};

///////////////////////////////////////////////////////////////////////////////////////////
