#include <new>

void* operator new[](size_t size)
{
    return operator new(size);
}
