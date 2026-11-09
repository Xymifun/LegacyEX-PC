#include "nn/pia/session/SyncClock.hpp"

#include "nn/pia/common/TimeSpan.hpp"

namespace nn
{
namespace pia
{
namespace session
{

SyncClock::SyncClock() : m_baseTime(0), m_hasBaseTime(false)
{
}

SyncClock::~SyncClock()
{
}

void SyncClock::SetBaseTime(const common::Time& time)
{
    m_baseTime = time.m_tick;
    m_hasBaseTime = true;
}

void SyncClock::ClearBaseTime()
{
    m_hasBaseTime = false;
    m_baseTime = 0;
}

unsigned long long SyncClock::GetTime() const
{
    if (m_hasBaseTime)
    {
        common::Time now;
        now.SetNow();
        return static_cast<long long>(now.m_tick - m_baseTime) / common::TimeSpan::GetTicksPerMSec().m_tick;
    }
    return static_cast<unsigned long long>(-1);
}

}
}
}
