#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatTraversalTimeList.hpp"

#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

NatTraversalTime* NatTraversalTimeList::Find(const transport::StationLocation& location)
{
    common::ListNode* sentinel = Sentinel();
    for (common::ListNode* node = sentinel->next; node != sentinel; node = node->next)
    {
        NatTraversalTime* value = TimeList::ValueOf(node);
        if (value->m_location.m_field1c == location.m_field1c)
            return value;
        if (value->m_location.m_field1c != location.m_field1c
            && location.m_stationAddress.m_inetAddress == value->m_location.m_stationAddress.m_inetAddress)
            return value;
    }
    return 0;
}

void NatTraversalTimeList::Add(const NatTraversalTime& time)
{
    NatTraversalTime* value = Find(time.m_location);
    if (value == 0)
    {
        if (GetFreeNum() == 0)
            return;
        value = Alloc();
        if (value == 0)
            return;
    }

    time.m_location.Trace(cTraceBrief);
    *value = time;
}

void NatTraversalTimeList::Remove(const transport::StationLocation& location)
{
    common::ListNode* sentinel = Sentinel();
    common::ListNode* node = sentinel->next;
    while (node != sentinel)
    {
        NatTraversalTime* value = TimeList::ValueOf(node);
        if (value->m_location.m_field1c == location.m_field1c
            || (value->m_location.m_field1c != location.m_field1c
                && location.m_stationAddress.m_inetAddress == value->m_location.m_stationAddress.m_inetAddress))
        {
            value->m_location.Trace(cTraceBrief);
            node = node->next;
            Release(value);
        }
        else
        {
            node = node->next;
        }
    }
}

}
}
}
