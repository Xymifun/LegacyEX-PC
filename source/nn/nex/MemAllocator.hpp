#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MEMALLOCATOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MEMALLOCATOR_HPP

#include <cstddef>
#include <new>

#include "nn/nex/MemoryManager.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class MemAllocator
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;

    template <typename U>
    struct rebind
    {
        typedef MemAllocator<U> other;
    };

    MemAllocator() {}
    MemAllocator(const MemAllocator<T>&) {}

    template <typename U>
    MemAllocator(const MemAllocator<U>&)
    {
    }

    ~MemAllocator() {}

    template <typename U>
    MemAllocator<T>& operator=(const MemAllocator<U>&)
    {
        return *this;
    }

    pointer address(reference value) const { return &value; }

    const_pointer address(const_reference value) const { return &value; }

    pointer allocate(size_type count)
    {
        return static_cast<pointer>(MemoryManager::Allocate(static_cast<unsigned int>(count * sizeof(T))));
    }

    pointer allocate(size_type count, const void*) { return allocate(count); }

    void deallocate(pointer block, size_type) { MemoryManager::Free(block); }

    void construct(pointer position, const_reference value) { ::new (static_cast<void*>(position)) T(value); }

    void destroy(pointer position) { position->~T(); }

    template <typename U>
    bool operator==(const MemAllocator<U>&) const
    {
        return true;
    }

    template <typename U>
    bool operator!=(const MemAllocator<U>&) const
    {
        return false;
    }

    size_type max_size() const { return static_cast<size_type>(-1) / sizeof(T); }
};

}
}

#endif
