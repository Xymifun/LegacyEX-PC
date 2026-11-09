#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CUSTOMDATAHOLDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CUSTOMDATAHOLDER_HPP

#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class CustomDataHolder : public AnyObjectHolder<Data, String>
{
public:
    explicit CustomDataHolder(T* pObject = 0) : AnyObjectHolder<Data, String>(pObject) {}

    CustomDataHolder(const CustomDataHolder<T>& rOther) : AnyObjectHolder<Data, String>(rOther) {}

    virtual ~CustomDataHolder() {}

    T* GetObject() const { return static_cast<T*>(this->m_pObject); }
};

}
}

#endif
