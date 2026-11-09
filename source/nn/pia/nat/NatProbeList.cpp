#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatProbeList.hpp"

#include "nn/pia/common/Memory.hpp"
#include "nn/pia/nat/NatProbe.hpp"
#include "nn/pia/nat/NatProbeRequestList.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

NatProbeList::NatProbeList() : m_allocation(0)
{
    m_allocation = common::NewArray<unsigned char>(64 * sizeof(ProbeList::Node));
    Initialize(m_allocation, 64);
    ProbeList::Clear();
}

NatProbeList::~NatProbeList()
{
    if (m_allocation != 0)
    {
        if (IsInitialized())
            ProbeList::Clear();
        common::DeleteArray(m_allocation);
        m_allocation = 0;
    }
}

bool NatProbeList::IsSameAddressPortAndConnectionId(const transport::StationLocation& a,
                                                    const transport::StationLocation& b)
{
    return a.m_stationAddress.m_inetAddress == b.m_stationAddress.m_inetAddress && a.m_field1c == b.m_field1c;
}

bool NatProbeList::IsDifferentPortSameAddressAndConnectionId(const transport::StationLocation& a,
                                                             const transport::StationLocation& b)
{
    return a.m_stationAddress.m_inetAddress.m_address == b.m_stationAddress.m_inetAddress.m_address
           && a.m_field1c == b.m_field1c;
}

NatProbe* NatProbeList::FindByAddressPortAndConnectionId(const transport::StationLocation& location)
{
    for (common::ListNode* node = Sentinel()->next; node != Sentinel(); node = node->next)
    {
        if (IsSameAddressPortAndConnectionId(ProbeOf(node)->m_stationLocation, location))
            return ProbeOf(node);
    }
    return 0;
}

NatProbe* NatProbeList::FindByAddressAndConnectionId(const transport::StationLocation& location)
{
    for (common::ListNode* node = Sentinel()->next; node != Sentinel(); node = node->next)
    {
        ProbeList::Node* entry = static_cast<ProbeList::Node*>(node);
        if (IsDifferentPortSameAddressAndConnectionId(entry->value.m_stationLocation, location))
            return &entry->value;
    }
    return 0;
}

NatProbe* NatProbeList::FindByConnectionId(unsigned int connectionId)
{
    for (common::ListNode* node = Sentinel()->next; node != Sentinel(); node = node->next)
    {
        ProbeList::Node* entry = static_cast<ProbeList::Node*>(node);
        if (entry->value.m_stationLocation.m_field1c == static_cast<int>(connectionId))
            return &entry->value;
    }
    return 0;
}

void NatProbeList::RemoveNotReceivedProbes(unsigned int connectionId)
{
    common::ListNode* node = Sentinel()->next;
    while (node != Sentinel())
    {
        NatProbe* probe = ProbeOf(node);
        if (probe->m_startTime.m_tick == 0 && probe->m_stationLocation.m_field1c == static_cast<int>(connectionId))
        {
            probe->Trace(cTraceBrief);
            node = node->next;
            Release(probe);
        }
        else
        {
            node = node->next;
        }
    }
}

void NatProbeList::RemoveDifferentConnectionIdAndSameAddressPortProbes(const transport::StationLocation& location)
{
    common::ListNode* node = Begin();
    common::ListNode* end = End();
    while (node != end)
    {
        NatProbe* probe = ProbeOf(node);
        if (probe->m_stationLocation.m_field1c != location.m_field1c
            && probe->m_stationLocation.m_stationAddress.m_inetAddress == location.m_stationAddress.m_inetAddress)
        {
            probe->Trace(cTraceBrief);
            node = node->next;
            Release(probe);
        }
        else
        {
            node = node->next;
        }
    }
}

void NatProbeList::RemoveDifferentPortAndSameAddressAndConnectionIdProbes(const transport::StationLocation& location)
{
    common::ListNode* node = Begin();
    common::ListNode* end = End();
    while (node != end)
    {
        NatProbe* probe = ProbeOf(node);
        if (IsDifferentPortSameAddressAndConnectionId(probe->m_stationLocation, location))
        {
            probe->Trace(cTraceBrief);
            node = node->next;
            Release(probe);
        }
        else
        {
            node = node->next;
        }
    }
}

bool NatProbeList::AddProbe(const NatProbe& probe)
{
    probe.Trace(cTraceBrief);
    if (probe.m_startTime.m_tick != 0)
    {
        RemoveNotReceivedProbes(static_cast<unsigned int>(probe.m_stationLocation.m_field1c));
        RemoveDifferentConnectionIdAndSameAddressPortProbes(probe.m_stationLocation);
    }
    else
    {
        RemoveDifferentConnectionIdAndSameAddressPortProbes(probe.m_stationLocation);
        if (FindByAddressPortAndConnectionId(probe.m_stationLocation) != 0)
            return true;

        if (FindByAddressAndConnectionId(probe.m_stationLocation) != 0)
        {
            RemoveDifferentPortAndSameAddressAndConnectionIdProbes(probe.m_stationLocation);
        }
        else
        {
            NatProbe* sameConnection = FindByConnectionId(static_cast<unsigned int>(probe.m_stationLocation.m_field1c));
            if (sameConnection != 0 && sameConnection->m_rtt != -1
                && sameConnection->m_stationLocation.m_stationAddress.m_inetAddress.IsPrivate()
                && !probe.m_stationLocation.m_stationAddress.m_inetAddress.IsPrivate())
                return true;
        }
    }

    NatProbe* target = probe.m_received ? AllocFront() : Alloc();
    if (target == 0)
        return false;

    *target = probe;
    return true;
}

