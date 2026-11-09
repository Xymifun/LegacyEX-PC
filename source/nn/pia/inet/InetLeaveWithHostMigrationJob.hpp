#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETLEAVEWITHHOSTMIGRATIONJOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_INETLEAVEWITHHOSTMIGRATIONJOB_HPP
#include "nn/pia/session/LeaveWithHostMigrationJob.hpp"
namespace nn
{
namespace pia
{
namespace inet
{
class InetLeaveWithHostMigrationJob : public session::LeaveWithHostMigrationJob
{
public:
    InetLeaveWithHostMigrationJob();
    virtual ~InetLeaveWithHostMigrationJob();
    void Trace(unsigned long long) const;
    int DecideNextHost();
};
}
}
}
#endif
