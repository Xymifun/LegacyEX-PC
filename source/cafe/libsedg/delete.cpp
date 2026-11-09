#include <stdlib.h>

void operator delete(void* ptr)
{
    if (ptr != NULL)
    {
        free(ptr);
    }
}
