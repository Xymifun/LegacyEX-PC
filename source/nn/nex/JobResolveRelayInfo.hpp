#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOBRESOLVERELAYINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOBRESOLVERELAYINFO_HPP

#include "nn/nex/JobCallContextCallback.hpp"
#include "nn/nex/RelaySignatureKey.hpp"

namespace nn
{
namespace nex
{

class CallContext;

class JobResolveRelayInfo : public JobCallContextCallback
{
public:
    enum RelayAddressType
    {
        RELAY_ADDRESS_IP = 0,
        RELAY_ADDRESS_HOST_NAME = 1
    };

    enum RelayMode
    {
        RELAY_MODE_DISABLED = 0,
        RELAY_MODE_ADDRESS_ONLY = 1,
        RELAY_MODE_WHEN_NEEDED = 2,
        RELAY_MODE_ALWAYS = 3
    };

    explicit JobResolveRelayInfo(CallContext* pCallContext)
        : JobCallContextCallback(JOB_TYPE_0, L"StartResolveRelayInfoCallback", pCallContext),
          m_key()
    {
    }

    virtual ~JobResolveRelayInfo() {}

    virtual void ProccessSuccess(CallContext*) {}

    virtual void ProccessFailure(CallContext*) { FailResolvedRelayInfo(); }

    virtual void ProccessCancel() { FailResolvedRelayInfo(); }

    static void FailResolvedRelayInfo();
    static JobResolveRelayInfo* CreateAndStartJob(CallContext* pCallContext);

    bool SetRelayAddress();
    void ResolvedRelayInfo();

private:
    friend class JobStartNATSession;

    RelaySignatureKey m_key;
};

}
}

#endif
