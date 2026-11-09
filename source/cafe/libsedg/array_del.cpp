#include <new>

void operator delete[](void* ptr)
{
    operator delete(ptr);
}
