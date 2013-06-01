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

template< typename Type >
void  CChainedThread< Type >::CallbackOnCleanup()
{
   ChainedInterface< Type > :: CleanupAllChainDependencies();
   CAbstractThread::CallbackOnCleanup();
}

//----------------------------------------------------------------

template <typename Type> 
int       CChainedThread<Type>::CallbackFunction()
{
   ChainedInterface< Type >::ProcessEvents();

   ChainedInterface< Type >::m_inputChainListMutex.lock();
   ProcessInputFunction();
   ChainedInterface< Type >::m_inputChainListMutex.unlock();

   //-------------------------------

   ChainedInterface< Type >::m_outputChainListMutex.lock();
   ProcessOutputFunction();
   ChainedInterface< Type >::m_outputChainListMutex.unlock();

   return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////BasePacket
