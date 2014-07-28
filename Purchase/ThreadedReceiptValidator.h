#pragma once

#include "../NetworkCommon/ChainedArchitecture\Thread.h"

class ThreadedReceiptValidator : public Threading::CAbstractThread
{
public:
   ThreadedReceiptValidator( int platformId, const string& url, const string& receipt );
   ~ThreadedReceiptValidator();

   int   CallbackFunction();
   bool  IsFinished() const { return m_isFinished; }
public:
   bool  m_isFinished;
   bool  m_hasStarted;
};