bool NatProbeList::RemoveSameConnectionIdOtherProbes(const transport::StationLocation& location)
{
    bool removed = false;
    common::ListNode* node = Sentinel()->next;
    while (node != Sentinel())
    {
        NatProbe* probe = ProbeOf(node);
        if (probe->m_stationLocation.m_field1c == location.m_field1c
            && !IsSameAddressPortAndConnectionId(probe->m_stationLocation, location))
        {
            probe->Trace(cTraceBrief);
            node = node->next;
            Release(probe);
            removed = true;
        }
        else
        {
            node = node->next;
        }
    }
    return removed;
}

bool NatProbeList::ProcessProbe(const transport::StationLocation& location, unsigned char timeout)
{
    common::Time now;
    now.SetNow();
    NatProbe* probe = FindByAddressPortAndConnectionId(location);
    if (probe == 0)
        probe = FindByConnectionId(static_cast<unsigned int>(location.m_field1c));

    bool result;
    if (probe == 0)
    {
        location.Trace(cTraceBrief);
        const common::TimeSpan duration(g_natProbeDurationMsec * common::TimeSpan::GetTicksPerMSec().m_tick);
        const NatProbe newProbe(location, now, duration, timeout, g_natProbeSprayCount, 0);
        result = AddProbe(newProbe);
    }
    else
    {
        result = probe->UpdateTargetAddress(location);
        RemoveDifferentConnectionIdAndSameAddressPortProbes(probe->m_stationLocation);
        probe->m_startTime = now;
        RemoveNotReceivedProbes(static_cast<unsigned int>(probe->m_stationLocation.m_field1c));
        RemoveSameConnectionIdOtherProbes(probe->m_stationLocation);
    }
    return result;
}

void NatProbeList::ProcessProbeReply(const transport::StationLocation& location, common::Time sent,
                                     unsigned char timeout)
{
    NatProbe* probe = FindByAddressAndConnectionId(location);
    if (probe == 0)
    {
        location.Trace(cTraceBrief);
        return;
    }

    probe->UpdateTargetPort(location);
    RemoveDifferentConnectionIdAndSameAddressPortProbes(probe->m_stationLocation);
    common::Time now;
    now.SetNow();
    if (sent.m_tick != 0)
        probe->UpdateRtt(now, sent);
    probe->m_startTime = now;
    probe->m_timeout = timeout;
    RemoveNotReceivedProbes(static_cast<unsigned int>(probe->m_stationLocation.m_field1c));
    RemoveSameConnectionIdOtherProbes(probe->m_stationLocation);
}

bool NatProbeList::RemoveSameAddressAndConnectionIdProbes(const transport::StationLocation& location)
{
    bool removed = false;
    common::ListNode* node = Sentinel()->next;
    while (node != Sentinel())
    {
        NatProbe* probe = ProbeOf(node);
        if (IsDifferentPortSameAddressAndConnectionId(probe->m_stationLocation, location))
        {
            probe->Trace(cTraceBrief);
            node = node->next;
            Release(probe);
            removed = true;
        }
        else
        {
            node = node->next;
        }
    }
    return removed;
}

bool NatProbeList::RemoveSameAddressPortAndConnectionIdProbes(const transport::StationLocation& location)
{
    bool removed = false;
    common::ListNode* node = Begin();
    common::ListNode* end = End();
    while (node != end)
    {
        NatProbe* probe = ProbeOf(node);
        if (IsSameAddressPortAndConnectionId(probe->m_stationLocation, location))
        {
            probe->Trace(cTraceBrief);
            node = node->next;
            Release(probe);
            removed = true;
        }
        else
        {
            node = node->next;
        }
    }
    return removed;
}

bool NatProbeList::RemoveProbesByConnectionId(unsigned int connectionId)
{
    bool removed = false;
    common::ListNode* node = Sentinel()->next;
    while (node != Sentinel())
    {
        NatProbe* probe = ProbeOf(node);
        if (probe->m_stationLocation.m_field1c == static_cast<int>(connectionId))
        {
            probe->Trace(cTraceBrief);
            node = node->next;
            Release(probe);
            removed = true;
        }
        else
        {
            node = node->next;
        }
    }
    return removed;
}

void NatProbeList::Trace(unsigned long long) const
{
}

NatProbeRequestList::NatProbeRequestList() : m_allocation(0)
{
    m_allocation = common::NewArray<unsigned char>(64 * sizeof(RequestList::Node));
    Initialize(m_allocation, 64);
    RequestList::Clear();
}

NatProbeRequestList::~NatProbeRequestList()
{
    if (m_allocation != 0)
    {
        if (IsInitialized())
            RequestList::Clear();
        common::DeleteArray(m_allocation);
        m_allocation = 0;
    }
}

}
}
}
