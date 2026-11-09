#include <new>
#include <stdlib.h>

extern std::new_handler _new_handler;

void __default_new_handler(void);

void* operator new(size_t size)
{
    void* ptr;

    if (size == 0)
    {
        size = 1;
    }
    while ((ptr = malloc(size)) == NULL)
    {
        std::new_handler handler = __default_new_handler;
        if (_new_handler != NULL)
        {
            handler = _new_handler;
        }
        (*handler)();
        if (handler == __default_new_handler)
        {
            return NULL;
        }
    }
    return ptr;
}
