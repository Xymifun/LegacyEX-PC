#include "nn/pia/transport/PacketAnalysisData.hpp"
#include "nn/pia/transport/AnalysisPrinter.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/inet/MonitoringDataProtocol.hpp"
#include "nn/pia/lan/LanProtocol.hpp"
#include "nn/pia/nat/NatTraversalProtocol.hpp"
#include "nn/pia/session/MeshProtocol.hpp"
#include "nn/pia/session/SessionProtocol.hpp"
#include "nn/pia/session/SyncClockProtocol.hpp"
#include "nn/pia/transport/BandwidthCheckProtocol.hpp"
#include "nn/pia/transport/RttProtocol.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/UnreliableProtocol.hpp"
#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

PacketAnalysisData::Entry::Entry() : m_protocolId(ProtocolId::INVALID)
{
}

void PacketAnalysisData::ClearExceptName()
{
    for (int i = 0; i < cEntryNumMax; ++i)
    {
        m_entries[i].m_protocolId = ProtocolId::INVALID;
        m_entries[i].m_packetNum = 0;
        m_entries[i].m_packetSize = 0;
        m_entries[i].m_totalPacketNum = 0;
        m_entries[i].m_totalPacketSize = 0;
    }
    m_entryNum = 0;
    m_passedMilliSecond = 0;
    m_packetNum = 0;
    m_packetSize = 0;
    m_totalPacketNum = 0;
    m_totalPacketSize = 0;
}

void PacketAnalysisData::ClearCounters()
{
    for (int i = 0; i < m_entryNum; ++i)
    {
        m_entries[i].m_packetNum = 0;
        m_entries[i].m_packetSize = 0;
    }
    m_passedMilliSecond = 0;
    m_packetNum = 0;
    m_packetSize = 0;
}

}
}
}

namespace
{
bool isInternalProtocol(nn::pia::transport::ProtocolId protocolId)
{
    switch ((unsigned short)(protocolId >> 8))
    {
    case 8:
    case nn::pia::transport::StationProtocol::cProtocolType:
    case nn::pia::session::MeshProtocol::cProtocolType:
    case nn::pia::session::SyncClockProtocol::cProtocolType:
    case 0x24:
    case 0x28:
    case 0x2c:
    case nn::pia::nat::NatTraversalProtocol::cProtocolType:
    case nn::pia::lan::LanProtocol::cProtocolType:
    case nn::pia::transport::BandwidthCheckProtocol::cProtocolType:
    case nn::pia::transport::RttProtocol::cProtocolType:
    case nn::pia::session::SessionProtocol::cProtocolType:
    case nn::pia::inet::MonitoringDataProtocol::cProtocolType:
    case 0xa8:
        return true;
    default:
        return false;
    }
}

const char* getProtocolName(nn::pia::transport::ProtocolId protocolId)
{
    switch ((unsigned short)(protocolId >> 8))
    {
    case 8:
        return "KeepAlive";
    case nn::pia::transport::StationProtocol::cProtocolType:
        return "Station";
    case nn::pia::session::MeshProtocol::cProtocolType:
        return "Mesh";
    case nn::pia::session::SyncClockProtocol::cProtocolType:
        return "SyncClock";
    case 0x24:
        return "Local";
    case 0x28:
        return "Direct";
    case 0x2c:
        return "Net";
    case nn::pia::nat::NatTraversalProtocol::cProtocolType:
        return "Nat";
    case nn::pia::lan::LanProtocol::cProtocolType:
        return "Lan";
    case nn::pia::transport::BandwidthCheckProtocol::cProtocolType:
        return "BandwidthCheck";
    case nn::pia::transport::RttProtocol::cProtocolType:
        return "Rtt";
    case 0x64:
        return "Sync";
    case nn::pia::transport::UnreliableProtocol::cProtocolType:
        return "Unreliable";
    case 0x6c:
        return "RoundrobinUnreliable";
    case 0x74:
        return "Clone";
    case nn::pia::chat::VoiceProtocol::cProtocolType:
        return "Voice";
    case nn::pia::transport::ReliableProtocol::cProtocolType:
        return "Reliable";
    case 0x84:
        return "ReliableBroadcast";
    case nn::pia::session::SessionProtocol::cProtocolType:
        return "Session";
    case nn::pia::inet::MonitoringDataProtocol::cProtocolType:
        return "Feedback";
    case 0xa8:
        return "RelayService";
    default:
        return "(KickoutReason_Unknown PROTOCOL NAME)";
    }
}
}

namespace nn
{
namespace pia
{
namespace transport
{

void PacketAnalysisData::Print(bool isDetail) const
{
    const int elapsed = static_cast<int>(m_passedMilliSecond);
    AnalysisPrinter::Write("[Analysis] ------ BEGIN(%s, %d.%03d sec. passed) ------", m_name, elapsed / 1000,
                           elapsed % 1000);
    AnalysisPrinter::Write("[Analysis] ProtocolId, TotalNum, TotalSize, AverageSize, Protocol(port)");
    unsigned int average = 0;
    if (m_packetNum != 0)
        average = m_packetSize / m_packetNum;
    AnalysisPrinter::Write("[Analysis]     0xffff, %8d,  %8d,    %8d, Packet", static_cast<int>(m_packetNum),
                           static_cast<int>(m_packetSize), static_cast<int>(average));
    for (int i = 0; i < m_entryNum; ++i)
    {
        const Entry& entry = m_entries[i];
        if (!isDetail && isInternalProtocol(entry.m_protocolId))
            continue;
        const unsigned int entryAverage = entry.m_packetNum == 0 ? ~0u : entry.m_packetSize / entry.m_packetNum;
        AnalysisPrinter::Write("[Analysis]     0x%04x, %8d,  %8d,    %8d, %s(%d)",
                               static_cast<unsigned int>(entry.m_protocolId), static_cast<int>(entry.m_packetNum),
                               static_cast<int>(entry.m_packetSize), static_cast<int>(entryAverage),
                               getProtocolName(entry.m_protocolId), static_cast<int>(entry.m_protocolId.GetPort()));
    }
    AnalysisPrinter::Write("[Analysis] ---------------------------- END ----------------------------");
}

int PacketAnalysisData::GetIndex(ProtocolId protocolId) const
{
    for (int i = 0; i < m_entryNum; ++i)
    {
        if (m_entries[i].m_protocolId == protocolId)
            return i;
    }
    return -1;
}

int PacketAnalysisData::CreateIndex(ProtocolId protocolId)
{
    if (m_entryNum < cEntryNumMax)
    {
        m_entries[m_entryNum].m_protocolId = protocolId;
        return m_entryNum++;
    }
    return -1;
}

}
}
}
