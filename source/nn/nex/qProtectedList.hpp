#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QPROTECTEDLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QPROTECTEDLIST_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class qProtectedList : public RootObject
{
public:
    qProtectedList() : m_criticalSection(1) {}

    virtual ~qProtectedList() {}

    void push_back(const T& rValue)
    {
        ScopedCS lock(&m_criticalSection);
        m_list.push_back(rValue);
    }

    CriticalSection& GetCriticalSection() const { return m_criticalSection; }

    qList<T>& GetList() { return m_list; }

    const qList<T>& GetList() const { return m_list; }

private:
    unsigned int m_unknown0;
    mutable CriticalSection m_criticalSection;
    qList<T> m_list;
};

}
}

#endif
