#ifndef SOURCE_NN_PIA_COMMON_MEMORY_HPP
#define SOURCE_NN_PIA_COMMON_MEMORY_HPP

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace common
{

template <typename T> static T* NewObj()
{
    void* buffer = pead::allocateFromSeadHeap(sizeof(T), HeapManager::GetHeap());
    if (buffer == 0)
        return 0;
    return ::new (buffer) T();
}

template <typename T, typename A1, typename A2> static T* NewObj(A1 a1, A2 a2)
{
    void* buffer = pead::allocateFromSeadHeap(sizeof(T), HeapManager::GetHeap());
    if (buffer == 0)
        return 0;
    return ::new (buffer) T(a1, a2);
}

template <typename T> static void DeleteObj(T* object)
{
    if (object != 0)
    {
        object->~T();
        pead::freeToSeadHeap(object);
    }
}

template <typename T> static T* NewArray(unsigned int count)
{
    void* buffer = pead::allocateFromSeadHeap(sizeof(T) * count, HeapManager::GetHeap());
    if (buffer == 0)
        return 0;
    T* array = static_cast<T*>(buffer);
    for (unsigned int index = 0; index < count; ++index)
        ::new (&array[index]) T();
    return array;
}

template <typename T> static T* NewArray(unsigned int count, int alignment)
{
    void* buffer = pead::allocateFromSeadHeap(sizeof(T) * count, HeapManager::GetHeap(), alignment);
    if (buffer == 0)
        return 0;
    T* array = static_cast<T*>(buffer);
    for (unsigned int index = 0; index < count; ++index)
        ::new (&array[index]) T();
    return array;
}

template <typename T> static void DeleteArray(T* array)
{
    if (array != 0)
    {
        unsigned int size = pead::getArraySize(array);
        unsigned int count = size / sizeof(T);
        for (unsigned int index = 0; index < count; ++index)
            array[index].~T();
        pead::freeToSeadHeap(array);
    }
}

}
}
}

#endif
