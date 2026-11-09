#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

StationConnectionInfo::StationConnectionInfo() : m_stationLocation(), m_localStationLocation()
{
}

StationConnectionInfo::StationConnectionInfo(const StationConnectionInfo& other)
    : m_stationLocation(),
      m_localStationLocation()
{
    SetStationConnectionInfo(other);
}

StationConnectionInfo::~StationConnectionInfo()
{
}

void StationConnectionInfo::SetStationConnectionInfo(const StationConnectionInfo& other)
{
    m_stationLocation.SetStationLocation(other.m_stationLocation);
    m_localStationLocation.SetStationLocation(other.m_localStationLocation);
}

StationConnectionInfo& StationConnectionInfo::operator=(const StationConnectionInfo& other)
{
    if (this != &other)
        SetStationConnectionInfo(other);
    return *this;
}

bool StationConnectionInfo::operator==(const StationConnectionInfo& other) const
{
    if (m_stationLocation == other.m_stationLocation && m_localStationLocation == other.m_localStationLocation)
    {
        return true;
    }
    return false;
}

unsigned int StationConnectionInfo::GetSerializedSize() const
{
    return m_stationLocation.GetSerializedSize() + m_localStationLocation.GetSerializedSize();
}

Result StationConnectionInfo::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    if (buffer == 0)
        return ResultInvalidArgument();
    if (size == 0)
        return ResultInvalidArgument();
    if (maxSize < GetSerializedSize())
        return cResultWindowOverflow;

    unsigned char* cursor = buffer;
    unsigned int written;
    Result result;
    result = m_stationLocation.Serialize(cursor, &written, m_stationLocation.GetSerializedSize());
    cursor += m_stationLocation.GetSerializedSize();
    if (result != 0)
        return result;
    result = m_localStationLocation.Serialize(cursor, &written, m_localStationLocation.GetSerializedSize());
    cursor += m_localStationLocation.GetSerializedSize();
    if (result != 0)
        return result;
    *size = cursor - buffer;
    return 0;
}

Result StationConnectionInfo::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return ResultInvalidArgument();

    const unsigned char* cursor = buffer;
    Result result;
    result = m_stationLocation.Deserialize(cursor);
    cursor += m_stationLocation.GetSerializedSize();
    if (result != 0)
        return result;
    result = m_localStationLocation.Deserialize(cursor);
    cursor += m_localStationLocation.GetSerializedSize();
    if (result != 0)
        return result;
    return 0;
}

unsigned int StationConnectionInfo::GetStreamlinedSerializedSize() const
{
    return 0x60;
}

Result StationConnectionInfo::StreamlinedSerialize(unsigned char* buffer, unsigned int* size,
                                                   unsigned int maxSize) const
{
    if (buffer == 0)
        return ResultInvalidArgument();
    if (size == 0)
        return ResultInvalidArgument();
    if (maxSize < GetStreamlinedSerializedSize())
        return cResultWindowOverflow;

    unsigned char* cursor = buffer;
    unsigned int written;
    Result result;
    result = m_stationLocation.m_stationAddress.m_inetAddress.Serialize(cursor, &written,
                                                                        common::InetAddress::cSerializedSize);
    cursor += common::InetAddress::cSerializedSize;
    if (result != 0)
        return result;
    result = m_localStationLocation.m_stationAddress.m_inetAddress.Serialize(cursor, &written,
                                                                             common::InetAddress::cSerializedSize);
    cursor += common::InetAddress::cSerializedSize;
    if (result != 0)
        return result;
    written = LittleMembersSerialize(cursor, m_stationLocation);
    cursor += written;
    written = LittleMembersSerialize(cursor, m_localStationLocation);
    cursor += written;
    *size = cursor - buffer;
    return 0;
}

Result StationConnectionInfo::StreamlinedDeserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return ResultInvalidArgument();

    common::InetAddress address;
    const unsigned char* cursor = buffer;
    Result result;
    result = address.Deserialize(cursor);
    cursor += common::InetAddress::cSerializedSize;
    if (result != 0)
        return result;
    result = m_stationLocation.m_stationAddress.SetInetAddress(address);
    if (result != 0)
        return result;
    result = address.Deserialize(cursor);
    cursor += common::InetAddress::cSerializedSize;
    if (result != 0)
        return result;
    result = m_localStationLocation.m_stationAddress.SetInetAddress(address);
    if (result != 0)
        return result;
    cursor += LittleMembersDeserialize(&m_stationLocation, cursor);
    LittleMembersDeserialize(&m_localStationLocation, cursor);
    return 0;
}

void StationConnectionInfo::Trace(unsigned long long) const
{
}

unsigned int StationConnectionInfo::LittleMembersSerialize(unsigned char* buffer, const StationLocation& location) const
{
    unsigned char* cursor = buffer;
    common::serializeU64(cursor, location.m_principalId);
    cursor += 8;
    common::serializeU32(cursor, location.m_field25);
    cursor += 4;
    common::serializeU32(cursor, (location.m_field18 & 0x3fffffU) | ((location.m_field20 & 3U) << 22)
                                     | (location.m_field21 << 24));
    cursor += 4;
    common::serializeU32(cursor, (location.m_field1c & 0x3fffffU) | ((location.m_field23 & 3U) << 24)
                                     | ((location.m_field24 & 3U) << 26) | (location.m_field22 << 28));
    cursor += 4;
    return cursor - buffer;
}

unsigned int StationConnectionInfo::LittleMembersDeserialize(StationLocation* location, const unsigned char* buffer)
{
    const unsigned char* cursor = buffer;
    location->m_principalId = common::deserializeU64(cursor);
    cursor += 8;
    location->m_field25 = static_cast<unsigned char>(common::deserializeU32(cursor));
    cursor += 4;
    unsigned int value = common::deserializeU32(cursor);
    cursor += 4;
    location->m_field18 = value & 0x3fffffU;
    location->m_field20 = static_cast<unsigned char>((value >> 22) & 3U);
    location->m_field21 = static_cast<unsigned char>(value >> 24);
    value = common::deserializeU32(cursor);
    cursor += 4;
    location->m_field1c = value & 0x3fffffU;
    location->m_field23 = static_cast<unsigned char>((value >> 24) & 3U);
    location->m_field24 = static_cast<unsigned char>((value >> 26) & 3U);
    location->m_field22 = static_cast<unsigned char>(value >> 28);
    return cursor - buffer;
}

}
}
}
