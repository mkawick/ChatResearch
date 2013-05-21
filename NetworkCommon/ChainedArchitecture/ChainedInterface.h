// ChainedInterface.h

#pragma once

#include "Thread.h"

/////////////////////////////////////////////////////////////////////////////

enum ThreadEventType
{
   ThreadEvent_None,
   ThreadEvent_NeedsService,
   ThreadEvent_DataAvailable
};

class ThreadEvent
{
public:
   ThreadEvent() : type(0), subType(0), meta(0) {}
   virtual ~ThreadEvent() { delete meta; }

   int      type;// custom
   int      subType;// custom
   int      identifier;
   void*    meta;
};

/////////////////////////////////////////////////////////////////////////////

   // this is a strange feature, but while rare, the importance of providing a clean
   // mechanism for allowing threads to chain and unchain correctly became obvious during test
template <typename Type> 
class ChainedInterface
{
public:
   ChainedInterface();

   U32            GetChainedId() const { return m_chainId; }
   U32				GetConnectionId() const { return m_connectionId; }
	void	         SetConnectionId( int connectionId ) { m_connectionId = connectionId; }

   void           AddInputChain( ChainedInterface*, bool recurse = true );
   void           RemoveInputChain( ChainedInterface*, bool recurse = true );
   void           AddOutputChain( ChainedInterface*, bool recurse = true );
   void           RemoveOutputChain( ChainedInterface*, bool recurse = true );

   // TODO: convert this to a const reference instead
   virtual bool   AddInputChainData( Type t, U32 filingData = -1 ) { return false; }// a false value means that the data was rejected
   virtual bool   AddOutputChainData( Type t, U32 filingData = -1 ) { return false; }// a false value means that the data was rejected

   virtual bool   PushInputEvent( ThreadEvent* ) { return false; }
   virtual bool   PushOutputEvent( ThreadEvent* ) { return false; }

   virtual bool   Log( const std::string& text, int priority = 1 );
   virtual bool   Log( const char* text, int priority = 1 );// not used too often

   //------------------------------------------------------
protected:

	struct ChainLink // a trick here is to read from the front and push to the back. No mutex?
	{
		ChainLink() : m_interface( NULL ) {}
		ChainLink( ChainedInterface* obj ) : m_interface( obj ) {}

		void	            AddData( Type t ) { m_data.push_back( t ); }
		Type	            RemoveData() { Type t = m_data.front(); m_data.pop_front(); return t; }
      bool              HasData() const { return m_data.size() > 0; }

		ChainedInterface*	m_interface;
		std::deque<Type>	m_data;
		//Mutex				m_mutex;
	};

   virtual void   NotifyFinishedAdding( ChainedInterface* obj = NULL ) {} 
   virtual void   NotifyFinishedRemoving( ChainedInterface* obj = NULL ) {} // when NULL, this means all nodes have been removed

protected:

   typedef std::list< ChainLink >	                        BaseOutputContainer;
   typedef typename BaseOutputContainer::iterator	         ChainLinkIteratorType;
   typedef std::back_insert_iterator< std::deque< int > >   inserter;

   typedef std::deque< ThreadEvent* >                       ThreadQueue;

   //----------------------------------------
   Threading::Mutex                 m_inputChainListMutex;
   Threading::Mutex                 m_outputChainListMutex;


   BaseOutputContainer	            m_listOfInputs;
   BaseOutputContainer	            m_listOfOutputs;
   ThreadQueue	                     m_eventsIn;
   ThreadQueue                      m_eventsOut;

   static U32                       m_chainIdCounter;
   U32                              m_chainId;
   U32                              m_connectionId;

protected:
   void           CleanupAllEvents();
   virtual void   ProcessEvents();

   void           CleanupAllChainDependencies();   
};

/////////////////////////////////////////////////////////////////////////////


template <typename Type> 
U32 ChainedInterface< Type >::m_chainIdCounter = 30;//0x7fffffff;

//----------------------------------------------------------------

