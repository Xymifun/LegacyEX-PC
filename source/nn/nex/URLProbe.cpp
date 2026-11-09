#include "nn/nex/URLProbe.hpp"

#include "nn/nex/TraceLog.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int PROBE_TRACE_INDENT = 2;

}

URLProbe::URLProbe(const StationURL& rStationURL, int iTimeout, int iInterval, unsigned int uiMaxProbes)
    : m_stationURL(rStationURL),
      m_uiPingTime(UNREACHED_PING_TIME),
      m_iInterval(iInterval),
      m_uiProbeCount(0),
      m_uiMaxProbes(uiMaxProbes)
{
    m_expirationTime = Time::GetTime() + iTimeout;
}

bool URLProbe::Update(const StationURL& rStationURL, Time pingSendTime)
{
    TraceLog::GetInstance()->GetOutputFormat()->IncreaseIndent(PROBE_TRACE_INDENT);
    m_stationURL.SetPortNumber(rStationURL.GetPortNumber());
    if (pingSendTime != Time())
        m_uiPingTime = static_cast<unsigned int>(Time::GetTime() - pingSendTime);
    TraceLog::GetInstance()->GetOutputFormat()->DecreaseIndent(PROBE_TRACE_INDENT);
    m_lastUpdateTime = Time::GetTime();
    return true;
}

bool URLProbe::UpdateIsNeeded(Time currentTime) const
{
    if (m_uiPingTime == UNREACHED_PING_TIME)
        return m_uiProbeCount < m_uiMaxProbes;

    return static_cast<long long>(currentTime - m_lastUpdateTime) > m_iInterval;
}

void URLProbe::Trace(unsigned long long)
{
}

}
}
