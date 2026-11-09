#include "nn/pia/transport/RttCalculator.hpp"
#include <algorithm>
#include "nn/pia/Transport.hpp"
namespace nn
{
namespace pia
{
namespace transport
{
int RttCalculator::s_pulseInterval = 2000;
int RttCalculator::s_initialPulseInterval = 2000;

RttCalculator::RttCalculator() : m_started(0), m_lastRtt(-1), m_rttMin(-1), m_rttMax(-1), m_latestMedian()
{
    m_latestMedian.m_count = 0;
    m_latestMedian.m_index = 0;
}

RttCalculator::~RttCalculator()
{
}

int RttCalculator::GetRtt() const
{
    if (!m_started || m_latestMedian.m_count == 0)
        return -1;
    return m_latestMedian.GetMedian();
}

int RttCalculator::GetRtt(unsigned int count) const
{
    if (!m_started || m_latestMedian.m_count == 0)
        return -1;
    if (count > m_latestMedian.m_count)
        count = m_latestMedian.m_count;

    int values[9];
    for (unsigned int i = 0; i < count; ++i)
    {
        const unsigned int index = (m_latestMedian.m_index + m_latestMedian.m_count - i - 1) % 9;
        values[i] = m_latestMedian.m_values[index];
    }
    std::sort(values, values + count);

    const unsigned int middle = count / 2;
    if ((count & 1u) == 0)
    {
        const int sum = values[middle - 1] + values[middle];
        return (sum >> 1) + ((sum < 0 && (sum & 1)) ? 1 : 0);
    }
    return values[middle];
}

void RttCalculator::ResetRttMinMax()
{
    m_rttMin = -1;
    m_rttMax = -1;
}

int RttCalculator::GetRttMin() const
{
    return m_started ? m_rttMin : -1;
}

int RttCalculator::GetRttMax() const
{
    return m_started ? m_rttMax : -1;
}

void RttCalculator::Startup()
{
    m_started = 1;
    m_lastRtt = -1;
    m_lastRequestedTime.m_tick = 0;
    m_latestMedian.m_count = 0;
    m_latestMedian.m_index = 0;
    ResetRttMinMax();
}

void RttCalculator::Cleanup()
{
    if (!m_started)
        return;
    m_started = 0;
    m_lastRequestedTime = common::Time();
    m_lastRtt = -1;
    m_latestMedian.m_count = 0;
    m_latestMedian.m_index = 0;
    ResetRttMinMax();
}

bool RttCalculator::IsTimeOut() const
{
    const int interval = m_latestMedian.m_count == 9 ? s_pulseInterval : s_initialPulseInterval;
    const common::TimeSpan elapsed = g_piaTransport->m_time - m_lastRequestedTime;
    const int elapsedMsec = static_cast<int>(elapsed.m_tick / common::TimeSpan::GetTicksPerMSec().m_tick);
    return elapsedMsec >= interval;
}

void RttCalculator::SetLastRequestedTime(const common::Time& time)
{
    m_lastRequestedTime = time;
}

void RttCalculator::Update(int rtt)
{
    m_lastRtt = rtt;
    m_latestMedian.m_values[(m_latestMedian.m_index + m_latestMedian.m_count) % 9] = rtt;
    if (m_latestMedian.m_count == 9)
        m_latestMedian.m_index = (m_latestMedian.m_index + 1) % 9;
    else
        ++m_latestMedian.m_count;

    if (m_rttMin == -1 || rtt < m_rttMin)
        m_rttMin = rtt;
    if (m_rttMax == -1 || rtt > m_rttMax)
        m_rttMax = rtt;
}

Result RttCalculator::SetInitialPulseInterval(int interval)
{
    if (interval < 1)
        return cResultInvalidArgument;
    s_initialPulseInterval = interval;
    return 0;
}

Result RttCalculator::SetPulseInterval(int interval)
{
    if (interval < 1)
        return cResultInvalidArgument;
    s_pulseInterval = interval;
    return 0;
}
}
}
}
