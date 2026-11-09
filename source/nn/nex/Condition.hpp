#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CONDITION_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CONDITION_HPP

#include <coreinit/condition.h>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Condition : public RootObject
{
public:
    Condition(unsigned int uiTraceMask, bool bFlag);
    virtual ~Condition();

    void Notify();
    void CancelAll();
    bool WaitForOSEvent(ForcedCriticalSection* pLock, unsigned int uiTimeout);
    bool WaitForEvent(ForcedCriticalSection* pLock, unsigned int uiTimeout);

    void ResetEvent() {}

private:
    class PlatformCondition : public RootObject
    {
    public:
        OSCondition m_cond;
    };

    PlatformCondition* m_pPlatformCondition;
    ForcedCriticalSection m_criticalSection;
    int m_iWaiterCount;
    bool m_bFlag;
};

}
}

#endif
