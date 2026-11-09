#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_INET_ADDRESS_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_INET_ADDRESS_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class String;

class InetAddress
{
public:
    enum
    {
        cSerializedSize = 6
    };

    InetAddress();
    InetAddress(unsigned int address, unsigned short port);
    InetAddress(const InetAddress& other);
    ~InetAddress();
    void operator=(const InetAddress& other);
    long long GetKey() const;
    bool operator==(const InetAddress& other) const { return GetKey() == other.GetKey(); }
    void GetAddressString(String* string) const;
    void Trace(unsigned long long value) const;
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer);
    bool IsValid() const;
    bool IsValidAddress() const;
    unsigned int IsPrivate() const;
    bool IsValidPort() const;

    unsigned int m_address;
    unsigned short m_port;
};

}
}
}

#endif
