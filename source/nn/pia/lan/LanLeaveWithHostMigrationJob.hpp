#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LEAVE_WITH_HOST_MIGRATION_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LEAVE_WITH_HOST_MIGRATION_JOB_HPP

#include "nn/pia/session/LeaveWithHostMigrationJob.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanLeaveWithHostMigrationJob : public session::LeaveWithHostMigrationJob
{
public:
    LanLeaveWithHostMigrationJob();
    virtual ~LanLeaveWithHostMigrationJob();
    void Trace(unsigned long long value) const;
    int DecideNextHost();
};

}
}
}

#endif
