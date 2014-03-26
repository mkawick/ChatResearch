#pragma once
#include "Thread.h"
#include "ChainedInterface.h"

namespace Threading
{
template <typename Type> 
class CChainedThread : public CAbstractThread, public ChainedInterface <Type>
{
public: 
   enum { DefaultSleepTime = 30 };
public:
   // consider leaving the need protection false. This should be managed in the 
	CChainedThread( bool needsThreadProtection = false, int sleepTime = 0, bool paused = false );


   // inherited classes must provide ths function. Mutexes will be locked
   virtual int       MainLoop_InputProcessing() { return 0; };
   virtual int       MainLoop_OutputProcessing() { return 0; };

   //------------------------------------------------------

protected:

   virtual void      CallbackOnCleanup();

   // consider not overriding this function. You should achieve the results you need by 
   // overriding the MainLoop_InputProcessing and MainLoop_OutputProcessing functions
   virtual int       CallbackFunction();
   
};


/////////////////////////////////////////////////////////////////////////////
#include "Thread.inl"
}// namespace Threading
/////////////////////////////////////////////////////////////////////////////
/*
class MyHandler::CChainedThread< BasePacket* >
{
   int       MainLoop_InputProcessing() 
   { 
      // look at queue of data
   };
};


class MyHandler2::CChainedThread< BasePacket* >
{
   int       MainLoop_InputProcessing() 
   { 
      // look at queue of data
   };
};*/