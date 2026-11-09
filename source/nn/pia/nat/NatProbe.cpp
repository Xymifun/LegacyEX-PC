#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatProbe.hpp"

#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include "nn/pia/session/Mesh.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

NatProbe::NatProbe(const transport::StationLocation& location, const common::Time& start,
                   const common::TimeSpan& duration, unsigned char timeout, unsigned char sprayCount,
                   unsigned char sprayRange)
    : m_stationLocation(location),
      m_startTime(start),
      m_deadline(),
      m_received(0),
      m_sent(0),
      m_replied(0),
      m_rtt(-1),
      m_timeout(timeout),
      m_probeCount(0),
      m_probeLimit(sprayCount)
{
    m_updateInterval = common::TimeSpan::FromMSec(90000).m_tick;

    common::Time now;
    now.SetNow();
    m_deadline.m_tick = now.m_tick + duration.m_tick;

    if (location.m_field22 == cInvalidIndex)
    {
        common::InetAddress adjusted(location.m_stationAddress.m_inetAddress.m_address,
                                     static_cast<unsigned short>(location.m_stationAddress.m_inetAddress.m_port + 10));
        m_stationLocation.m_stationAddress.SetInetAddress(adjusted);
    }

    m_firstPort = m_stationLocation.m_stationAddress.m_inetAddress.m_port;
    m_lastPort = static_cast<unsigned short>(m_firstPort + sprayRange);
    if (m_lastPort < m_firstPort)
        m_lastPort = cPortMax;
}

bool NatProbe::UpdateTargetAddress(const transport::StationLocation& location)
{
    if (m_stationLocation.m_stationAddress.m_inetAddress.IsPrivate()
        && !location.m_stationAddress.m_inetAddress.IsPrivate())
    {
        if (session::g_piaMesh == 0 || !session::g_piaMesh->IsProcessedAtMeshBegin())
        {
            unsigned short& addressChangeCount = common::g_sessionBeginMonitoringContent.m_metric150;
            if (addressChangeCount == cCounterMax)
                addressChangeCount = 1;
            else
                ++addressChangeCount;
        }
        if (m_rtt != -1 || m_startTime.m_tick != 0)
            return false;
    }
    else if (!m_stationLocation.m_stationAddress.m_inetAddress.IsPrivate()
             && location.m_stationAddress.m_inetAddress.IsPrivate())
    {
        if (session::g_piaMesh == 0 || !session::g_piaMesh->IsProcessedAtMeshBegin())
        {
            unsigned short& addressChangeCount = common::g_sessionBeginMonitoringContent.m_metric150;
            if (addressChangeCount == cCounterMax)
                addressChangeCount = 1;
            else
                ++addressChangeCount;
        }
    }
    else if (m_startTime.m_tick != 0 && m_stationLocation.m_stationAddress != location.m_stationAddress)
    {
        return false;
    }

    common::InetAddress oldAddress(m_stationLocation.m_stationAddress.m_inetAddress);
    m_stationLocation.m_stationAddress.SetInetAddress(location.m_stationAddress.m_inetAddress);
    m_firstPort = location.m_stationAddress.m_inetAddress.m_port;
    m_lastPort = location.m_stationAddress.m_inetAddress.m_port;
    if (!(oldAddress == location.m_stationAddress.m_inetAddress))
        m_probeCount = 0;
    return true;
}

void NatProbe::UpdateTargetPort(const transport::StationLocation& location)
{
    if (m_startTime.m_tick != 0 && m_stationLocation.m_field26 == 0)
        return;
    const unsigned short oldPort = m_stationLocation.m_stationAddress.m_inetAddress.m_port;
    m_stationLocation.m_stationAddress.m_inetAddress.m_port = location.m_stationAddress.m_inetAddress.m_port;
    if (oldPort != location.m_stationAddress.m_inetAddress.m_port)
        m_probeCount = 0;
}

void NatProbe::UpdateRtt(const common::Time& sent, const common::Time& received)
{
    const common::TimeSpan elapsed = sent - received;
    m_rtt = static_cast<int>(elapsed.m_tick / common::TimeSpan::GetTicksPerMSec().m_tick);
}

bool NatProbe::UpdateIsNeeded(common::Time now) const
{
    if (m_rtt != -1)
    {
        const common::TimeSpan elapsed = now - m_startTime;
        return elapsed.m_tick > m_updateInterval;
    }
    return m_probeCount < m_probeLimit;
}

void NatProbe::SprayTargetPort()
{
    const unsigned short count = static_cast<unsigned short>(m_lastPort - m_firstPort + 1);
    if (count == 1)
        return;
    m_stationLocation.m_stationAddress.m_inetAddress.m_port = static_cast<unsigned short>(
        m_firstPort + static_cast<unsigned short>(m_probeCount) % count);
}

unsigned short NatProbe::GetPortSprayCount()
{
    return static_cast<unsigned short>(m_stationLocation.m_stationAddress.m_inetAddress.m_port - m_firstPort);
}

void NatProbe::Trace(unsigned long long) const
{
}

}
}
}
