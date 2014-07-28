#include "ThreadedReceiptValidator.h"

ThreadedReceiptValidator::ThreadedReceiptValidator( int platformId, const string& url, const string& receipt ) : 
                                                   m_isFinished( false ),
                                                   m_hasStarted( false )
{
}

ThreadedReceiptValidator::~ThreadedReceiptValidator()
{
}

int       ThreadedReceiptValidator::CallbackFunction()
{
   if( m_hasStarted == false )
   {
      // initialize
      m_hasStarted = true;
   }
   return 0;
}
