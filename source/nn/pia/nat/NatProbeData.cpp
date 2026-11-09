#include "nn/pia/nat/Api.hpp"
#include "nn/pia/nat/NatProbeData.hpp"

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace nat
{

NatProbeData::NatProbeData() : m_connectionId(0), m_type(0), m_time()
{
}

Result NatProbeData::Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    if (maxSize < cProbeMessageSize)
        return cResultInvalidArgument;

    common::serializeU32(buffer, m_connectionId);
    unsigned char* p = buffer + sizeof(m_connectionId);
    p[0] = m_type;
    ++p;
    for (unsigned int i = 0; i < sizeof(m_extra); ++i)
    {
        *p = m_extra[i];
        ++p;
    }
    common::serializeU64(p, m_time.m_tick);
    *size = cProbeMessageSize;
    return 0;
}

Result NatProbeData::Deserialize(const unsigned char* buffer, unsigned int size)
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size < cProbeMessageSize)
        return cResultInvalidArgument;

    m_connectionId = common::deserializeU32(buffer);
    const unsigned char* p = buffer + sizeof(m_connectionId);
    m_type = p[0];
    ++p;
    for (unsigned int i = 0; i < sizeof(m_extra); ++i)
    {
        m_extra[i] = *p;
        ++p;
    }
    m_time.m_tick = common::deserializeU64(p);
    const Result success = 0;
    return success;
}

void NatProbeData::Trace(uint64_t) const
{
}

}
}
}
