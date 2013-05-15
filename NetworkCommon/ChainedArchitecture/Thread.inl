// Thread.inl

#pragma once


//////////////////////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------
//////////////////////////////////////////////////////////////////////////////////////////////

template <typename Type> 
CChainedThread<Type>::CChainedThread( bool needsThreadProtection, int sleepTime, bool paused ) : CAbstractThread( needsThreadProtection, sleepTime, paused ), ChainedInterface <BasePacket*>()
{
}

//----------------------------------------------------------------

template <typename Type> 
void  CChainedThread<Type>::CallbackOnCleanup()
{
   CleanupAllChainDependencies();
   CAbstractThread::CallbackOnCleanup();
}

//----------------------------------------------------------------

template <typename Type> 
int       CChainedThread<Type>::CallbackFunction()
{
   ProcessEvents();

   m_inputChainListMutex.lock();
   ProcessInputFunction();
   m_inputChainListMutex.unlock();

   //-------------------------------

   m_outputChainListMutex.lock();
   ProcessOutputFunction();
   m_outputChainListMutex.unlock();

   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////
