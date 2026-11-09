#include "nn/pia/transport/RelayStationPacketHandler.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/common/Packet.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

const unsigned long long cStationIdUpperMask = 0xffffffff00000000ULL;

RelayStationPacketHandler::RelayStationPacketHandler() : m_relayRouteManager(0)
{
}

bool RelayStationPacketHandler::checkDerivedRuntimeTypeInfo(const pead::RuntimeTypeInfo::Interface* type) const
{
    return checkDerivedRuntimeTypeInfoStatic(type);
}

const pead::RuntimeTypeInfo::Interface* RelayStationPacketHandler::getRuntimeTypeInfo() const
{
    return getRuntimeTypeInfoStatic();
}

RelayStationPacketHandler::~RelayStationPacketHandler()
{
}

Result RelayStationPacketHandler::Initialize(unsigned int stationNum, bool isStarConnection,
                                             unsigned int maxProtocolNum)
{
    const Result result = StationPacketHandler::Initialize(stationNum, isStarConnection, maxProtocolNum);
    if (result != 0)
        return result;

    m_relayRouteManager = 0;
    return 0;
}

void RelayStationPacketHandler::Finalize()
{
    m_relayRouteManager = 0;
    StationPacketHandler::Finalize();
}

void RelayStationPacketHandler::Cleanup()
{
    m_relayRouteManager = 0;
    StationPacketHandler::Cleanup();
}

void RelayStationPacketHandler::Trace(unsigned long long value) const
{
}

Result RelayStationPacketHandler::GetRelayRoute(const RelayRouteManager* manager, ::nn::pia::StationIndex source,
                                                ::nn::pia::StationIndex destination, ::nn::pia::StationIndex* relay)
{
    return manager->GetRelayRoute(source, destination, relay);
}

Result RelayStationPacketHandler::GetDirectStationList(const RelayRouteManager* manager, ::nn::pia::StationIndex source,
                                                       unsigned int* directBitmap)
{
    return manager->GetDirectStationList(source, directBitmap);
}

Result RelayStationPacketHandler::GetDestStationList(const RelayRouteManager* manager, ::nn::pia::StationIndex source,
                                                     ::nn::pia::StationIndex relay, unsigned int* destinationBitmap)
{
    return manager->GetDestStationList(source, relay, destinationBitmap);
}

bool RelayStationPacketHandler::AssignRelayStationMessage(::nn::pia::StationIndex relayStation,
                                                          unsigned int stationBitmap, unsigned int size,
                                                          unsigned int sizeLimit, bool isPriority, bool field602,
                                                          bool field604)
{
    if (stationBitmap == 0)
        return true;
    if (!isPriority)
    {
        for (PacketStream::Accessor::Iterator it = m_writer->Begin(); it != m_writer->End(); ++it)
        {
            common::Packet* candidate = &*it;
            if (IsBundleCandidate(candidate, field602, sizeLimit, field604)
                && candidate->m_stationIndex == relayStation)
            {
                void* payload = AssignPacketPayload(candidate, size);
                m_protocolMessageWriter.AddMessageBuffer(candidate, payload, stationBitmap, true, true);
                m_keepAliveStationBitmap |= stationBitmap;
                return true;
            }
        }
    }
    const unsigned int relayBitmap = 1u << relayStation;
    common::Packet* packet;
    {
        common::StationAddress address;
        if (GetStationAddress(GetStationManager(), &address, relayStation) != 0)
            return false;
        packet = AssignPacket(relayStation, relayBitmap, address, isPriority, field602, field604);
        if (packet == 0)
            return false;
    }
    void* payload = AssignPacketPayload(packet, size);
    m_protocolMessageWriter.AddMessageBuffer(packet, payload, stationBitmap, true, true);
    m_keepAliveStationBitmap |= stationBitmap;
    return true;
}

ProtocolMessageWriter* RelayStationPacketHandler::AssignSingle(const ProtocolId& protocolId,
                                                               ::nn::pia::StationIndex stationIndex, unsigned int size,
                                                               bool isPriority, bool field602, bool field604)
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
    if (stationIndex == 0xfe)
    {
        AssignHostMessage(alignedSize, sizeLimit, isPriority, field602, field604);
    }
    else
    {
        ::nn::pia::StationIndex relay;
        if (static_cast<unsigned int>(stationIndex) >= 32 || m_localStationIndex == 0xfd)
            return 0;
        if (m_relayRouteManager != 0)
        {
            if (GetRelayRoute(m_relayRouteManager, static_cast< ::nn::pia::StationIndex>(m_localStationIndex),
                              stationIndex, &relay)
                != 0)
                return 0;
            if (relay == m_localStationIndex || relay == 0xfd)
                return 0;
            if (stationIndex != relay)
            {
                AssignRelayStationMessage(relay, 1u << stationIndex, alignedSize, sizeLimit, isPriority, field602,
                                          field604);
                return ReserveMessageWriter();
            }
        }
        AssignDirectMessage(1u << stationIndex, alignedSize, sizeLimit, isPriority, field602, field604);
    }
    return ReserveMessageWriter();
}

