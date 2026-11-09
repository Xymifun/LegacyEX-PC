#include "nn/pia/lan/LanStationInfo.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

LanStationInfo::LanStationInfo()
{
    m_userName.Clear();
}

void LanStationInfo::Clear()
{
    session::SessionStationInfo::Clear();
    m_userName.Clear();
}

const common::FixedString<10>& LanStationInfo::GetUserName() const
{
    return m_userName;
}

void LanStationInfo::SetUserName(const common::FixedString<10>& userName)
{
    m_userName.Copy(userName);
}

unsigned long long LanStationInfo::GetStationId() const
{
    return m_stationId;
}

void LanStationInfo::SetStationId(unsigned long long stationId)
{
    m_stationId = stationId;
}

unsigned int LanStationInfo::GetSerializedSize()
{
    return session::SessionStationInfo::GetSerializedSize() + 0x31;
}

}
}
}
