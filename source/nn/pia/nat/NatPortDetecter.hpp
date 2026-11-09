#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_PORT_DETECTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_NAT_NAT_PORT_DETECTER_HPP

#include "nn/pia/nat/NatDetecter.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

class NatPortDetecter : public NatDetecter
{
public:
    NatPortDetecter();
    virtual ~NatPortDetecter();

    Result Startup(NatTraverser* traverser, common::CallContext* context, const common::InetAddress& address);
    virtual Result Startup(NatTraverser* traverser, common::CallContext* context, const common::InetAddress& address,
                           const transport::StationLocation& primary, const transport::StationLocation& secondary,
                           bool relay);
    void Cleanup();
    void SendNatPortDetectionProbes();
    void StartSendingNatCheckProbe();
    bool IsReceivedAllMessage();
    unsigned int HandleResult();
    unsigned int CheckRetry();
    unsigned int GetDetectionTimeout() const;

private:
    transport::StationLocation m_primaryStation;
    transport::StationLocation m_secondaryStation;
    bool m_relay;
    common::Time m_startTime;
};

}
}
}

#endif
