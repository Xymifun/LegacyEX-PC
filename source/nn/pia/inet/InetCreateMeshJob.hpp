#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETCREATEMESHJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETCREATEMESHJOB_HPP

#include "nn/pia/session/CreateMeshJob.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

class InetCreateMeshJob : public session::CreateMeshJob
{
public:
    InetCreateMeshJob();
    virtual ~InetCreateMeshJob();
    virtual Result StartupImpl();
    virtual void SetHostInfoToMonitoringDataImpl();
    virtual void SetNextStepAtSetupLocalPlayerInfoImpl();
    ExecuteResult StartMonitoringServerAddressResolution();
    virtual void CleanupImpl();
};

}
}
}

#endif
