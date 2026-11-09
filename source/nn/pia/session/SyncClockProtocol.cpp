#include "nn/pia/session/SyncClockProtocol.hpp"

#include "nn/pia/Transport.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"

namespace nn
{
namespace pia
{
namespace session
{

namespace
{
const unsigned int cPayloadSize = 16;

inline unsigned int readU32(const unsigned char* buffer)
{
    return (static_cast<unsigned int>(buffer[0]) << 24) | (static_cast<unsigned int>(buffer[1]) << 16)
           | (static_cast<unsigned int>(buffer[2]) << 8) | static_cast<unsigned int>(buffer[3]);
}

inline unsigned long long readU64(const unsigned char* buffer)
{
    return (static_cast<unsigned long long>(readU32(buffer)) << 32) | readU32(buffer + 4);
}

inline void writeU32(unsigned char* buffer, unsigned int value)
{
    buffer[0] = static_cast<unsigned char>(value >> 24);
    buffer[1] = static_cast<unsigned char>(value >> 16);
    buffer[2] = static_cast<unsigned char>(value >> 8);
    buffer[3] = static_cast<unsigned char>(value);
}

inline void writeU64(unsigned char* buffer, unsigned long long value)
{
    writeU32(buffer, static_cast<unsigned int>(value >> 32));
    writeU32(buffer + 4, static_cast<unsigned int>(value));
}
}

SyncClockProtocol::Data::~Data()
{
}

Result SyncClockProtocol::Data::Serialize(unsigned char* buffer, unsigned int* size) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    writeU64(buffer, m_requestTime);
    writeU64(buffer + 8, m_syncTime);
    *size = cSerializedSize;
    return 0;
}

Result SyncClockProtocol::Data::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return cResultInvalidArgument;
    m_requestTime = readU64(buffer);
    m_syncTime = readU64(buffer + 8);
    return 0;
}

SyncClockProtocol::SyncClockProtocol()
    : transport::Protocol(),
      m_syncClock(),
      m_hostStationIndex(static_cast<StationIndex>(0xfd)),
      m_pulseInterval(2000),
      m_lastRequestTime(),
      m_hostConnected(false),
      m_requestSent(false),
      m_latencyMedian()
{
}

unsigned short SyncClockProtocol::GetProtocolType() const
{
    return cProtocolType;
}

Result SyncClockProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(address);
    if (station == 0)
        return 0x10c07;
    if (station->m_field10 < 0x20)
    {
        if (m_hostStationIndex != 0xfd)
            return 0x10c08;
        m_hostStationIndex = static_cast<StationIndex>(station->m_field10);
    }
    else
    {
        return 0x10c07;
    }
    return 0;
}

void SyncClockProtocol::CleanupWithStationAddress()
{
    m_hostStationIndex = static_cast<StationIndex>(0xfd);
    m_latencyMedian.m_count = 0;
    m_latencyMedian.m_index = 0;
}

Result SyncClockProtocol::Request()
{
    Mesh* mesh = g_piaMesh;
    if (mesh == 0)
        return 0x10c08;
    if (mesh->IsHost())
        return 0x10c08;
    transport::StationPacketHandler* handler = static_cast<transport::StationPacketHandler*>(mStationPacketHandler);
    if (handler == 0)
        return 0x10c08;

    const transport::ProtocolId protocolId = mProtocolId;
    common::Time now;
    now.SetNow();
    unsigned char payload[cPayloadSize];
    writeU64(payload, now.m_tick);
    writeU64(payload + 8, 0);

    const StationIndex hostIndex = mesh->GetHostStationIndex();
    if (hostIndex != 0xfd)
    {
        transport::ProtocolMessageWriter* writer = handler->AssignByStationIndex(protocolId, hostIndex, cPayloadSize,
                                                                                 false, false, true);
        if (writer != 0)
        {
            writer->SetPayload(payload);
            Result result = handler->Commit();
            if (result == 0)
                return 0;
            return result;
        }
    }
    return 0;
}

