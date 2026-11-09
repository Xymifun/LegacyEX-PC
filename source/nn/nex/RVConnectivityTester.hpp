#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RVCONNECTIVITYTESTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RVCONNECTIVITYTESTER_HPP

#include "nn/nex/ConnectivityTester.hpp"

namespace nn
{
namespace nex
{

class RVConnectivityTester : public ConnectivityTester
{
public:
    RVConnectivityTester();
    virtual ~RVConnectivityTester();

    virtual bool ProbeStations(CallContext* pCallContext, StationProbeList* pStationProbeList, unsigned int uiTimeout,
                               bool bUseRelay, Job::JobType jobType);
};

}
}

#endif
