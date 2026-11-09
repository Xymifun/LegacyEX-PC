#include "nn/pia/common/StationAddress.hpp"

namespace nn
{
namespace pia
{
namespace common
{

StationAddress::StationAddress()
{
}

StationAddress::StationAddress(const StationAddress& other) : m_inetAddress(other.m_inetAddress)
{
}

Result StationAddress::SetInetAddress(const InetAddress& address)
{
    m_inetAddress = address;
    return 0;
}

void StationAddress::Clear()
{
    m_inetAddress.m_address = 0;
    m_inetAddress.m_port = 0;
}

Result StationAddress::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    const unsigned int serializedSize = InetAddress::cSerializedSize;
    if (buffer == 0)
        return 0x10c07;
    if (size == 0)
        return 0x10c07;
    if (maxSize < serializedSize)
        return 0x10c07;

    unsigned int inetAddressSize;
    const Result result = m_inetAddress.Serialize(buffer, &inetAddressSize, maxSize);
    if (result != 0)
        return result;

    *size = serializedSize;
    return 0;
}

Result StationAddress::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;

    const Result result = m_inetAddress.Deserialize(buffer);
    if (result != 0)
        return result;

    return 0;
}

bool StationAddress::IsValid() const
{
    return m_inetAddress.IsValid();
}

bool StationAddress::operator==(const StationAddress& other) const
{
    return m_inetAddress.GetKey() == other.m_inetAddress.GetKey();
}

StationAddress& StationAddress::operator=(const StationAddress& other)
{
    if (this != &other)
        m_inetAddress = other.m_inetAddress;
    return *this;
}

bool StationAddress::operator<(const StationAddress& other) const
{
    return m_inetAddress.GetKey() < other.m_inetAddress.GetKey();
}

int StationAddress::Compare(const StationAddress& lhs, const StationAddress& rhs)
{
    if (lhs.m_inetAddress.GetKey() < rhs.m_inetAddress.GetKey())
        return -1;
    return rhs.m_inetAddress.GetKey() < lhs.m_inetAddress.GetKey();
}

void StationAddress::Trace(unsigned long long value) const
{
}

unsigned int StationAddress::GetSerializedSize() const
{
    return InetAddress::cSerializedSize;
}

}
}
}
