#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/Assert.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace transport {

const unsigned long long cStationIdUpperMask = 0xffffffff00000000ULL;
const unsigned char cKeepAliveProtocolType = 8;

StationPacketHandler::StationPacketHandler()
{
}

StationPacketHandler::~StationPacketHandler()
{
}

Result StationPacketHandler::Initialize(unsigned int stationNum, bool isStarConnection,
                                       unsigned int maxProtocolNum)
{
    if (stationNum == 0)
        return 0x10c07;
    const Result result = PacketHandler::Initialize(maxProtocolNum);
    if (result != 0)
        return result;

    m_isStarConnection = isStarConnection;
    m_localStationIndex = ::nn::pia::cStationIndexInvalid;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    m_stationNum = stationNum;
    return 0;
}

void StationPacketHandler::Finalize()
{
    m_localStationIndex = ::nn::pia::cStationIndexInvalid;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    PacketHandler::Finalize();
}

Result StationPacketHandler::Startup(PacketStream::Writer* writer,
                                     PacketStream::Reader* reader,
                                     common::IPacketInput* packetInput,
                                     common::IPacketOutput* packetOutput,
                                     unsigned long long* stationId,
                                     const common::CryptoSetting* cryptoSetting)
{
    const Result result = PacketHandler::Startup(writer, reader, packetInput, packetOutput,
                                                 stationId, cryptoSetting);
    if (result != 0)
        return result;

    m_localStationIndex = ::nn::pia::cStationIndexInvalid;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    m_protocolMessageWriter.SetSource(0);
    m_keepAliveStationBitmap = 0;
    return 0;
}

void StationPacketHandler::Cleanup()
{
    m_localStationIndex = ::nn::pia::cStationIndexInvalid;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    PacketHandler::Cleanup();
}

StationManager* StationPacketHandler::GetStationManager()
{
    PIA_ASSERT(StationManager::s_pInstance != 0);
    return StationManager::s_pInstance;
}

Station* StationPacketHandler::GetStation(StationManager* manager,
                                          const common::StationAddress& address)
{
    return manager->GetStation(address);
}

::nn::pia::StationIndex StationPacketHandler::GetStationIndex(const Station* station)
{
    return station->GetStationIndex();
}

Station* StationPacketHandler::GetLocalStation(StationManager* manager)
{
    return manager->m_localStation;
}

unsigned long long StationPacketHandler::GetStationId(const Station* station)
{
    return station->m_stationId;
}

void StationPacketHandler::UpdateLocalStation()
{
    if (m_localStationIndex < 32)
        return;
    const Station* station = GetLocalStation(GetStationManager());
    if (station == 0)
        return;
    m_localStationIndex = GetStationIndex(station);
    if (m_localStationIndex < 32)
    {
        m_localStationId = GetStationId(station);
        m_localStationBitmap = 1u << m_localStationIndex;
        m_protocolMessageWriter.SetSource(m_localStationId);
    }
}

void StationPacketHandler::BeginDispatch(const common::Time& time)
{
    UpdateLocalStation();
    PacketHandler::BeginDispatch(time);
    StationManager* manager = GetStationManager();
    unsigned int stationBitmap = 0;
    PacketStream::Reader* reader = m_reader;
    for (PacketStream::Accessor::Iterator it = reader->Begin(); it != reader->End(); ++it)
    {
        const common::Packet* packet = &*it;
        if (packet->m_field614 != 0)
        {
            if (packet->m_packetData.m_header.m_passId != 1)
            {
                const Station* station = GetStation(manager, packet->m_field608);
                if (station != 0)
                {
                    const unsigned int stationIndex = GetStationIndex(station);
                    if (stationIndex < 32)
                        stationBitmap |= 1u << stationIndex;
                }
            }
            unsigned int offset = 0;
            while (offset < packet->m_size - common::Packet::cHeaderSize)
            {
                m_protocolMessageReader.Attach(*packet, offset);
                if (m_protocolMessageReader.m_message == 0)
                    break;
                if (m_protocolMessageReader.m_header.m_source != 0)
                {
                    const Station* station = manager->GetStation(m_protocolMessageReader.m_header.m_source);
                    if (station != 0)
                        stationBitmap |= 1u << station->GetStationIndex();
                }
                offset += ((m_protocolMessageReader.m_messageSize + 23u) & ~3u) + 4u;
            }
        }
    }
    m_keepAliveReceiver.Update(stationBitmap, time);
}

