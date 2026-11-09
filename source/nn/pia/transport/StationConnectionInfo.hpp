#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_CONNECTION_INFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_STATION_CONNECTION_INFO_HPP

#include "nn/pia/transport/StationLocation.hpp"
#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{
class String;
}
}
}

namespace nn
{
namespace pia
{
namespace transport
{

class StationConnectionInfo
{
public:
    StationConnectionInfo();
    StationConnectionInfo(const StationConnectionInfo& other);
    virtual ~StationConnectionInfo();
    StationConnectionInfo& operator=(const StationConnectionInfo& other);
    void SetStationConnectionInfo(const StationConnectionInfo& other);
    bool operator==(const StationConnectionInfo& other) const;
    virtual unsigned int GetSerializedSize() const;
    virtual Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    virtual Result Deserialize(const unsigned char* buffer);
    virtual unsigned int GetStreamlinedSerializedSize() const;
    virtual Result StreamlinedSerialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    virtual Result StreamlinedDeserialize(const unsigned char* buffer);
    virtual void Trace(unsigned long long value) const;
    unsigned int LittleMembersSerialize(unsigned char* buffer, const StationLocation& location) const;
    unsigned int LittleMembersDeserialize(StationLocation* location, const unsigned char* buffer);

    StationLocation m_stationLocation;
    StationLocation m_localStationLocation;
};

}
}
}

#endif
