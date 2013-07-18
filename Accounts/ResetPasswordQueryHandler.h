#pragma once

#include "NewAccountQueryHandler.h"

///////////////////////////////////////////////////////////////////////////////////////////

class ResetPasswordQueryHandler : public NewAccountQueryHandler
{
public:
   ResetPasswordQueryHandler( U32 id, Queryer* parent, string& query );

   void     Update( time_t currentTime );
   bool     HandleResult( const PacketDbQueryResult* dbResult );

public:
   ResetPasswordQueryHandler();
   ~ResetPasswordQueryHandler();
   bool                 m_isServicingResetPassword;
};

///////////////////////////////////////////////////////////////////////////////////////////
