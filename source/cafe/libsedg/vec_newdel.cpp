#include <new>

#include <cafe/libsedg/edg_runtime.h>

typedef long an_element_count_param;
typedef size_t a_sizeof_t;
typedef void (*a_destructor_ptr)(void*, int);
typedef void* (*a_new_ptr)(size_t);
typedef void (*a_delete_ptr)(void*);
typedef void (*a_two_operand_delete_ptr)(void*, size_t);
typedef void (*a_cfront_constructor_ptr)(void*, void*, void*, void*, void*, void*, void*, void*, void*);
typedef void* a_ctor_return_type;
typedef a_ctor_return_type (*a_constructor_ptr)(void*);
typedef a_ctor_return_type (*a_copy_constructor_ptr)(void*, void*);

typedef struct an_alloc_prefix
{
    size_t size;
    size_t inverted_number_of_elements;
} an_alloc_prefix, *an_alloc_prefix_ptr;

extern "C" void __memzero(void* ptr, size_t size);

a_sizeof_t __array_new_prefix_size = sizeof(an_alloc_prefix);

extern "C" void _array_pointer_not_from_vec_new(void)
{
    __abort_execution(ec_array_not_from_vec_new);
}

static inline void* alloc_array(size_t size, size_t prefix_size, a_new_ptr new_routine)
{
    void* ptr;
    size_t full_size = size + prefix_size;

    if (new_routine == NULL)
    {
        ptr = operator new[](full_size);
    }
    else
    {
        ptr = (*new_routine)(full_size);
    }
    if (ptr != NULL)
    {
        ptr = (char*)ptr + prefix_size;
    }
    return ptr;
}

static inline void free_array(void* array_ptr, size_t size, size_t prefix_size, a_delete_ptr delete_routine,
                              int is_two_arg)
{
    void* ptr = (char*)array_ptr - prefix_size;
    size_t full_size = size + prefix_size;

    if (delete_routine == NULL)
    {
        operator delete[](ptr);
    }
    else if (is_two_arg)
    {
        (*(a_two_operand_delete_ptr)delete_routine)(ptr, full_size);
    }
    else
    {
        (*delete_routine)(ptr);
    }
}

static inline int record_array_alloc_info(void* array_ptr, size_t size, int number_of_elements, size_t prefix_size)
{
    an_alloc_prefix_ptr prefix = (an_alloc_prefix_ptr)((char*)array_ptr - __array_new_prefix_size);

    prefix->size = size;
    prefix->inverted_number_of_elements = ~number_of_elements;
    return TRUE;
}

static inline size_t get_array_size(void* array_ptr, size_t element_size, size_t* number_of_elements)
{
    an_alloc_prefix_ptr prefix = (an_alloc_prefix_ptr)((char*)array_ptr - __array_new_prefix_size);
    size_t size = prefix->size;
    size_t expected_size;

    *number_of_elements = ~prefix->inverted_number_of_elements;
    expected_size = element_size * *number_of_elements;
    if (expected_size == 0)
    {
        expected_size = 1;
    }
    if (size != expected_size)
    {
        _array_pointer_not_from_vec_new();
    }
    return size;
}

