#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_FIRSTERRORHOLDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_FIRSTERRORHOLDER_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qResult.hpp"

namespace nn
{
namespace nex
{

class FirstErrorHolder : public RootObject
{
public:
    qResult GetFirstError()
    {
        ScopedCS lock(&m_criticalSection);
        return m_firstError;
    }

    void SetFirstError(const qResult& rError)
    {
        ScopedCS lock(&m_criticalSection);
        if (m_firstError == true)
            m_firstError = rError;
    }

private:
    unsigned int m_unknown0;
    CriticalSection m_criticalSection;
    qResult m_firstError;
};

}
}

#endif
