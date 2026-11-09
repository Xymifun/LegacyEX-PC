#include "nn/pia/common/InetAddress.hpp"

#include "nn/pia/String.hpp"

namespace nn
{
namespace pia
{
namespace common
{

InetAddress::InetAddress() : m_address(0), m_port(0)
{
}

InetAddress::InetAddress(unsigned int address, unsigned short port) : m_address(address), m_port(port)
{
}

InetAddress::InetAddress(const InetAddress& other)
{
    *this = other;
}

InetAddress::~InetAddress()
{
}

void InetAddress::operator=(const InetAddress& other)
{
    m_address = other.m_address;
    m_port = other.m_port;
}

long long InetAddress::GetKey() const
{
    unsigned long long key = m_address;
    key <<= 32;
    key |= m_port;
    return key;
}

void InetAddress::GetAddressString(String* string) const
{
    const unsigned int address = m_address;
    const unsigned short port = m_port;
    string->Clear();
    string->Format("%d.%d.%d.%d:%d", address >> 24, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff,
                   port);
}

void InetAddress::Trace(unsigned long long value) const
{
}

Result InetAddress::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    const Result invalid = 0x10c07;
    if (buffer == 0)
        return invalid;
    if (size == 0)
        return invalid;
    if (maxSize < cSerializedSize)
        return invalid;

    serializeU32(buffer, m_address);
    serializeU16(buffer + 4, m_port);

    *size = cSerializedSize;
    return 0;
}

Result InetAddress::Deserialize(const unsigned char* buffer)
{
    const Result invalid = 0x10c07;
    if (buffer == 0)
        return invalid;

    m_address = deserializeU32(buffer);
    m_port = deserializeU16(buffer + 4);
    const Result success = 0;
    return success;
}

bool InetAddress::IsValid() const
{
    if (m_address == 0)
        return false;
    if (m_port == 0)
        return false;
    return true;
}

bool InetAddress::IsValidAddress() const
{
    return m_address != 0;
}

unsigned int InetAddress::IsPrivate() const
{
    const unsigned int first = m_address >> 24;
    const unsigned int second = (m_address >> 16) & 0xff;
    unsigned int result = 0;

    if (first == 10)
    {
        result = 1;
    }
    else if (first == 172)
    {
        if (second >= 16 && second <= 31)
            result = 1;
    }
    else if (first == 192)
    {
        if (second == 168)
            result = 1;
    }
    return result;
}

}
}
}
