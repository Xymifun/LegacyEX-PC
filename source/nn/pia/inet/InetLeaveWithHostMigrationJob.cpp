#include "nn/pia/inet/InetLeaveWithHostMigrationJob.hpp"

#include "nn/pia/session/Mesh.hpp"

namespace nn
{
namespace pia
{
namespace inet
{

InetLeaveWithHostMigrationJob::InetLeaveWithHostMigrationJob() : session::LeaveWithHostMigrationJob()
{
}
InetLeaveWithHostMigrationJob::~InetLeaveWithHostMigrationJob()
{
}
void InetLeaveWithHostMigrationJob::Trace(unsigned long long) const
{
}
int InetLeaveWithHostMigrationJob::DecideNextHost()
{
    const StationIndex local = session::g_piaMesh->GetLocalStationIndex();
    if (local <= 31 && local == session::g_piaMesh->GetHostStationIndex())
    {
        for (int candidate = 0; candidate < 32; ++candidate)
            if (candidate != local && session::g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(candidate)))
                return candidate;
    }
    return cStationIndexInvalid;
}

}
}
}
