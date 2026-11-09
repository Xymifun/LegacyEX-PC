#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_WITH_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_SESSION_LEAVE_WITH_HOST_MIGRATION_JOB_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/CallContext.hpp"
#include "nn/pia/common/StepSequenceJob.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace session
{

class LeaveWithHostMigrationJob : public common::StepSequenceJob
{
public:
    enum
    {
        cStationNumMax = 32
    };

    LeaveWithHostMigrationJob();
    virtual ~LeaveWithHostMigrationJob();

    bool Startup(common::CallContext* context);
    ExecuteResult SendStartMigrationMessage();
    ExecuteResult WaitMigrationResponse();
    ExecuteResult CleanupMesh();
    ExecuteResult SendStartMultiMigrationMessage();
    ExecuteResult CheckHostMigrationProcess();
    ExecuteResult WaitHostMigrationProcess();
    ExecuteResult StartDisconnectStations();
    ExecuteResult WaitDisconnectStations();
    void ReceiveMigrationResponse(StationIndex station);
    void Cleanup();
    virtual void Trace(unsigned long long value) const;
    virtual int DecideNextHost() = 0;
    virtual void CleanupStatus();

    common::CallContext* m_callerContext;
    common::Time m_deadline;
    int m_timeoutMsec;
    bool m_started;
    StationIndex m_newHost;
    bool m_stationResponses[cStationNumMax];
    bool m_migrationStarted;
};

}
}
}

#endif
