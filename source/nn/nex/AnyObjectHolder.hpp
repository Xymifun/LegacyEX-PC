#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ANYOBJECTHOLDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ANYOBJECTHOLDER_HPP

#include "nn/nex/Holder.hpp"

namespace nn
{
namespace nex
{

template <typename T, typename KeyType>
class AnyObjectHolder : public Holder<T>
{
public:
    explicit AnyObjectHolder(T* pObject = 0) : Holder<T>(pObject) {}

    AnyObjectHolder(const AnyObjectHolder<T, KeyType>& rOther) : Holder<T>(rOther) {}

    virtual ~AnyObjectHolder() {}
};

}
}

#endif
