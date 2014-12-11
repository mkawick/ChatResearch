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
   U8*      meta;
};

/////////////////////////////////////////////////////////////////////////////

class IChainedInterface
{
public:
   virtual void   InputConnected( IChainedInterface * ) {}
   virtual void   OutputConnected( IChainedInterface * ) {}
   virtual void   InputRemovalInProgress( IChainedInterface * ) {}
   virtual void   OutputRemovalInProgress( IChainedInterface * ) {}
   virtual void   NotifyFinishedAdding( IChainedInterface* obj ) {} 
   virtual void   NotifyFinishedRemoving( IChainedInterface* obj ) {} // when NULL, this means all nodes have been removed
};

/////////////////////////////////////////////////////////////////////////////

   // this is a strange feature, but while rare, the importance of providing a clean
   // mechanism for allowing threads to chain and unchain correctly became obvious during test
template<typename Type>
class ChainedInterface : public IChainedInterface
{
protected:
   typedef ChainedInterface<Type> ChainType;
public:
   enum ChainedType
   {
      ChainedType_Default,
      ChainedType_InboundSocketConnector, // khaan
      ChainedType_OutboundSocketConnector,// fruitadens
      ChainedType_MainThreadContainer,    // normal diplodocus
      ChainedType_DatabaseConnector,      // deltadromeus
      ChainedType_AlternateThreadContainer,// Diplodocus Server to Server
      ChainedType_Other
   };
public:
   ChainedInterface();

   virtual const char*    GetClassName() const { return "ChainedInterface"; }
   bool           DoesNameMatch( const char* name ) const { return strcmp( GetClassName(), name ) == 0; }
   ChainedType    GetChainedType() const { return m_chainedType; }

   U32            GetChainedId() const { return m_chainId; }
   U32				GetConnectionId() const { return m_connectionId; }
	void	         SetConnectionId( int connectionId ) { m_connectionId = connectionId; }

   void           AddInputChain( IChainedInterface*, bool recurse = true );
   void           RemoveInputChain( IChainedInterface*, bool recurse = true );
   void           AddOutputChain( IChainedInterface*, bool recurse = true );
   void           RemoveOutputChain( IChainedInterface*, bool recurse = true );


   virtual bool   AddInputChainData( Type t, U32 filingData = -1 ) { return false; }// a false value means that the data was rejected
   virtual bool   AddOutputChainData( Type t, U32 filingData = -1 ) { return false; }// a false value means that the data was rejected
   virtual bool   AddOutputChainDataNoLock( Type t ) { return false; }

   virtual bool   PushInputEvent( ThreadEvent* ) { return false; }
   virtual bool   PushOutputEvent( ThreadEvent* ) { return false; }

   virtual bool   Log( const std::string& text, int priority = 1 ) const;
   virtual bool   Log( const char* text, int priority = 1 ) const;// not used too often

   //------------------------------------------------------
protected:

	struct ChainLink // a trick here is to read from the front and push to the back. No mutex?
	{
		ChainLink() : m_interface( NULL ) {}
		ChainLink( ChainedInterface* obj ) : m_interface( obj ) {}

		void	            AddData( Type t ) { m_data.push_back( t ); }
		Type	            RemoveData() { Type t = m_data.front(); m_data.pop_front(); return t; }
      bool              HasData() const { return m_data.size() > 0; }

		IChainedInterface*	   m_interface;
		std::deque<Type>	         m_data;
	};


protected:

   typedef std::list< ChainLink >	                        BaseOutputContainer;
   typedef typename BaseOutputContainer::iterator	         ChainLinkIteratorType;
   typedef typename BaseOutputContainer::const_iterator	   ChainLinkConstIterator;
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
   ChainedType                      m_chainedType;

protected:
   void           CleanupAllEvents();
   virtual void   ProcessEvents();

   void           CleanupAllChainDependencies();   
   ChainLinkIteratorType   FindInputConnection( U32 connectionId );
   ChainLinkIteratorType   FindOutputConnection( U32 connectionId );
};

/////////////////////////////////////////////////////////////////////////////


template <typename Type> 
U32 ChainedInterface< Type >::m_chainIdCounter = 30;//0x7fffffff;

//----------------------------------------------------------------

