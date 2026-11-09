#include "nn/nex/Condition.hpp"

#include "nn/nex/Platform.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int INFINITE_TIMEOUT = 0xFFFFFFFF;

}

Condition::Condition(unsigned int uiTraceMask, bool bFlag) : m_criticalSection(uiTraceMask)
{
    m_bFlag = bFlag;
    m_pPlatformCondition = new PlatformCondition();
    m_iWaiterCount = 0;
    OSInitCond(&m_pPlatformCondition->m_cond);
}

Condition::~Condition()
{
    CancelAll();
    delete m_pPlatformCondition;
}

void Condition::Notify()
{
    OSSignalCond(&m_pPlatformCondition->m_cond);
}

void Condition::CancelAll()
{
    do
    {
        Notify();
        if (m_iWaiterCount > 0)
        {
            Platform::YieldThread();
            Platform::Sleep(1);
        }
    } while (m_iWaiterCount > 0);
}

bool Condition::WaitForOSEvent(ForcedCriticalSection* pLock, unsigned int uiTimeout)
{
    m_criticalSection.Enter();
    ++m_iWaiterCount;
    m_criticalSection.Leave();
    if (uiTimeout == INFINITE_TIMEOUT)
        OSWaitCond(&m_pPlatformCondition->m_cond, &pLock->m_mutex.m_mutex);
    m_criticalSection.Enter();
    --m_iWaiterCount;
    m_criticalSection.Leave();
    return true;
}

bool Condition::WaitForEvent(ForcedCriticalSection* pLock, unsigned int uiTimeout)
{
    return WaitForOSEvent(pLock, uiTimeout);
}

}
}