unsigned int StationPacketHandler::GetParticipatingStationBitmap(const StationManager* manager)
{
    return manager->GetParticipatingStationBitmap(false);
}

ProtocolMessageWriter* StationPacketHandler::AssignByStationBitmap(
    const ProtocolId& protocolId, unsigned int stationBitmap, unsigned int size,
    bool isReliable, bool isPriority, bool isBroadcast)
{
    const unsigned int participating = GetParticipatingStationBitmap(GetStationManager());
    return AssignMulti(protocolId, stationBitmap & participating, size,
                       isReliable, isPriority, isBroadcast);
}

void StationPacketHandler::EndDispatch(const common::Time& time)
{
    const unsigned int stationBitmap = m_keepAliveSender.Update(m_keepAliveStationBitmap, time);
    m_keepAliveStationBitmap = 0;
    if (stationBitmap != 0)
    {
        const ProtocolId protocolId(cKeepAliveProtocolType, 0);
        if (AssignByStationBitmap(protocolId, stationBitmap, 0, false, false, true) != 0)
        {
            Commit();
            if (m_keepAliveStationBitmap != 0)
            {
                m_keepAliveSender.Update(m_keepAliveStationBitmap, time);
                m_keepAliveStationBitmap = 0;
            }
        }
    }
    PacketHandler::EndDispatch(time);
}

ProtocolMessageWriter* StationPacketHandler::AssignByStationIndex(
    const ProtocolId& protocolId, ::nn::pia::StationIndex stationIndex,
    unsigned int size, bool isReliable, bool isPriority, bool isBroadcast)
{
    ProtocolMessageWriter* writer;
    if (stationIndex == ::nn::pia::cStationIndexSentinelFF)
    {
        if (IsBroadcastSupported() != 0)
            writer = AssignAll(protocolId, size, isReliable, isPriority, isBroadcast);
        else
            writer = AssignMulti(protocolId, GetParticipatingStationBitmap(GetStationManager()),
                                 size, isReliable, isPriority, isBroadcast);
    }
    else
        writer = AssignSingle(protocolId, stationIndex, size, isReliable, isPriority, isBroadcast);
    return writer;
}

ProtocolMessageWriter* StationPacketHandler::AssignByStationId(
    const ProtocolId& protocolId, unsigned long long stationId,
    unsigned int size, bool isReliable, bool isPriority, bool isBroadcast)
{
    Station* station = StationManager::s_pInstance->GetStationByStationIdWithoutConvert(stationId);
    if (station == 0 || station->GetStationIndex() == ::nn::pia::cStationIndexInvalid)
        return 0;
    return AssignByStationIndex(protocolId, station->GetStationIndex(),
                                size, isReliable, isPriority, isBroadcast);
}

unsigned int StationPacketHandler::GenerateConnectionId() const
{
    common::Time time;
    time.SetNow();
    return static_cast<unsigned int>(time.m_tick);
}

void StationPacketHandler::Trace(unsigned long long value) const
{
}

StationConnectionInfoTable* StationPacketHandler::GetStationConnectionInfoTable()
{
    PIA_ASSERT(g_stationConnectionInfoTable != 0);
    return g_stationConnectionInfoTable;
}

const common::StationAddress& StationPacketHandler::GetHostStationAddress(
    const StationConnectionInfoTable* table)
{
    return table->m_hostStationAddress;
}

Station* StationPacketHandler::GetStation(StationManager* manager,
                                          ::nn::pia::StationIndex stationIndex)
{
    return manager->GetStation(stationIndex);
}

unsigned char StationPacketHandler::GetSendingPassId(const Station* station)
{
    return station->m_field54;
}

unsigned short StationPacketHandler::GetNextSendSequenceId(Station* station)
{
    return station->m_sequenceIdController.GetNextSendSequenceId();
}

