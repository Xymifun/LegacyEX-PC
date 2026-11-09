#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBEREQUESTLIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NATPROBEREQUESTLIST_HPP

#include "nn/pia/common/PooledList.hpp"
#include "nn/pia/nat/NatProbeRequest.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

class NatProbeRequestList : public common::PooledList<NatProbeRequest>
{
public:
    typedef common::PooledList<NatProbeRequest> RequestList;

    NatProbeRequestList();
    virtual ~NatProbeRequestList();

    unsigned int GetSize() const { return GetNodeNum(); }

    common::ListNode* Begin() { return Sentinel()->next; }

    common::ListNode* End() { return Sentinel(); }

    static NatProbeRequest* RequestOf(common::ListNode* node) { return RequestList::ValueOf(node); }

    NatProbeRequest* Allocate() { return Alloc(); }

    void Release(NatProbeRequest* request) { Destroy(request); }

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
