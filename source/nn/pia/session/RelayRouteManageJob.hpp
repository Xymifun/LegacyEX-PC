#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_RELAY_ROUTE_MANAGE_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_RELAY_ROUTE_MANAGE_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class RelayRouteManageJob : public common::StepSequenceJob
{
public:
    RelayRouteManageJob();
    virtual ~RelayRouteManageJob();
    virtual void Trace(unsigned long long value) const;

    bool Startup(StationIndex stationIndex, unsigned int generation, unsigned int reportSerial);
    void UpdateConnectionReport(StationIndex stationIndex, const unsigned char* report, unsigned int size);
    ExecuteResult WaitAllDirectConnectionReport();
    ExecuteResult SendRelayRouteDirections();
    void Cleanup();
    void PrepareForBecomingNewHost();

    unsigned short m_stationNum;
    unsigned int m_generation;
    unsigned int m_sequenceNumber;
    unsigned char* m_connectionReports;
    unsigned int* m_reportSerials;
    unsigned int m_stationBitmap;
    unsigned int m_refusedStationBitmap;
    unsigned char* m_failureReasons;
    common::Time m_resendTime;
};

}
}
}

#endif
