#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QMAP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QMAP_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename K, typename V>
class qMap : public std::map<K, V, std::less<K>, MemAllocator<std::pair<const K, V> > >, public RootObject
{
public:
    typedef std::map<K, V, std::less<K>, MemAllocator<std::pair<const K, V> > > _Base;

    ~qMap() {}

    qMap<K, V>& operator=(const qMap<K, V>& other)
    {
        _Base::operator=(other);
        return *this;
    }

    qMap<K, V>& operator=(const _Base& other)
    {
        _Base::operator=(other);
        return *this;
    }
};

}
}

#endif
