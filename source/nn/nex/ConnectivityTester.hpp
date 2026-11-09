#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIVITYTESTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CONNECTIVITYTESTER_HPP

#include "nn/nex/Job.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class StationProbeList;

class ConnectivityTester : public RootObject
{
public:
    virtual ~ConnectivityTester() {}

    virtual bool ProbeStations(CallContext* pCallContext, StationProbeList* pStationProbeList, unsigned int uiTimeout,
                               bool bUseRelay, Job::JobType jobType) = 0;
};

}
}

#endif
