#include "nn/pia/transport/RttProtocol.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn
{
namespace pia
{
namespace transport
{

namespace
{

inline unsigned int readU32(const unsigned char* buffer)
{
    return (static_cast<unsigned int>(buffer[0]) << 24) | (static_cast<unsigned int>(buffer[1]) << 16)
           | (static_cast<unsigned int>(buffer[2]) << 8) | static_cast<unsigned int>(buffer[3]);
}

inline unsigned long long readU64(const unsigned char* buffer)
{
    return (static_cast<unsigned long long>(buffer[0]) << 56) | (static_cast<unsigned long long>(buffer[1]) << 48)
           | (static_cast<unsigned long long>(buffer[2]) << 40) | (static_cast<unsigned long long>(buffer[3]) << 32)
           | (static_cast<unsigned long long>(buffer[4]) << 24) | (static_cast<unsigned long long>(buffer[5]) << 16)
           | (static_cast<unsigned long long>(buffer[6]) << 8) | static_cast<unsigned long long>(buffer[7]);
}

inline void writeU32(unsigned char* buffer, unsigned int value)
{
    buffer[0] = static_cast<unsigned char>(value >> 24);
    buffer[1] = static_cast<unsigned char>((value >> 16) & 0xff);
    buffer[2] = static_cast<unsigned char>((value >> 8) & 0xff);
    buffer[3] = static_cast<unsigned char>(value);
}

inline void writeU64(unsigned char* buffer, unsigned long long value)
{
    buffer[0] = static_cast<unsigned char>(value >> 56);
    buffer[1] = static_cast<unsigned char>((value >> 48) & 0xff);
    buffer[2] = static_cast<unsigned char>((value >> 40) & 0xff);
    buffer[3] = static_cast<unsigned char>((value >> 32) & 0xff);
    buffer[4] = static_cast<unsigned char>((value >> 24) & 0xff);
    buffer[5] = static_cast<unsigned char>((value >> 16) & 0xff);
    buffer[6] = static_cast<unsigned char>((value >> 8) & 0xff);
    buffer[7] = static_cast<unsigned char>(value);
}

const unsigned int cDataSerializedSize = 16;

}

RttProtocol::RttProtocol()
    : Protocol(),
      m_stationIndex(cStationIndexInvalid),
      m_stationId(0),
      m_rttCalculators(0),
      m_stationCount(0),
      m_isBroadcastSupported(0)
{
}

RttProtocol::~RttProtocol()
{
}

Result RttProtocol::Initialize(unsigned short stationCount, int initialPulseInterval)
{
    if (m_rttCalculators != 0)
        return 0x10c02;
    if (stationCount == 0 || stationCount > 32)
        return 0x10c07;

    RttCalculator::SetInitialPulseInterval(initialPulseInterval);
    m_rttCalculators = common::NewArray<RttCalculator>(stationCount);
    if (m_rttCalculators == 0)
        return 0x10c01;
    m_stationCount = stationCount;
    m_isBroadcastSupported = 0;
    return 0;
}

void RttProtocol::Finalize()
{
    if (m_rttCalculators != 0)
    {
        m_isBroadcastSupported = 0;
        m_stationCount = 0;
        common::DeleteArray(m_rttCalculators);
        m_rttCalculators = 0;
    }
}

Result RttProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    if (m_rttCalculators == 0)
        return 0x10c08;
    Station* station = StationManager::s_pInstance->GetStation(address);
    if (station == 0)
        return 0x10c07;
    if (station->m_field10 >= 0x20)
        return 0x10c07;
    if (m_stationIndex != cStationIndexInvalid)
        return 0x10c08;

    m_stationIndex = station->m_field10;
    if (station->m_stationId == 0)
        return 0x10c07;
    if (m_stationId != 0)
        return 0x10c08;

    m_stationId = station->m_stationId;
    m_isBroadcastSupported = g_piaThreadStreamManager->m_packetOutput->IsBroadcastSupported();
    return 0;
}

void RttProtocol::CleanupWithStationAddress()
{
    m_stationIndex = cStationIndexInvalid;
    m_stationId = 0;
    m_isBroadcastSupported = 0;
    if (m_rttCalculators != 0)
    {
        for (unsigned int index = 0; index < m_stationCount; ++index)
            m_rttCalculators[index].Cleanup();
    }
}

Result RttProtocol::send(StationIndex stationIndex, const Data& data)
{
    StationPacketHandler* handler = static_cast<StationPacketHandler*>(GetStationPacketHandler());
    if (handler == 0)
        return 0x10c08;
    if (stationIndex == m_stationIndex)
        return 0x10c08;

    unsigned int size = 0;
    data.Serialize(m_data, &size);
    ProtocolMessageWriter* writer = handler->AssignByStationIndex(GetProtocolId(), stationIndex, size, false, false,
                                                                  true);
    if (writer == 0)
        return 0;
    writer->SetPayload(m_data);
    Result result = handler->Commit();
    if (result != 0)
        return result;
    return 0;
}

Result RttProtocol::request(StationIndex stationIndex)
{
    if ((stationIndex < 0 || stationIndex >= m_stationCount) && stationIndex != cStationIndexSentinelFE)
        return 0x10c07;
    if (m_stationIndex == cStationIndexInvalid)
        return 0x10c08;

    common::Time now;
    Data data;
    data.m_type = 0;
    data.m_sequence = 0;
    now.SetNow();
    data.m_timestamp = now.m_tick;
    if (m_isBroadcastSupported)
        return send(cStationIndexSentinelFF, data);
    return send(stationIndex, data);
}

void RttProtocol::UpdateLastRequestedTimeAll()
{
    for (unsigned int index = 0; index < m_stationCount; ++index)
    {
        RttCalculator* calculator = m_rttCalculators + index;
        if (*reinterpret_cast<const unsigned char*>(calculator) != 0)
            calculator->SetLastRequestedTime(g_piaTransport->m_time);
    }
}

void RttProtocol::process(const ProtocolMessageReader* reader)
{
    Data data;
    data.Deserialize(reader->m_message + ProtocolMessageReader::cMessageHeaderSize);
    Station* station = StationManager::s_pInstance->GetStationByStationIdWithoutConvert(reader->m_header.m_source);
    if (station == 0)
        return;
    const StationIndex stationIndex = static_cast<StationIndex>(station->m_field10);
    if (stationIndex == cStationIndexInvalid)
        return;

    if (data.m_type == 0)
    {
        Data response;
        response.m_type = 1;
        response.m_sequence = 0;
        response.m_timestamp = data.m_timestamp;
        send(stationIndex, response);
    }
    else if (data.m_type == 1)
    {
        common::Time now;
        now.SetNow();
        const int rtt = static_cast<int>(static_cast<long long>(now.m_tick - data.m_timestamp)
                                         / common::TimeSpan::GetTicksPerMSec().m_tick);
        if (stationIndex >= 0 && stationIndex < m_stationCount)
            m_rttCalculators[stationIndex].Update(rtt);
    }
}

Result RttProtocol::Dispatch()
{
    if (m_stationIndex == cStationIndexInvalid)
        return 0;
    StationPacketHandler* handler = static_cast<StationPacketHandler*>(mStationPacketHandler);
    if (handler == 0)
        return 0x10c08;
    for (unsigned int index = 0; index < m_stationCount; ++index)
    {
        RttCalculator* calculator = m_rttCalculators + index;
        if (*reinterpret_cast<const unsigned char*>(calculator) != 0 && calculator->IsTimeOut())
        {
            request(static_cast<StationIndex>(index));
            if (m_isBroadcastSupported)
            {
                UpdateLastRequestedTimeAll();
                break;
            }
            calculator->SetLastRequestedTime(g_piaTransport->m_time);
        }
    }

    PacketHandler::Iterator& iterator = handler->GetIterator(GetProtocolId());
    while (!iterator.IsEndIteration())
    {
        ProtocolMessageReader* reader = iterator.GetMessageReader();
        if (reader->m_header.m_source == m_stationId)
            reader->Trace(0x40000);
        else
            process(reader);
        iterator.NextIteration();
    }
    return 0;
}

Result RttProtocol::UpdateProtocolEvent(const ProtocolEvent& event)
{
    if (m_stationIndex == cStationIndexInvalid)
        return 0x10c08;
    const StationIndex stationIndex = static_cast<StationIndex>(
        StationManager::s_pInstance->GetStation(event.m_stationAddress)->m_field10);
    if (stationIndex < 0 || m_stationCount <= stationIndex)
        return 0x10c07;
    if (stationIndex == m_stationIndex)
        return 0x10c07;

    switch (event.m_type)
    {
    case 0:
        m_rttCalculators[stationIndex].Startup();
        request(stationIndex);
        if (m_isBroadcastSupported)
            UpdateLastRequestedTimeAll();
        else
            m_rttCalculators[stationIndex].SetLastRequestedTime(g_piaTransport->m_time);
        return 0;
    case 1:
        m_rttCalculators[stationIndex].Cleanup();
        return 0;
    default:
        return 0x10c07;
    }
}

int RttProtocol::GetRtt(StationIndex stationIndex) const
{
    if (stationIndex >= 0 && stationIndex < m_stationCount)
        return m_rttCalculators[stationIndex].GetRtt();
    return -1;
}

int RttProtocol::GetRtt(StationIndex stationIndex, unsigned int count) const
{
    if (stationIndex >= 0 && stationIndex < m_stationCount)
        return m_rttCalculators[stationIndex].GetRtt(count);
    return -1;
}

void RttProtocol::Trace(unsigned long long) const
{
}

void RttProtocol::ResetRttMinMax(StationIndex stationIndex)
{
    if (stationIndex < 0 || static_cast<int>(stationIndex) >= static_cast<int>(m_stationCount))
        return;
    m_rttCalculators[stationIndex].ResetRttMinMax();
}

int RttProtocol::GetRttMin(StationIndex stationIndex) const
{
    if (stationIndex >= 0 && stationIndex < m_stationCount)
        return m_rttCalculators[stationIndex].GetRttMin();
    return -1;
}

int RttProtocol::GetRttMax(StationIndex stationIndex) const
{
    if (stationIndex >= 0 && stationIndex < m_stationCount)
        return m_rttCalculators[stationIndex].GetRttMax();
    return -1;
}

unsigned short RttProtocol::GetProtocolType() const
{
    return cProtocolType;
}

Result RttProtocol::Data::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;
    const unsigned char* cursor = buffer;
    m_type = readU32(cursor);
    cursor += 4;
    m_sequence = readU32(cursor);
    cursor += 4;
    m_timestamp = readU64(cursor);
    return 0;
}

Result RttProtocol::Data::Serialize(unsigned char* buffer, unsigned int* size) const
{
    if (buffer == 0)
        return 0x10c07;
    if (size == 0)
        return 0x10c07;
    unsigned char* cursor = buffer;
    writeU32(cursor, m_type);
    cursor += 4;
    writeU32(cursor, m_sequence);
    cursor += 4;
    writeU64(cursor, m_timestamp);
    cursor += 8;
    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

unsigned int RttProtocol::Data::GetSerializedSize() const
{
    return cDataSerializedSize;
}

}
}
}
