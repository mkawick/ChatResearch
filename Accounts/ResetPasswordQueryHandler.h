#pragma once

#include "NewAccountQueryHandler.h"

///////////////////////////////////////////////////////////////////////////////////////////

class ResetPasswordQueryHandler : public NewAccountQueryHandler
{
public:
   ResetPasswordQueryHandler( int id, Queryer* parent, string& query );

   void     Update( time_t currentTime );
   bool     HandleResult( const PacketDbQueryResult* dbResult );

public:
   ResetPasswordQueryHandler();
   ~ResetPasswordQueryHandler();
};

///////////////////////////////////////////////////////////////////////////////////////////
