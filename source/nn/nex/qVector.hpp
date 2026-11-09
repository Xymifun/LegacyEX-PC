#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QVECTOR_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QVECTOR_HPP

#include <cstddef>
#include <vector>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class qVector : public std::vector<T, MemAllocator<T> >, public RootObject
{
public:
    typedef std::vector<T, MemAllocator<T> > _Base;
    typedef typename _Base::size_type size_type;
    typedef typename _Base::value_type value_type;

    qVector() : _Base() {}

    explicit qVector(size_type count) : _Base(count, value_type()) {}

    qVector(size_type count, const value_type& value) : _Base(count, value) {}

    qVector(const _Base& other) : _Base(other) {}

    qVector<T>& operator=(const _Base& other)
    {
        _Base::operator=(other);
        return *this;
    }
};

}
}

#endif