ProtocolMessageWriter* RelayStationPacketHandler::AssignAll(const ProtocolId& protocolId, unsigned int size,
                                                            bool isPriority, bool field602, bool field604)
{
    if (m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23u) & ~3u) + 4u;
    if (alignedSize > m_payloadSize)
        return 0;
    const unsigned int sizeLimit = m_packetSize - alignedSize;
    UpdateLocalStation();
    if (m_localStationIndex == 0xfd)
        return 0;
    const unsigned int participating = GetParticipatingStationBitmap(GetStationManager());
    if (participating == 0)
        return 0;

    struct
    {
        ::nn::pia::StationIndex relay;
        pead::BitFlag<unsigned int> stationBitmap;
    } destinations[32];
    int destinationNum = 0;
    unsigned int directBitmap;
    if (m_relayRouteManager != 0)
    {
        unsigned int directStations;
        if (GetDirectStationList(m_relayRouteManager, static_cast< ::nn::pia::StationIndex>(m_localStationIndex),
                                 &directStations)
            != 0)
            return 0;
        directBitmap = directStations & participating;
        unsigned int remaining = participating & ~directBitmap;
        if (remaining != 0)
        {
            for (unsigned int station = 0; station < 32; ++station)
            {
                if ((remaining & (1u << station)) != 0)
                {
                    ::nn::pia::StationIndex relay;
                    if (GetRelayRoute(m_relayRouteManager, static_cast< ::nn::pia::StationIndex>(m_localStationIndex),
                                      static_cast< ::nn::pia::StationIndex>(station), &relay)
                            == 0
                        && relay != m_localStationIndex && relay != 0xfd)
                    {
                        unsigned int relayedStations;
                        GetDestStationList(m_relayRouteManager,
                                           static_cast< ::nn::pia::StationIndex>(m_localStationIndex), relay,
                                           &relayedStations);
                        const unsigned int relayedBitmap = relayedStations & remaining;
                        destinations[destinationNum].stationBitmap = pead::BitFlag<unsigned int>(relayedBitmap);
                        remaining &= ~relayedBitmap;
                        destinations[destinationNum].relay = relay;
                        ++destinationNum;
                    }
                    else
                        remaining &= ~(1 << station);
                }
            }
        }
    }
    else
    {
        directBitmap = participating;
    }
    m_protocolMessageWriter.SetSource(m_localStationId);
    m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
    AssignAllMessage(directBitmap, alignedSize, sizeLimit, isPriority, field602, field604);
    for (int i = 0; i < destinationNum; ++i)
        AssignRelayStationMessage(destinations[i].relay, destinations[i].stationBitmap.getDirect(), alignedSize,
                                  sizeLimit, isPriority, field602, field604);
    return ReserveMessageWriter();
}

ProtocolMessageWriter* RelayStationPacketHandler::AssignMulti(const ProtocolId& protocolId, unsigned int stationBitmap,
                                                              unsigned int size, bool isPriority, bool field602,
                                                              bool field604)
{
    return AssignMulti(protocolId, stationBitmap, size, isPriority, field602, m_localStationId, false, field604);
}

