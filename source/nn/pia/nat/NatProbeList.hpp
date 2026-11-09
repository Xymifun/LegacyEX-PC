#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBELIST_HPP

#include <stdint.h>

#include "nn/pia/nat/Api.hpp"
#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/nat/NatProbe.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

class NatProbeList : public common::PooledList<NatProbe>
{
public:
    typedef common::PooledList<NatProbe> ProbeList;

    NatProbeList();
    virtual ~NatProbeList();

    void RemoveNotReceivedProbes(unsigned int connectionId);
    void RemoveDifferentConnectionIdAndSameAddressPortProbes(const transport::StationLocation& location);
    static bool IsSameAddressPortAndConnectionId(const transport::StationLocation&, const transport::StationLocation&);
    NatProbe* FindByAddressPortAndConnectionId(const transport::StationLocation&);
    static bool IsDifferentPortSameAddressAndConnectionId(const transport::StationLocation&,
                                                          const transport::StationLocation&);
    NatProbe* FindByAddressAndConnectionId(const transport::StationLocation&);
    void RemoveDifferentPortAndSameAddressAndConnectionIdProbes(const transport::StationLocation&);
    NatProbe* FindByConnectionId(unsigned int connectionId);
    bool AddProbe(const NatProbe&);
    bool RemoveSameConnectionIdOtherProbes(const transport::StationLocation&);
    bool ProcessProbe(const transport::StationLocation&, unsigned char);
    void ProcessProbeReply(const transport::StationLocation&, common::Time, unsigned char);
    bool RemoveSameAddressAndConnectionIdProbes(const transport::StationLocation&);
    bool RemoveSameAddressPortAndConnectionIdProbes(const transport::StationLocation&);
    bool RemoveProbesByConnectionId(unsigned int connectionId);
    virtual void Trace(unsigned long long) const;

    common::ListNode* Begin() { return Sentinel()->next; }

    common::ListNode* End() { return Sentinel(); }

    static NatProbe* ProbeOf(common::ListNode* node) { return ProbeList::ValueOf(node); }

    void Release(NatProbe* probe) { Destroy(probe); }

    void ReleaseAll()
    {
        while (GetNodeNum() != 0)
            Release(Back());
    }

private:
    unsigned char* m_allocation;
};

}
}
}

#endif
