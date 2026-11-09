#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HOLDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HOLDER_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class Holder : public RootObject
{
public:
    explicit Holder(T* pObject = 0) : m_pObject(pObject) {}

    Holder(const Holder<T>& rOther) : m_pObject(rOther.m_pObject) { rOther.m_pObject = 0; }

    ~Holder() { delete m_pObject; }

    Holder<T>& operator=(T* pObject)
    {
        delete m_pObject;
        m_pObject = pObject;
        return *this;
    }

    Holder<T>& operator=(const Holder<T>& rOther)
    {
        T* pObject = rOther.m_pObject;
        rOther.m_pObject = 0;
        delete m_pObject;
        m_pObject = pObject;
        return *this;
    }

    T* operator->() const { return m_pObject; }

    T* GetObject() const { return m_pObject; }

    T* Release()
    {
        T* pObject = m_pObject;
        m_pObject = 0;
        return pObject;
    }

    void SetObject(T* pObject)
    {
        delete m_pObject;
        m_pObject = pObject;
    }

protected:
    mutable T* m_pObject;
};

}
}

#endif