common::Packet* StationPacketHandler::AssignPacket(
    ::nn::pia::StationIndex stationIndex, unsigned int stationBitmap,
    const common::StationAddress& address, bool isPriority, bool field602, bool field604)
{
    Station* station = 0;
    unsigned char sendingPassId = 0;
    if (m_localStationIndex < 32)
    {
        if (static_cast<unsigned int>(stationIndex) < 32)
        {
            station = GetStation(GetStationManager(), stationIndex);
            if (station != 0 && field604)
            {
                sendingPassId = GetSendingPassId(station);
                if (sendingPassId == 0)
                {
                    station->Trace(0x8000);
                    return 0;
                }
            }
        }
    }
    else
        sendingPassId = 1;

    common::Packet* packet = PacketHandler::AssignPacket(address, isPriority, field602);
    if (packet == 0)
        return 0;
    const unsigned short sequenceId = station != 0 ?
        GetNextSendSequenceId(station) : 0;
    packet->m_stationBitmap = stationBitmap;
    packet->m_packetData.m_header.m_passId = sendingPassId;
    packet->m_packetData.m_header.m_sequenceId = sequenceId;
    packet->m_stationIndex = stationIndex;
    m_keepAliveStationBitmap |= stationBitmap;
    return packet;
}

bool StationPacketHandler::AssignHostMessage(unsigned int size, unsigned int sizeLimit,
                                              bool isPriority, bool field602, bool field604)
{
    if (!isPriority)
    {
        PacketStream::Writer* writer = m_writer;
        for (PacketStream::Accessor::Iterator it = writer->Begin(); it != writer->End(); ++it)
        {
            common::Packet* candidate = &*it;
            if (IsBundleCandidate(candidate, field602, sizeLimit, field604) &&
                candidate->m_stationIndex == ::nn::pia::cStationIndexSentinelFE)
            {
                void* payload = AssignPacketPayload(candidate, size);
                m_protocolMessageWriter.AddMessageBuffer(candidate, payload, 0, true, false);
                return true;
            }
        }
    }
    common::Packet* packet;
    {
        const common::StationAddress address(GetHostStationAddress(GetStationConnectionInfoTable()));
        packet = AssignPacket(::nn::pia::cStationIndexSentinelFE, 0, address, isPriority,
                              field602, field604);
        if (packet == 0)
            return false;
    }
    void* payload = AssignPacketPayload(packet, size);
    m_protocolMessageWriter.AddMessageBuffer(packet, payload, 0, true, false);
    return true;
}

Result StationPacketHandler::GetStationAddress(const StationManager* manager,
                                               common::StationAddress* address,
                                               ::nn::pia::StationIndex stationIndex)
{
    return manager->GetStationAddress(address, stationIndex);
}

bool StationPacketHandler::AssignDirectMessage(unsigned int stationBitmap, unsigned int size,
                                                unsigned int sizeLimit, bool isPriority,
                                                bool field602, bool field604)
{
    if (stationBitmap == 0)
        return true;
    StationManager* manager = GetStationManager();
    PacketStream::Writer* writer = m_writer;
    unsigned int index = static_cast<unsigned int>(isPriority ? writer->m_field08 : writer->m_field04);
    while (stationBitmap != 0)
    {
        if (index == static_cast<unsigned int>(writer->m_field08))
        {
            const int bit = pead::BitFlagUtil::findOnBitFromRight(stationBitmap, m_stationNum);
            const unsigned int selected = bit < 0 ? stationBitmap :
                stationBitmap & ((1u << (bit + 1)) - 1u);
            stationBitmap &= ~selected;
            const int selectedNum = pead::BitFlagUtil::countOnBit(selected);
            common::Packet* packet;
            {
                common::StationAddress address;
                ::nn::pia::StationIndex stationIndex;
                if (selectedNum == 1)
                {
                    stationIndex = static_cast< ::nn::pia::StationIndex>(
                        pead::BitFlagUtil::countOnBit((selected & (0u - selected)) - 1u));
                    if (GetStationAddress(manager, &address, stationIndex) != 0)
                        continue;
                }
                else
                {
                    stationIndex = ::nn::pia::cStationIndexInvalid;
                    address.Clear();
                }
                packet = AssignPacket(stationIndex, selected, address, isPriority, field602,
                                      field604);
                if (packet == 0)
                    return false;
                ++index;
                if (static_cast<int>(index) >= writer->m_packetStream->m_packets.size())
                    index = 0;
            }
            void* payload = AssignPacketPayload(packet, size);
            m_protocolMessageWriter.AddMessageBuffer(packet, payload, selected, true, false);
        }
        else
        {
            PacketStream* stream = writer->m_packetStream;
            common::Packet* packet = stream->m_packets.get(index);
            ++index;
            if (static_cast<int>(index) >= stream->m_packets.size())
                index = 0;
            if (!IsBundleCandidate(packet, field602, sizeLimit, field604))
                continue;
            const unsigned int packetBitmap = packet->m_stationBitmap;
            if (packetBitmap == 0 || (packetBitmap & stationBitmap) != packetBitmap)
                continue;
            stationBitmap &= ~packetBitmap;
            void* payload = AssignPacketPayload(packet, size);
            m_protocolMessageWriter.AddMessageBuffer(packet, payload, packetBitmap, true, false);
        }
    }
    return true;
}

