#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALTIMELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATTRAVERSALTIMELIST_HPP

#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/nat/NatTraversalTime.hpp"

namespace nn
{
namespace pia
{
namespace transport
{
class StationLocation;
}
namespace nat
{

class NatTraversalTimeList : public common::FixedPooledList<NatTraversalTime, 32>
{
public:
    typedef common::PooledList<NatTraversalTime> TimeList;

    NatTraversalTimeList() { TimeList::Clear(); }
    virtual ~NatTraversalTimeList() { TimeList::Clear(); }

    NatTraversalTime* Find(const transport::StationLocation& location);
    void Add(const NatTraversalTime& time);
    void Remove(const transport::StationLocation& location);

    void Release(NatTraversalTime* time) { Destroy(time); }

    void ReleaseAll()
    {
        while (GetNodeNum() != 0)
            Release(Back());
    }
};

}
}
}

#endif