template < typename Type > 
ChainedInterface< Type >::ChainedInterface() : m_chainId ( m_chainIdCounter++ ), 
                                                m_connectionId( 0 ), 
                                                m_chainedType( ChainedType_Default )
{
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::AddInputChain( IChainedInterface* chainedInterfaceObject, bool recurse )
{
   bool found = false;
   m_inputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks
      
      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs++;
		   IChainedInterface* interfacePtr = chainedInput.m_interface;
         if( interfacePtr == chainedInterfaceObject )
         {
            found = true;
            break;
         }
      }
      if( found == false )
      {
         m_listOfInputs.push_back( ChainLink( static_cast< ChainType*> ( chainedInterfaceObject ) ) );
      }

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_inputChainListMutex.unlock();

   // notice that we are outside of the locks now.
   if( found == false && recurse == true )
   {
      static_cast< ChainType*> ( chainedInterfaceObject )->AddOutputChain( this, false ); // note, output
   }
   NotifyFinishedAdding( chainedInterfaceObject );
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::RemoveInputChain( IChainedInterface* chainedInterfaceObject, bool recurse )
{
   bool found = false;
   m_inputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itInputs = m_listOfInputs.begin();
      while( itInputs != m_listOfInputs.end() )
      {
         ChainLink& chainedInput = *itInputs;
		   IChainedInterface* interfacePtr = chainedInput.m_interface;
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
      static_cast<ChainType*>( chainedInterfaceObject )->RemoveOutputChain( this, false ); // note, output
   }
   NotifyFinishedRemoving( chainedInterfaceObject );
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::AddOutputChain( IChainedInterface* chainedInterfaceObject, bool recurse )
{
   bool found = false;
   m_outputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         const ChainLink& chain = *itOutputs++;
         IChainedInterface* interfacePtr = chain.m_interface;
         if( interfacePtr == chainedInterfaceObject )
         {
            found = true;
            break;
         }
      }

      if( found == false )// insert into the list
      {
         m_listOfOutputs.push_back( ChainLink( static_cast< ChainType*> ( chainedInterfaceObject ) ) );
      }

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_outputChainListMutex.unlock();

   // notice that we are outside of the locks now.
   if( found == false && recurse == true )
   {
      static_cast<ChainType*>( chainedInterfaceObject )->AddInputChain( this, false ); // note, input
   }
   NotifyFinishedAdding( chainedInterfaceObject );
}

//----------------------------------------------------------------

template <typename Type> 
void  ChainedInterface<Type>::RemoveOutputChain( IChainedInterface* chainedInterfaceObject, bool recurse )
{
   bool found = false;
   m_outputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         ChainLink& chainedOutput = *itOutputs;
		   IChainedInterface* interfacePtr = chainedOutput.m_interface;
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
      static_cast<ChainType*>( chainedInterfaceObject )->RemoveInputChain( this, false );// note, input
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
		   IChainedInterface* interfacePtr = chainedInput.m_interface;
         ChainType* chainPtr = static_cast< ChainType* >( interfacePtr );
         chainPtr->RemoveOutputChain( this );// note the output
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
         IChainedInterface* interfacePtr = chainedOutput.m_interface;
         ChainType* chainPtr = static_cast< ChainType* >( interfacePtr );
         chainPtr->RemoveInputChain( this );// note the input
      }
      m_listOfOutputs.clear();

      // the indentation here is to show that we are in the 'scope' of the locks
   }
   m_outputChainListMutex.unlock();
   //NotifyFinishedRemoving();
}

//---------------------------------------------------------------

template <typename Type> 
typename ChainedInterface< Type >::ChainLinkIteratorType   // note the typename hack to make the template work.
ChainedInterface< Type >::FindInputConnection( U32 connectionId )
{
   ChainLinkIteratorType itInputs = m_listOfInputs.begin();
   while( itInputs != m_listOfInputs.end() )
   {
      ChainType* inputPtr = static_cast< ChainType*> ( itInputs->m_interface );
      if( inputPtr->GetConnectionId() == connectionId )
      {
         return itInputs;
      }
      itInputs++;
   }
   return itInputs;
}


//---------------------------------------------------------------

template <typename Type> 
typename ChainedInterface< Type >::ChainLinkIteratorType   // note the typename hack to make the template work.
ChainedInterface< Type >::FindOutputConnection( U32 connectionId )
{
   ChainLinkIteratorType itOutputs = m_listOfOutputs.begin();
   while( itOutputs != m_listOfOutputs.end() )
   {
      ChainType* outputPtr = static_cast< ChainType*> ( itOutputs->m_interface );
      if( outputPtr->GetConnectionId() == connectionId )
      {
         return itOutputs;
      }
      itOutputs++;
   }
   return itOutputs;
}


//----------------------------------------------------------------

template <typename Type> 
void     ChainedInterface<Type>::CleanupAllEvents()
{
   if( m_eventsIn.size() == 0 && m_eventsOut.size() == 0 )
      return;

   m_inputChainListMutex.lock();
   while( m_eventsIn.size() > 0 )
   {
      ThreadEvent* event = m_eventsIn.front();
      m_eventsIn.pop_front();
      delete event;
   }

   while( m_eventsOut.size() > 0 )
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
bool     ChainedInterface<Type>::Log( const std::string& text, int priority ) const
{
   return Log( text.c_str(), priority );
}

template <typename Type> 
bool     ChainedInterface<Type>::Log( const char* text, int priority ) const
{
   bool didLog = false;
   m_outputChainListMutex.lock();
   {
      // the indentation here is to show that we are in the 'scope' of the locks

      ChainLinkConstIterator itOutputs = m_listOfOutputs.begin();
      while( itOutputs != m_listOfOutputs.end() )
      {
         const ChainLink& chainedOutput = *itOutputs++;
         const ChainedInterface<Type>* interfacePtr = static_cast<const ChainedInterface<Type>*>( chainedOutput.m_interface );
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