ProtocolMessageWriter* StationPacketHandler::AssignSingle(
    const ProtocolId& protocolId, ::nn::pia::StationIndex stationIndex,
    unsigned int size, bool isPriority, bool field602, bool field604)
{
    if (m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23u) & ~3u) + 4u;
    if (alignedSize > m_payloadSize)
        return 0;
    const unsigned int sizeLimit = m_packetSize - alignedSize;
    UpdateLocalStation();
    m_protocolMessageWriter.SetSource(m_localStationId);
    m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
    if (stationIndex == ::nn::pia::cStationIndexSentinelFE)
        AssignHostMessage(alignedSize, sizeLimit, isPriority, field602, field604);
    else if (static_cast<unsigned int>(stationIndex) < 32 && m_localStationIndex != ::nn::pia::cStationIndexInvalid)
        AssignDirectMessage(1u << stationIndex, alignedSize, sizeLimit, isPriority, field602, field604);
    else
        return 0;
    return ReserveMessageWriter();
}

bool StationPacketHandler::AssignAllMessage(unsigned int stationBitmap, unsigned int size,
                                             unsigned int sizeLimit, bool isPriority,
                                             bool field602, bool field604)
{
    if (stationBitmap == 0)
        return true;
    if (!isPriority)
    {
        PacketStream::Writer* writer = m_writer;
        for (PacketStream::Accessor::Iterator it = writer->Begin(); it != writer->End(); ++it)
        {
            common::Packet* candidate = &*it;
            if (IsBundleCandidate(candidate, field602, sizeLimit, field604) &&
                candidate->m_stationIndex == ::nn::pia::cStationIndexSentinelFF)
            {
                void* payload = AssignPacketPayload(candidate, size);
                m_protocolMessageWriter.AddMessageBuffer(candidate, payload, stationBitmap, true, false);
                return true;
            }
        }
    }
    common::Packet* packet;
    {
        common::StationAddress address;
        packet = AssignPacket(::nn::pia::cStationIndexSentinelFF, stationBitmap, address,
                              isPriority, field602, field604);
    }
    if (packet == 0)
        return false;
    void* payload = AssignPacketPayload(packet, size);
    m_protocolMessageWriter.AddMessageBuffer(packet, payload, stationBitmap, true, false);
    return true;
}

ProtocolMessageWriter* StationPacketHandler::AssignAll(
    const ProtocolId& protocolId, unsigned int size, bool isPriority, bool field602, bool field604)
{
    if (m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23u) & ~3u) + 4u;
    if (alignedSize > m_payloadSize)
        return 0;
    const unsigned int sizeLimit = m_packetSize - alignedSize;
    UpdateLocalStation();
    if (m_localStationIndex == ::nn::pia::cStationIndexInvalid)
        return 0;
    const unsigned int stationBitmap = GetParticipatingStationBitmap(GetStationManager());
    if (stationBitmap == 0)
        return 0;
    m_protocolMessageWriter.SetSource(m_localStationId);
    m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
    AssignAllMessage(stationBitmap, alignedSize, sizeLimit, isPriority, field602, field604);
    return ReserveMessageWriter();
}

