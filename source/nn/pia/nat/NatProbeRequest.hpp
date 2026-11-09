#ifndef NN_PIA_NAT_NATPROBEREQUEST_HPP
#define NN_PIA_NAT_NATPROBEREQUEST_HPP

#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/transport/StationLocation.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

class NatProbeRequest
{
public:
    NatProbeRequest(const transport::StationLocation& location, const transport::StationConnectionInfo& connectionInfo,
                    bool probeRequested, bool addressValid, common::CallContext* callContext);
    explicit NatProbeRequest(const transport::StationLocation& location);
    NatProbeRequest() : m_stationLocation(), m_stationConnectionInfo(), m_deadline() {}
    virtual ~NatProbeRequest() {}

    virtual void Trace(unsigned long long value) const;

    transport::StationLocation m_stationLocation;
    transport::StationConnectionInfo m_stationConnectionInfo;
    bool m_probeRequested;
    bool m_addressValid;
    common::Time m_deadline;
    common::CallContext* m_callContext;
};

}
}
}

#endif
