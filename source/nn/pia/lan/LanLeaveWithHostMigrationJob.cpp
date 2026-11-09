#include "nn/pia/lan/LanLeaveWithHostMigrationJob.hpp"
#include "nn/pia/session/Mesh.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

LanLeaveWithHostMigrationJob::LanLeaveWithHostMigrationJob() : session::LeaveWithHostMigrationJob()
{
}
LanLeaveWithHostMigrationJob::~LanLeaveWithHostMigrationJob()
{
}
void LanLeaveWithHostMigrationJob::Trace(unsigned long long) const
{
}
int LanLeaveWithHostMigrationJob::DecideNextHost()
{
    int local = session::g_piaMesh->GetLocalStationIndex();
    if (local <= 0x1f && local == session::g_piaMesh->GetHostStationIndex())
    {
        for (int candidate = 0; candidate < 0x20; ++candidate)
            if (candidate != local && session::g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(candidate)))
                return candidate;
    }
    return cStationIndexInvalid;
}

}
}
}
