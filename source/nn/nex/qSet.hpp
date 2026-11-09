#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QSET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QSET_HPP

#include <functional>
#include <set>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class qSet : public std::set<T, std::less<T>, MemAllocator<T> >, public RootObject
{
public:
    typedef std::set<T, std::less<T>, MemAllocator<T> > _Base;

    qSet() : _Base() {}

    qSet(const qSet<T>& other) : _Base(other) {}

    ~qSet() {}

    qSet<T>& operator=(const qSet<T>& other)
    {
        _Base::operator=(other);
        return *this;
    }
};

}
}

#endif
