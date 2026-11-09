#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_STATION_ADDRESS_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_STATION_ADDRESS_HPP

#include "nn/pia/common/InetAddress.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class StationAddress
{
public:
    StationAddress();
    StationAddress(const StationAddress& other);
    virtual ~StationAddress() {}
    Result SetInetAddress(const InetAddress& address);
    void Clear();
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer);
    bool IsValid() const;
    bool operator==(const StationAddress& other) const;
    bool operator!=(const StationAddress& other) const { return !(*this == other); }
    StationAddress& operator=(const StationAddress& other);
    bool operator<(const StationAddress& other) const;
    bool operator>(const StationAddress& other) const;
    static int Compare(const StationAddress& lhs, const StationAddress& rhs);
    void GetAddressString(String* string) const;
    virtual void Trace(unsigned long long value) const;
    unsigned int GetSerializedSize() const;

    InetAddress m_inetAddress;
};

}
}
}

#endif