ProtocolMessageWriter* RelayStationPacketHandler::AssignMulti(const ProtocolId& protocolId, unsigned int stationBitmap,
                                                              unsigned int size, bool isPriority, bool field602,
                                                              unsigned long long sourceId, bool isReliable,
                                                              bool field604)
{
    if (m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23u) & ~3u) + 4u;
    if (alignedSize > m_payloadSize)
        return 0;
    const unsigned int sizeLimit = m_packetSize - alignedSize;
    UpdateLocalStation();
    if (m_localStationIndex == 0xfd)
        return 0;

    struct
    {
        ::nn::pia::StationIndex relay;
        pead::BitFlag<unsigned int> stationBitmap;
    } destinations[32];
    int destinationNum = 0;
    unsigned int directBitmap = 0;
    if (m_relayRouteManager != 0)
    {
        for (unsigned int station = 0; station < 32; ++station)
        {
            if ((stationBitmap & (1u << station)) != 0)
            {
                ::nn::pia::StationIndex relay;
                if (GetRelayRoute(m_relayRouteManager, static_cast< ::nn::pia::StationIndex>(m_localStationIndex),
                                  static_cast< ::nn::pia::StationIndex>(station), &relay)
                        == 0
                    && relay != m_localStationIndex && relay != 0xfd)
                {
                    unsigned int relayedStations;
                    GetDestStationList(m_relayRouteManager, static_cast< ::nn::pia::StationIndex>(m_localStationIndex),
                                       relay, &relayedStations);
                    const unsigned int relayedBitmap = relayedStations & stationBitmap;
                    if (relayedBitmap == (1u << relay))
                    {
                        directBitmap |= 1 << relay;
                    }
                    else
                    {
                        destinations[destinationNum].relay = relay;
                        destinations[destinationNum].stationBitmap = pead::BitFlag<unsigned int>(relayedBitmap);
                        ++destinationNum;
                    }
                    stationBitmap &= ~relayedBitmap;
                }
                else
                    stationBitmap &= ~(1 << station);
            }
        }
    }
    else
    {
        directBitmap = stationBitmap;
    }
    m_protocolMessageWriter.SetSource(sourceId);
    m_protocolMessageWriter.Reset(protocolId, size, isReliable, isPriority);
    AssignDirectMessage(directBitmap, alignedSize, sizeLimit, isPriority, field602, field604);
    for (int i = 0; i < destinationNum; ++i)
        AssignRelayStationMessage(destinations[i].relay, destinations[i].stationBitmap.getDirect(), alignedSize,
                                  sizeLimit, isPriority, field602, field604);
    return ReserveMessageWriter();
}

void RelayStationPacketHandler::RelayMessage(const ProtocolMessageReader& reader)
{
    StationManager* manager = GetStationManager();
    const unsigned int destination = static_cast<unsigned int>(reader.GetDestination());
    const unsigned int stationBitmap = destination & GetParticipatingStationBitmap(manager);
    if (stationBitmap != 0)
    {
        const ProtocolId protocolId = reader.m_header.m_protocolId;
        ProtocolMessageWriter* writer = AssignMulti(protocolId, stationBitmap, reader.m_messageSize,
                                                    (reader.m_header.m_flags >> 3) & 1, !reader.m_field3a,
                                                    reader.m_header.m_source, true, true);
        if (writer != 0)
        {
            writer->SetPayload(reader.m_message + ProtocolMessageReader::cMessageHeaderSize);
            Commit();
        }
    }
}

unsigned int RelayStationPacketHandler::CheckMessage(const ProtocolMessageReader& reader)
{
    const unsigned char flags = reader.m_header.m_flags;
    if (((flags >> 1) & 1) != 0)
    {
        if ((flags & 1) == 0)
            return 0;
        if ((reader.GetDestination() & cStationIdUpperMask) != 0)
            return 0;
        const unsigned int stationBitmap = static_cast<unsigned int>(reader.GetDestination());
        if (m_localStationBitmap == 0)
            return 1;
        if ((stationBitmap & ~m_localStationBitmap) != 0)
            return 1;
        return 0;
    }
    if ((reader.m_header.m_flags & 1) != 0)
    {
        if ((reader.GetDestination() & cStationIdUpperMask) != 0)
            return 0;
        if (IsBroadcastSupported() == 0)
            return 1;
        if (m_localStationBitmap == 0)
            return 1;
        if (reader.GetDestination() == 0)
            return 1;
        if ((static_cast<unsigned int>(reader.GetDestination()) & m_localStationBitmap) != 0)
            return 1;
        return 0;
    }
    if (reader.GetDestination() == m_localStationId)
        return 1;
    return 0;
}

unsigned int RelayStationPacketHandler::CheckReceive(const ProtocolMessageReader& reader)
{
    if ((reader.m_header.m_flags & 1) != 0)
    {
        const unsigned int stationBitmap = static_cast<unsigned int>(reader.GetDestination());
        if (stationBitmap == 0)
        {
            if (((reader.m_header.m_flags >> 1) & 1) == 0)
                return 1;
        }
        else if ((stationBitmap & m_localStationBitmap) != 0)
            return 1;
    }
    else
    {
        if (((reader.m_header.m_flags >> 1) & 1) == 0 && reader.GetDestination() == m_localStationId)
            return 1;
    }
    return 0;
}

Result RelayStationPacketHandler::Startup(PacketStream::Writer* writer, PacketStream::Reader* reader,
                                          common::IPacketInput* packetInput, common::IPacketOutput* packetOutput,
                                          unsigned long long* stationId, const RelayRouteManager* relayRouteManager,
                                          const common::CryptoSetting* cryptoSetting)
{
    const Result result = StationPacketHandler::Startup(writer, reader, packetInput, packetOutput, stationId,
                                                        cryptoSetting);
    if (result != 0)
        return result;

    m_relayRouteManager = relayRouteManager;
    return 0;
}

}
}
}