template < typename Type > 
ChainedInterface< Type >::ChainedInterface() : m_chainId ( m_chainIdCounter++ ), m_connectionId( 0 )
{
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::AddInputChain( ChainedInterface<Type>* chainedInterfaceObject, bool recurse )
{
   bool found = false;
   m_inputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks
      
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
		   ChainedInterface* interfacePtr = chainedInput.m_interface;
         if( interfacePtr == chainedInterfaceObject )
         {
            found = true;
            break;
         }
      }
      if( found == false )
      {
         m_listOfInputs.push_back( ChainLink( chainedInterfaceObject ) );
      }

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_inputChainListMutex.unlock();

   // notice that we are outside of the locks now.
   if( found == false && recurse == true )
   {
      chainedInterfaceObject->AddOutputChain( this, false ); // note, output
   }
   NotifyFinishedAdding( chainedInterfaceObject );
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::RemoveInputChain( ChainedInterface<Type>* chainedInterfaceObject, bool recurse )
{
   bool found = false;
   m_inputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs;
		   ChainedInterface* interfacePtr = chainedInput.m_interface;
         if( interfacePtr == chainedInterfaceObject )
         {
            found = true;
            m_listOfInputs.erase( itInputs );
            break;
         }

         // increment here so that erase works
         itInputs++;
      }

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_inputChainListMutex.unlock();

   // notice that we are outside of the locks now.
   if( found == true && recurse == true )
   {
      chainedInterfaceObject->RemoveOutputChain( this, false ); // note, output
   }
   NotifyFinishedRemoving( chainedInterfaceObject );
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::AddOutputChain( ChainedInterface<Type>* chainedInterfaceObject, bool recurse )
{
   bool found = false;
   m_outputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         const ChainLink& chain = *itOutputs++;
         ChainedInterface* interfacePtr = chain.m_interface;
         if( interfacePtr == chainedInterfaceObject )
         {
            found = true;
            break;
         }
      }

      if( found == false )// insert into the list
      {
         m_listOfOutputs.push_back( ChainLink( chainedInterfaceObject ) );
      }

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_outputChainListMutex.unlock();

   // notice that we are outside of the locks now.
   if( found == false && recurse == true )
   {
      chainedInterfaceObject->AddInputChain( this, false ); // note, input
   }
   NotifyFinishedAdding( chainedInterfaceObject );
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::RemoveOutputChain( ChainedInterface<Type>* chainedInterfaceObject, bool recurse )
{
   bool found = false;
   m_outputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainLink& chainedOutput = *itOutputs;
		   ChainedInterface* interfacePtr = chainedOutput.m_interface;
         if( interfacePtr == chainedInterfaceObject )
         {
            found = true;
            m_listOfOutputs.erase( itOutputs );
            break;
         }

         // increment here so that erase works
         itOutputs++;
      }

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_outputChainListMutex.unlock();

   // notice that we are outside of the locks now.
   if( found == true && recurse == true )
   {
      chainedInterfaceObject->RemoveInputChain( this, false );// note, input
   }
   NotifyFinishedRemoving( chainedInterfaceObject );
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::CleanupAllChainDependencies()
{
   m_inputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
		   ChainedInterface* interfacePtr = chainedInput.m_interface;
         interfacePtr->RemoveOutputChain( this );// note the output
      }
      m_listOfInputs.clear();

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_inputChainListMutex.unlock();

   //-------------------------------

   m_outputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainLink& chainedOutput = *itOutputs++;
         ChainedInterface* interfacePtr = chainedOutput.m_interface;
         interfacePtr->RemoveInputChain( this );// note the input
      }
      m_listOfOutputs.clear();

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_outputChainListMutex.unlock();
   NotifyFinishedRemoving();
}

//----------------------------------------------------------------

template <typename Type> 
void     ChainedInterface<Type>::CleanupAllEvents()
{
   if( m_eventsIn.size() == 0 && m_eventsOut.size() == 0 )
      return;

   m_inputChainListMutex.lock();
   int num = m_eventsIn.size();
   while( num -- )
   {
      ThreadEvent* event = m_eventsIn.front();
      m_eventsIn.pop_front();
      delete event;
   }

   num = m_eventsOut.size();
   while( num -- )
   {
      ThreadEvent* event = m_eventsOut.front();
      m_eventsOut.pop_front();
      delete event;
   }
   m_inputChainListMutex.unlock();
}

template <typename Type> 
void     ChainedInterface<Type>::ProcessEvents()
{
   CleanupAllEvents();
}

// the default behavior is simply to pass along the log to the next in the chain
// however, this can lead to duplate log entries so be careful. Make sure to 
// return false if you do not plan to consume the log.
template <typename Type> 
bool     ChainedInterface<Type>::Log( const std::string& text, int priority )
{
   return Log( text.c_str(), priority );
}

template <typename Type> 
bool     ChainedInterface<Type>::Log( const char* text, int priority )
{
   bool didLog = false;
   m_outputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainLink& chainedOutput = *itOutputs++;
         ChainedInterface* interfacePtr = chainedOutput.m_interface;
         didLog = interfacePtr->Log( text, priority );
         if( didLog )// someone captured the log so stop sending it.
            break;
      }

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_outputChainListMutex.unlock();

   return didLog;
}

//////////////////////////////////////////////////////////////////////////////////////////////
