#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SPECIALARRAY_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SPECIALARRAY_HPP

#include <new>

#include "nn/nex/MemoryManager.hpp"

namespace nn
{
namespace nex
{

template <class T>
T* SpecialNew(wchar_t*, unsigned int)
{
    T* object;

    object = static_cast<T*>(MemoryManager::Allocate(sizeof(T)));
    new (static_cast<void*>(object)) T;

    return object;
}

template <class T>
void SpecialDelete(T* object)
{
    if (object != 0)
        object->~T();

    MemoryManager::Free(object);
}

template <class T>
T* SpecialNewArray(unsigned int count, wchar_t*, unsigned int)
{
    unsigned int* block;
    T* elements;
    unsigned int index;

    block = static_cast<unsigned int*>(MemoryManager::Allocate(count * sizeof(T) + sizeof(unsigned int)));
    elements = reinterpret_cast<T*>(block + 1);
    *block = count;

    for (index = 0; index < count; ++index)
        new (static_cast<void*>(elements + index)) T;

    return elements;
}

template <class T>
void SpecialDeleteArray(T* elements)
{
    unsigned int* block;
    unsigned int index;

    block = (unsigned int*)elements - 1;

    for (index = 0; index < *block; ++index)
        elements[index].~T();

    MemoryManager::Free(block);
}

}
}

#endif
