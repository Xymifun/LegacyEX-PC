#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QMULTIMAP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QMULTIMAP_HPP

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
class qMultiMap : public std::multimap<K, V, std::less<K>, MemAllocator<std::pair<const K, V> > >, public RootObject
{
public:
    typedef std::multimap<K, V, std::less<K>, MemAllocator<std::pair<const K, V> > > _Base;
    typedef typename _Base::size_type size_type;
    typedef typename _Base::key_type key_type;
    typedef typename _Base::mapped_type mapped_type;
    typedef typename _Base::value_type value_type;
    typedef typename _Base::key_compare key_compare;

    qMultiMap() : _Base() {}

    explicit qMultiMap(const key_compare& comparator) : _Base(comparator) {}

    qMultiMap(const qMultiMap<K, V>& other) : _Base(other) {}

    qMultiMap(const _Base& other) : _Base(other) {}

    ~qMultiMap() {}

    qMultiMap<K, V>& operator=(const qMultiMap<K, V>& other)
    {
        _Base::operator=(other);
        return *this;
    }

    qMultiMap<K, V>& operator=(const _Base& other)
    {
        _Base::operator=(other);
        return *this;
    }
};

}
}

#endif
