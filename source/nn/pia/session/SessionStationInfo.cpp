#include "nn/pia/Reconstruction.hpp"

namespace nn
{
namespace pia
{
namespace session
{

SessionStationInfo::SessionStationInfo() : m_role(cStationInfoRole_Invalid)
{
}

SessionStationInfo::~SessionStationInfo()
{
}

void SessionStationInfo::Clear()
{
    m_role = cStationInfoRole_Invalid;
}

SessionStationInfo::StationInfoRole SessionStationInfo::GetRole() const
{
    return m_role;
}

void SessionStationInfo::SetRole(StationInfoRole role)
{
    m_role = role;
}

unsigned int SessionStationInfo::GetSerializedSize()
{
    return 1;
}

}
}
}
