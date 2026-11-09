#include <stddef.h>

typedef void* a_ctor_return_type;
typedef a_ctor_return_type (*a_copy_constructor_ptr)(void*, void*);

extern "C" void __vec_cctor(void* array_ptr, size_t number_of_elements, size_t element_size,
                            a_copy_constructor_ptr ctor, void* src_array_ptr)
{
    if (ctor != NULL)
    {
        size_t i = 0;
        char* ptr = (char*)array_ptr;
        char* src_ptr = (char*)src_array_ptr;
        for (; i < number_of_elements; i++, ptr += element_size, src_ptr += element_size)
        {
            (*ctor)(ptr, src_ptr);
        }
    }
}
