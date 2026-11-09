#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETJOINMESHJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETJOINMESHJOB_HPP

#include "nn/pia/session/JoinMeshJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class InetJoinMeshJob : public session::JoinMeshJob
{
public:
    InetJoinMeshJob();
    virtual ~InetJoinMeshJob();
    virtual unsigned int StartupImpl();
    ExecuteResult StartMonitoringServerAddressResolution();
    ExecuteResult StartBandwidthCheck();
    ExecuteResult WaitBandwidthCheck();
    ExecuteResult WaitCheckHostConnection();
    ExecuteResult WaitCheckKickoutJob();
    virtual void SetHostInfoToMonitoringData(const transport::StationConnectionInfo& info);
    virtual void SetNextStepAtSetupLocalPlayerInfoImpl();
    virtual ExecuteResult SetNextStepAfterWaitAllConnection();
    virtual void CleanupImpl();
};

}
}
}

#endif
