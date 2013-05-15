// Allocator.h


namespace Allocate
{
   void* operator new(size_t);
   void operator delete(void*, size_t);
   void operator delete[](void *ptr);
};