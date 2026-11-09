#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SIMPLE_CONTAINER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SIMPLE_CONTAINER_HPP

#include "nn/pia/common/InetAddress.hpp"

namespace nn
{
namespace pia
{
namespace common
{

template <class T, int N>
struct SimpleContainer
{
    typedef T* iterator;

    SimpleContainer() : size(0), data() {}

    virtual ~SimpleContainer() {}

    virtual void Trace(unsigned long long) const {}

    iterator begin() { return data; }

    iterator end() { return &data[size]; }

    unsigned int size;
    T data[N];
};

}
}
}

#endif
