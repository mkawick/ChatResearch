#pragma once
#include "NewAccountQueryHandler.h"


///////////////////////////////////////////////////////////////////////////////////////////

class ResetUserEmailQueryHandler :  public NewAccountQueryHandler
{
public:
ResetUserEmailQueryHandler( U32 id, Queryer* parent, string& query );

   void     Update( time_t currentTime );
   bool     HandleResult( const PacketDbQueryResult* dbResult );

public:
   ResetUserEmailQueryHandler();
   ~ResetUserEmailQueryHandler();
   bool                 m_isServicingResetPassword;
};
#pragma once




///////////////////////////////////////////////////////////////////////////////////////////