ProtocolMessageWriter* StationPacketHandler::AssignMulti(
    const ProtocolId& protocolId, unsigned int stationBitmap, unsigned int size,
    bool isPriority, bool field602, unsigned long long sourceId, bool field604)
{
    if (m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23u) & ~3u) + 4u;
    if (alignedSize > m_payloadSize)
        return 0;
    const unsigned int sizeLimit = m_packetSize - alignedSize;
    UpdateLocalStation();
    if (m_localStationIndex == ::nn::pia::cStationIndexInvalid)
        return 0;
    m_protocolMessageWriter.SetSource(sourceId);
    m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
    AssignDirectMessage(stationBitmap, alignedSize, sizeLimit, isPriority, field602, field604);
    return ReserveMessageWriter();
}

ProtocolMessageWriter* StationPacketHandler::AssignMulti(
    const ProtocolId& protocolId, unsigned int stationBitmap, unsigned int size,
    bool isPriority, bool field602, bool field604)
{
    return AssignMulti(protocolId, stationBitmap, size, isPriority, field602,
                       m_localStationId, field604);
}

unsigned char StationPacketHandler::GetReceivingPassId(const Station* station)
{
    return station->m_field55;
}

bool StationPacketHandler::CheckReceivedSequenceId(Station* station, unsigned short sequenceId)
{
    return station->m_sequenceIdController.CheckReceivedSequenceId(sequenceId);
}

unsigned int StationPacketHandler::CheckPacket(const common::Packet& packet)
{
    StationManager* manager = GetStationManager();
    Station* station = GetStation(manager, packet.m_field608);
    if (station == 0)
        return packet.m_size;

    const unsigned char passId = packet.m_packetData.m_header.m_passId;
    const unsigned char receivingPassId = GetReceivingPassId(station);
    if (passId != 0 && passId != 1 && receivingPassId != 0 && passId != receivingPassId)
        return 0;
    if (m_localStationIndex < 32 &&
        !CheckReceivedSequenceId(station, packet.m_packetData.m_header.m_sequenceId))
        return 0;
    if (m_localStationIndex != static_cast<unsigned int>(manager->m_invalidStationIndex) &&
        station->m_field20 == 5 && station->GetStationIndex() == ::nn::pia::cStationIndexInvalid)
        return 0;
    return packet.m_size;
}

unsigned int StationPacketHandler::CheckMessage(const ProtocolMessageReader& reader)
{
    if (((reader.m_header.m_flags >> 1) & 1) != 0)
        return 0;
    if ((reader.m_header.m_flags & 1) != 0)
    {
        if ((reader.GetDestination() & cStationIdUpperMask) != 0)
            return 0;
        if (IsBroadcastSupported() == 0 || m_localStationBitmap == 0 ||
            reader.GetDestination() == 0 ||
            (static_cast<unsigned int>(reader.GetDestination()) & m_localStationBitmap) != 0)
            return 1;
        return 0;
    }
    if (reader.GetDestination() == m_localStationId)
        return 1;
    return 0;
}

unsigned int StationPacketHandler::CheckReceive(const ProtocolMessageReader& reader)
{
    if (((reader.m_header.m_flags >> 1) & 1) != 0)
        return 0;
    if ((reader.m_header.m_flags & 1) != 0)
    {
        const unsigned int stationBitmap = static_cast<unsigned int>(reader.GetDestination());
        if (stationBitmap == 0)
            return 1;
        if ((stationBitmap & m_localStationBitmap) != 0)
            return 1;
        return 0;
    }
    if (reader.GetDestination() == m_localStationId)
        return 1;
    return 0;
}

bool StationPacketHandler::checkDerivedRuntimeTypeInfo(const pead::RuntimeTypeInfo::Interface* type) const
{
    return checkDerivedRuntimeTypeInfoStatic(type);
}

const pead::RuntimeTypeInfo::Interface* StationPacketHandler::getRuntimeTypeInfo() const
{
    return getRuntimeTypeInfoStatic();
}

unsigned int StationPacketHandler::IsBroadcastSupported() const
{
    return m_isStarConnection;
}

}}}