Result SyncClockProtocol::Response(unsigned long long stationId, unsigned long long requestTime)
{
    if (stationId == 0)
        return 0x10c07;
    transport::StationPacketHandler* handler = static_cast<transport::StationPacketHandler*>(mStationPacketHandler);
    if (handler == 0)
        return 0x10c08;
    if (m_syncClock.GetTime() == static_cast<unsigned long long>(-1))
        return 0x10c08;

    const transport::ProtocolId protocolId = mProtocolId;
    const unsigned long long syncTime = m_syncClock.GetTime();
    unsigned char payload[cPayloadSize];
    writeU64(payload, requestTime);
    writeU64(payload + 8, syncTime);
    transport::ProtocolMessageWriter* writer = handler->AssignByStationId(protocolId, stationId, cPayloadSize, false,
                                                                          false, true);
    if (writer == 0)
        return 0;
    writer->SetPayload(payload);
    return handler->Commit();
}

void SyncClockProtocol::ProcessByHost(const transport::ProtocolMessageReader* reader)
{
    const unsigned char* payload = reader->m_message + transport::ProtocolMessageReader::cMessageHeaderSize;
    Response(reader->m_header.m_source, readU64(payload));
}

void SyncClockProtocol::ProcessByClient(const transport::ProtocolMessageReader* reader)
{
    const unsigned char* payload = reader->m_message + transport::ProtocolMessageReader::cMessageHeaderSize;
    const unsigned long long requestTime = readU64(payload);
    const unsigned long long syncTime = readU64(payload + 8);
    const unsigned long long syncTicks = syncTime * common::TimeSpan::GetTicksPerMSec().m_tick;

    common::Time now;
    now.SetNow();
    const int latency = static_cast<int>(static_cast<long long>(now.m_tick - requestTime)
                                         / common::TimeSpan::GetTicksPerMSec().m_tick);
    m_latencyMedian.m_values[(m_latencyMedian.m_index + m_latencyMedian.m_count) % 10] = latency;
    if (m_latencyMedian.m_count == 10)
        m_latencyMedian.m_index = (m_latencyMedian.m_index + 1) % 10;
    else
        ++m_latencyMedian.m_count;

    const int halfLatency = m_latencyMedian.GetMedian() / 2;
    common::Time baseTime;
    baseTime.m_tick = now.m_tick - syncTicks - common::TimeSpan::GetTicksPerMSec().m_tick * halfLatency;
    m_syncClock.SetBaseTime(baseTime);
}

Result SyncClockProtocol::Dispatch()
{
    Mesh* mesh = g_piaMesh;
    if (mesh == 0)
        return 0x10c08;

    if (m_hostConnected && m_pulseInterval > 0)
    {
        if (!m_requestSent)
        {
            Request();
            m_lastRequestTime = transport::g_piaTransport->m_time;
            m_requestSent = true;
        }
        else
        {
            const long long elapsed = static_cast<long long>(transport::g_piaTransport->m_time.m_tick
                                                             - m_lastRequestTime.m_tick)
                                      / common::TimeSpan::GetTicksPerMSec().m_tick;
            if (m_pulseInterval < elapsed)
            {
                Request();
                m_lastRequestTime = transport::g_piaTransport->m_time;
            }
        }
    }

    transport::StationPacketHandler* handler = static_cast<transport::StationPacketHandler*>(mStationPacketHandler);
    if (handler == 0)
        return 0x10c08;

    transport::PacketHandler::Iterator& iterator = handler->GetIterator(mProtocolId);
    while (!handler->IsEndIteration())
    {
        const transport::ProtocolMessageReader* reader = iterator.GetMessageReader();
        if (mesh->IsHost())
            ProcessByHost(reader);
        else
            ProcessByClient(reader);
        handler->NextIteration();
    }
    return 0;
}

Result SyncClockProtocol::UpdateProtocolEvent(const transport::ProtocolEvent& event)
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(event.m_stationAddress);
    Mesh* mesh = g_piaMesh;
    const unsigned int stationIndex = station->m_field10;
    if (m_hostStationIndex == 0xfd)
        return 0x10c08;
    if (stationIndex >= 0x20)
        return 0x10c07;
    if (stationIndex == m_hostStationIndex)
        return 0x10c07;
    if (mesh == 0)
        return 0x10c08;
    if (mesh->IsHost())
        return 0;

    if (event.m_type == 0)
    {
        if (stationIndex == mesh->GetHostStationIndex())
        {
            m_requestSent = false;
            m_hostConnected = true;
        }
        return 0;
    }
    if (event.m_type == 1)
    {
        if (stationIndex == mesh->GetHostStationIndex())
            m_hostConnected = false;
        return 0;
    }
    return 0x10c07;
}

void SyncClockProtocol::Trace(unsigned long long) const
{
}

}
}
}
