#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORING_SERVER_ADDRESS_RESOLVE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_MONITORING_SERVER_ADDRESS_RESOLVE_JOB_HPP

#include "nn/pia.h"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class InetAddress;
}
namespace inet
{

class MonitoringDataSender;

class MonitoringServerAddressResolveJob : public common::StepSequenceJob
{
public:
    MonitoringServerAddressResolveJob();
    virtual ~MonitoringServerAddressResolveJob();
    void Cleanup();
    ExecuteResult StepResolveMonitoringServerAddress();
    Result Startup(unsigned int monitoringServerId, common::InetAddress* address, MonitoringDataSender* sender);

private:
    common::CallContext m_callContext;
    unsigned int m_monitoringServerId;
    common::InetAddress* m_address;
    MonitoringDataSender* m_sender;
};

}
}
}

#endif
