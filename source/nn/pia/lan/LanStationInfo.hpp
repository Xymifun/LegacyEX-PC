#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSTATIONINFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSTATIONINFO_HPP

#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/String.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

class LanStationInfo : public session::SessionStationInfo
{
public:
    LanStationInfo();

    void Clear();
    const common::FixedString<10>& GetUserName() const;
    void SetUserName(const common::FixedString<10>& userName);
    unsigned long long GetStationId() const;
    void SetStationId(unsigned long long stationId);
    static unsigned int GetSerializedSize();

    common::FixedString<10> m_userName;
    unsigned long long m_stationId;
};

}
}
}

#endif
