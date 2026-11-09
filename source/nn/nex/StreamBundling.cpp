#include "nn/nex/StreamBundling.hpp"

#include "nn/nex/Core.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

StreamBundling::StreamBundling() : m_lastFlushTime()
{
    m_bEnabled = false;
    m_iMaxFlushDelay = 0;
    m_lastFlushTime = 0;
    m_unknown10 = 0xffff;
    m_unknown12 = 0xffff;
}

StreamBundling::~StreamBundling()
{
}

void StreamBundling::Enable(int iMaxFlushDelay)
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_iMaxFlushDelay = iMaxFlushDelay;
    m_bEnabled = true;
    m_lastFlushTime = Time(Time::GetTime());
}

void StreamBundling::Flush()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    m_lastFlushTime = Time(Time::GetTime());
}

void StreamBundling::SetMaxFlushDelay(int iMaxFlushDelay)
{
    m_iMaxFlushDelay = iMaxFlushDelay;
}

void StreamBundling::UpdateFlushTime()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    Time currentTime(Time::GetTime());
    Time elapsed = currentTime - m_lastFlushTime;
    if (static_cast<long long>(elapsed) > m_iMaxFlushDelay)
        m_lastFlushTime = currentTime;
}

Time StreamBundling::GetLastFlushTime()
{
    ScopedCS lock(&Scheduler::GetInstance()->GetCriticalSection());
    UpdateFlushTime();
    return m_lastFlushTime;
}

}
}
