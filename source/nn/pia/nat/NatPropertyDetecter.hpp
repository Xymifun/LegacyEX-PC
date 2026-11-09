#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_PROPERTY_DETECTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_PROPERTY_DETECTER_HPP

#include "nn/pia/common/Time.hpp"
#include "nn/pia/nat/NatDetecter.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

class NatPropertyDetecter : public NatDetecter
{
public:
    NatPropertyDetecter();
    virtual ~NatPropertyDetecter();
    virtual Result Startup(NatTraverser* traverser, common::CallContext* context, const common::InetAddress& address);
    virtual void Cleanup();
    void SendNatPropertyDetectionProbes();
    virtual void StartSendingNatCheckProbe();
    virtual bool IsReceivedAllMessage();
    virtual unsigned int HandleResult();
    virtual unsigned int CheckRetry();
    virtual unsigned int GetDetectionTimeout() const;

private:
    static unsigned short s_lastDetectionMilliseconds;
    common::Time m_startTime;
};

}
}
}

#endif
