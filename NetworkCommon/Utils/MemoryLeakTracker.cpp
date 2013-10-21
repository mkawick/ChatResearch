
#ifdef _TRACK_MEMORY_LEAK_

#include <windows.h>
#include <stdio.h>
#include <wtypes.h>

/// based on http://blogs.msdn.com/b/calvin_hsia/archive/2009/01/19/9341632.aspx

void OutputDebugStringf(char *szFmt, ...)
{
    va_list marker;
    va_start(marker, szFmt);  // the varargs start at szFmt
    char szBuf[1024];
    _vsnprintf_s(szBuf, sizeof(szBuf),szFmt, marker);
    OutputDebugStringA("\n");
    OutputDebugStringA(szBuf);
}
 
struct AllocHeader  // we'll put a header at the beginning of each alloc
{
    char szFile[MAX_PATH];
    UINT nLineNo;
    UINT nSize;
};
 
void * MyAlloc(size_t cbSize, char *szFile = __FILE__, UINT nLineNo = __LINE__)
{
    // We allocate a header followed by the desired allocation
    void *p = malloc(sizeof(AllocHeader) + cbSize );
    AllocHeader *pHeader = (AllocHeader *)p;
    strcpy_s(pHeader->szFile, szFile);
    pHeader->nLineNo = nLineNo;
    pHeader->nSize = cbSize;

    if( cbSize > 100 )
    {
       OutputDebugStringf(">100 ");
    }
    if( cbSize == 2128 )
    {
       OutputDebugStringf(">2128 ");
    }
    OutputDebugStringf("new %x : %d ", p, cbSize );//, szFile, nLineNo);
    // we return the address + sizeof(AllocHeader)
    return (void *)( (size_t)p+sizeof(AllocHeader));
}

void MyDelete(void *p)
{
    // we need to free our allocator too
    AllocHeader *pHeader = (AllocHeader *)((size_t)p - sizeof(AllocHeader));
    OutputDebugStringf("del %x : %d", p, pHeader->nSize );//pHeader->szFile, pHeader->nLineNo);
    free((void *)((size_t)p - sizeof(AllocHeader)));
}
 
 
// the single override of the module's new operator:
void * _cdecl operator new (size_t cbSize)
//void * _cdecl operator new (size_t cbSize, char *szFile = __FILE__, UINT nLineNo = __LINE__)
{
//#define USEWORKAROUND 1 // uncomment this to get the caller addr
#if USEWORKAROUND
    UINT *EbpRegister ; // the Base Pointer: the stack frame base
    _asm { mov EbpRegister, ebp};
    UINT CallerAddr = *(((size_t *)EbpRegister)+1) ; // the return addr
// if you get a leak, you'll get something like:
//          d:\dev\vc\overnew\overnew.cpp(10189270)
// Break into the debugger. Take the # in parens, put it in Watch window: turn on hex display->it shows addr of caller
// Go to disassembly, put the address in the Address bar hit enter. Bingo: you're at the caller that didn't free!
 
    //    CallerAddr -=  (size_t)g_hinstDll;    // you can get a relative address if you like: look at the link map
    void *p = MyAlloc(cbSize, __FILE__, CallerAddr); 
    OutputDebugStringf("Op new %x CallerAddr = %x", p, CallerAddr);
#else
    void *p = MyAlloc(cbSize, __FILE__, __LINE__);  // this line will show for all New operator calls<sigh>
    //OutputDebugStringf("Op new %x", p);
#endif
 
    return p;
}
 
// an overload of the new operator, with an int param
void * _cdecl operator new (size_t cbSize, int nAnyIntParam, char *szFile = __FILE__, UINT nLineNo = __LINE__)
{
    void *p = MyAlloc( cbSize, szFile, nLineNo );  // this line will show for all New operator calls<sigh>
    //OutputDebugStringf( "Op new %x %d", p, cbSize );
    return p;
}
 
 
void  _cdecl operator delete(void *p)
{
    //OutputDebugStringf("Op del %x", p);
    MyDelete(p);
}


#endif // _TRACK_MEMORY_LEAK_


/*class MemoryTracker
{
};
/////////////////////////////////////////////////////////////////////////

MemoryTracker* MemoryTracker::singleton = NULL;

//---------------------------------------

MemoryTracker& MemoryTracker::Instance()
{
   if( singleton == NULL )
   {
      void* ptr = malloc( sizeof( MemoryTracker ) );
      singleton = new (ptr) MemoryTracker();
      singleton->init();
   }

   return *singleton;
}

/////////////////////////////////////////////////////////////////////////

void* operator new (size_t size)
{
   void* pointer = AllocatorSingleton::Instance().allocate( static_cast< int >( size ) );
   if( pointer == NULL ) // did malloc succeed?
      throw std::bad_alloc(); // ANSI/ISO compliant behavior

   return pointer;
}


void operator delete (void* pointer)
{
   AllocatorSingleton::Instance().deallocate( pointer );
}*/