static void* array_new_general(void* array_ptr, int number_of_elements, size_t element_size, size_t prefix_size,
                               void* src_array_ptr, a_constructor_ptr ctor, a_destructor_ptr dtor,
                               a_new_ptr new_routine, a_delete_ptr delete_routine, int is_two_arg, int zero_init)
{
    size_t size;
    int i;
    char* ptr;

    if (array_ptr == NULL || prefix_size != 0)
    {
        size = number_of_elements * element_size;
        if (size == 0)
        {
            size = 1;
        }
        if (array_ptr == NULL)
        {
            array_ptr = alloc_array(size, prefix_size, new_routine);
            if (array_ptr == NULL)
            {
                return array_ptr;
            }
        }
        if (prefix_size != 0)
        {
            record_array_alloc_info(array_ptr, size, number_of_elements, prefix_size);
        }
        if (zero_init)
        {
            __memzero(array_ptr, size);
        }
    }
    else if (zero_init)
    {
        size = number_of_elements * element_size;
        __memzero(array_ptr, size);
    }
    if (ctor != NULL)
    {
        for (i = 0, ptr = (char*)array_ptr; i < number_of_elements; i++, ptr += element_size)
        {
            if (src_array_ptr == NULL)
            {
                (*(a_cfront_constructor_ptr)ctor)(ptr, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
            }
            else
            {
                (*(a_copy_constructor_ptr)ctor)(ptr, src_array_ptr);
                src_array_ptr = (char*)src_array_ptr + element_size;
            }
        }
    }
    return array_ptr;
}

extern "C" void* __array_new(an_element_count_param number_of_elements, size_t element_size, a_constructor_ptr ctor,
                             a_destructor_ptr dtor, a_new_ptr new_routine, a_delete_ptr delete_routine, int is_two_arg)
{
    return array_new_general(NULL, number_of_elements, element_size, __array_new_prefix_size, NULL, ctor, dtor,
                             new_routine, delete_routine, is_two_arg, FALSE);
}

extern "C" void* __placement_array_new(void* array_ptr, an_element_count_param number_of_elements, size_t element_size,
                                       a_constructor_ptr ctor, a_destructor_ptr dtor)
{
    return array_new_general(array_ptr, number_of_elements, element_size, __array_new_prefix_size, NULL, ctor, dtor,
                             NULL, NULL, FALSE, FALSE);
}

extern "C" void* __vec_new_eh(void* array_ptr, an_element_count_param number_of_elements, size_t element_size,
                              a_constructor_ptr ctor, a_destructor_ptr dtor)
{
    return array_new_general(array_ptr, number_of_elements, element_size,
                             array_ptr == NULL ? __array_new_prefix_size : 0, NULL, ctor, dtor, NULL, NULL, FALSE,
                             FALSE);
}

extern "C" void* __vec_new(void* array_ptr, an_element_count_param number_of_elements, size_t element_size,
                           a_constructor_ptr ctor)
{
    return array_new_general(array_ptr, number_of_elements, element_size,
                             array_ptr == NULL ? __array_new_prefix_size : 0, NULL, ctor, NULL, NULL, NULL, FALSE,
                             FALSE);
}

extern "C" void __vec_cctor_eh(void* array_ptr, size_t number_of_elements, size_t element_size,
                               a_copy_constructor_ptr ctor, void* src_array_ptr, a_destructor_ptr dtor)
{
    array_new_general(array_ptr, number_of_elements, element_size, array_ptr == NULL ? __array_new_prefix_size : 0,
                      src_array_ptr, (a_constructor_ptr)ctor, dtor, NULL, NULL, FALSE, FALSE);
}

extern "C" void* __array_new_zero(an_element_count_param number_of_elements, size_t element_size,
                                  a_constructor_ptr ctor, a_destructor_ptr dtor, a_new_ptr new_routine,
                                  a_delete_ptr delete_routine, int is_two_arg)
{
    return array_new_general(NULL, number_of_elements, element_size, __array_new_prefix_size, NULL, ctor, dtor,
                             new_routine, delete_routine, is_two_arg, TRUE);
}

extern "C" void* __placement_array_new_zero(void* array_ptr, an_element_count_param number_of_elements,
                                            size_t element_size, a_constructor_ptr ctor, a_destructor_ptr dtor)
{
    return array_new_general(array_ptr, number_of_elements, element_size, __array_new_prefix_size, NULL, ctor, dtor,
                             NULL, NULL, FALSE, TRUE);
}

extern "C" void* __vec_new_eh_zero(void* array_ptr, an_element_count_param number_of_elements, size_t element_size,
                                   a_constructor_ptr ctor, a_destructor_ptr dtor)
{
    return array_new_general(array_ptr, number_of_elements, element_size,
                             array_ptr == NULL ? __array_new_prefix_size : 0, NULL, ctor, dtor, NULL, NULL, FALSE,
                             TRUE);
}

static void array_delete_general(void* array_ptr, int number_of_elements_param, size_t element_size, size_t prefix_size,
                                 a_destructor_ptr dtor, int delete_flag, a_delete_ptr delete_routine, int is_two_arg,
                                 int terminate_immediately)
{
    size_t size = 0;
    size_t number_of_elements;
    char* ptr;
    size_t i;

    if (array_ptr != NULL)
    {
        number_of_elements = number_of_elements_param;
        if (number_of_elements_param == -1 && prefix_size != 0)
        {
            size = get_array_size(array_ptr, element_size, &number_of_elements);
        }
        if (dtor != NULL)
        {
            ptr = (char*)array_ptr + (number_of_elements - 1) * element_size;
            for (i = 0; i < number_of_elements; i++, ptr -= element_size)
            {
                (*dtor)(ptr, 2);
            }
        }
        if (delete_flag)
        {
            free_array(array_ptr, size, prefix_size, delete_routine, is_two_arg);
        }
    }
}

extern "C" void __vec_delete(void* array_ptr, an_element_count_param number_of_elements, size_t element_size,
                             a_destructor_ptr dtor, int delete_flag, int unused)
{
    array_delete_general(array_ptr, number_of_elements, element_size,
                         number_of_elements == -1 ? __array_new_prefix_size : 0, dtor, delete_flag, NULL, FALSE, FALSE);
}

extern "C" void __array_delete(void* array_ptr, an_element_count_param number_of_elements, size_t element_size,
                               a_destructor_ptr dtor, a_delete_ptr delete_routine, int is_two_arg)
{
    array_delete_general(array_ptr, number_of_elements, element_size,
                         number_of_elements == -1 ? __array_new_prefix_size : 0, dtor, TRUE, delete_routine, is_two_arg,
                         FALSE);
}
