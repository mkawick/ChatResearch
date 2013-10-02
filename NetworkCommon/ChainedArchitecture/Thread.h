// Thread.h

#pragma once // replaced the compile guards, this can be faster in VS2010 or earlier

#include <deque>
#include <list>
#include <string>
#include "../DataTypes.h"

#ifndef BasePacket

#include "../Packets/BasePacket.h"
#include "../Utils/Utils.h"

#endif

#if PLATFORM == PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // preventing issues with winsock, only
#include <winsock2.h>


typedef HANDLE             ThreadMutex;
typedef HANDLE             ThreadId;

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

#define _MULTI_THREADED
#include <pthread.h>
#include <sched.h>
#include <unistd.h>

typedef pthread_mutex_t    ThreadMutex;
typedef pthread_t          ThreadId;
typedef pthread_attr_t     ThreadAttributes;

#endif

namespace Threading
{
/////////////////////////////////////////////////////////////////////////////

   enum ThreadEnum
   {
      InvalidThread = 0
   };

class Mutex
{
public:
   Mutex();
   ~Mutex();
   bool  lock() const;
   bool  unlock() const;

   bool  IsLocked() const { return m_isLocked; }
   int	 NumPendingLockRequests() const { return m_pendingLockReqs; }

private:
   mutable ThreadMutex    m_mutex;
   mutable bool           m_isLocked;
   mutable int            m_pendingLockReqs;
};

class MutexLock
{
public:
	explicit MutexLock( Mutex& mutex ): m_mutex( mutex ) { m_mutex.lock(); }
	~MutexLock() { m_mutex.unlock(); }

	Mutex& m_mutex;
};

/////////////////////////////////////////////////////////////////////////////

// typical use case
/*
   class Thread01 : public CAbstractThread
   {
   public:
      Thread01( int timeOut ) : CAbstractThread( true, timeOut ), m_count( 0 ) {}

      int CallbackFunction()
      {
         m_count ++;
         cout << "Running " << m_count << endl;
         return 0;
      }
      int GetCount() const { return m_count; }

   private:
      ~Thread01() {}

   private:
      int m_count;
   };


   bool  RunCounterThread()
   {
      cout << "Count test start" << endl;
      Thread01* thread = new Thread01( 100 );
      Sleep(20000);
      cout << "Count test end" << endl;

      int num = thread->GetCount();
      thread->Cleanup();

      if( num > 195 ) return true;// it is threaded but should count at least to 195
      return false;
   }
*/
/////////////////////////////////////////////////////////////////////////////

class CAbstractThread
{
public:
   enum ePriority
   {
      ePriorityLow, 
      ePriorityNormal, 
      ePriorityHigh
   };

public:
	CAbstractThread( bool needsThreadProtection = false, int sleepTime = 0, bool paused = false );
   void  Cleanup(); // invoke this instead of the d'tor

   void              SetPriority( ePriority );
   bool              IsRunning() const { return m_running; }
   bool              IsThreadLocked() const { return m_mutex.IsLocked(); }
   int				   GetSleepTime() const { return m_sleepTime; }
   void              SetSleepTime( int ms ) { m_sleepTime = ms; }

   void              Pause() { m_isPaused = true; }
   void              Resume();

   //------------------------------------------------------

protected:
   // allowing constness to be maintained with mutable
   mutable Mutex     m_mutex;
   void LockMutex() const { m_mutex.lock(); }
   void UnlockMutex() const { m_mutex.unlock(); }
   bool              m_isPaused;

protected:

   // ******** inherited classes must provide this function *********
   virtual int       CallbackFunction() = 0;

   // prevent people from destroying directly. Call Cleanup instead.
   virtual ~CAbstractThread();
   virtual void      CallbackOnCleanup() {}

private:

   bool              m_running;
   bool              m_markedForCleanup;
   bool              m_needsThreadProtection;
   int               m_sleepTime;

   ThreadId          m_thread;
   
   //---------------------------

   int               CreateThread();
   int               DestroyThread();

#if PLATFORM == PLATFORM_WINDOWS

   DWORD             m_threadId;
   static DWORD  WINAPI  ThreadFunction( void* data );

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

   pthread_attr_t    m_attr;
   static void*      ThreadFunction( void* data );

#endif
};

/////////////////////////////////////////////////////////////////////////////

}// namespace Threading