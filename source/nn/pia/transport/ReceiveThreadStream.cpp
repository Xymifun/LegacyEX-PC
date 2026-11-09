#include "nn/pia/transport/ReceiveThreadStream.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/transport/LatencyEmulator.hpp"
#include "nn/pia/transport/NetworkRttManager.hpp"
#include "nn/pia/Transport.hpp"

namespace nn
{
namespace pia
{
namespace transport
{

namespace
{
enum
{
    cTraceThreadStream = 0x80000
};

enum
{
    cWatermarkReceiveThreadStream = 1
};
}

ReceiveThreadStream::ReceiveThreadStream() : TransportThreadStream(), m_packetInput(0), m_receiveDataSize(0)
{
}

ReceiveThreadStream::~ReceiveThreadStream()
{
}

Result ReceiveThreadStream::Initialize(common::IPacketInput* packetInput, unsigned int packetNum, int priority,
                                       unsigned int latencyEntryNum, bool isPacketLossEnabled)
{
    if (m_packetInput != 0)
        return cResultAlreadyInitialized;
    if (packetInput == 0)
        return ResultInvalidArgument();

    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(cWatermarkReceiveThreadStream)->SetName("ReceiveThreadStream buffer num");

    Result result = InitializeCore("Pia ReceiveThreadStream", priority, packetNum, latencyEntryNum, isPacketLossEnabled,
                                   cWatermarkReceiveThreadStream);
    if (result != 0)
        return result;

    if (m_latencyEmulator != 0)
        m_latencyEmulator->SetInput(packetInput);
    m_receivePacketNum = 0;
    m_packetInput = packetInput;
    m_receiveDataSize = 0;
    return 0;
}

void ReceiveThreadStream::Finalize()
{
    FinalizeCore();
    m_packetInput = 0;
}

void ReceiveThreadStream::SetMonitoringData()
{
    const int sleepMilliSecond = m_sleepMilliSecond;
    common::g_sessionStateMonitoringContent.m_receiveThreadSleepMilliSecond =
        (0 < sleepMilliSecond && sleepMilliSecond < 0xff) ? static_cast<unsigned char>(sleepMilliSecond) : 0xff;
}

void ReceiveThreadStream::OnStartup()
{
    common::g_sessionStateMonitoringContent.m_receiveThreadMetric = 0;
}

Result ReceiveThreadStream::ProcessOne()
{
    Writer& writer = m_writer;
    common::Packet* packet;
    if (static_cast<int>(writer.m_field0c) > 0)
    {
        packet = writer.Get(0);
    }
    else
    {
        packet = writer.Assign();
        if (packet == 0)
        {
            PacketStream::Trace(cTraceThreadStream);
            return cResultWindowOverflow;
        }
    }

    Result result;
    if (m_latencyEmulator != 0)
        result = m_latencyEmulator->Read(packet);
    else
        result = m_packetInput->Read(packet);
    if (result != 0)
        return result;

    packet->m_packetData.DeserializeHeader();
    packet->m_field603 = packet->m_packetData.m_header.m_version != 1;
    if (isDropPacket())
        return cResultDataNotReady;

    ++m_receivePacketNum;
    m_receiveDataSize += packet->m_size;
    if (!packet->IsValid())
        return cResultDataSizeInvalid;

    if (NetworkRttManager::sInstance != 0)
        NetworkRttManager::sInstance->ReadFromPacket(packet);
    writer.Push();
    return 0;
}

}
}
}
