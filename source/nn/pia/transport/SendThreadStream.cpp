#include "nn/pia/transport/SendThreadStream.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

#include "nn/pia/common/IPacketOutput.hpp"
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
    cWatermarkSendThreadStream = 0
};
}

SendThreadStream::SendThreadStream() : TransportThreadStream(), m_packetOutput(0), m_sendDataSize(0)
{
}

SendThreadStream::~SendThreadStream()
{
}

Result SendThreadStream::Initialize(common::IPacketOutput* packetOutput, unsigned int packetNum, int priority,
                                    unsigned int latencyEntryNum, bool isPacketLossEnabled)
{
    if (m_packetOutput != 0)
        return cResultAlreadyInitialized;
    if (packetOutput == 0)
        return ResultInvalidArgument();

    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(cWatermarkSendThreadStream)->SetName("SendThreadStream buffer num");

    Result result = InitializeCore("Pia SendThreadStream", priority, packetNum, latencyEntryNum, isPacketLossEnabled,
                                   cWatermarkSendThreadStream);
    if (result != 0)
        return result;

    if (m_latencyEmulator != 0)
        m_latencyEmulator->SetOutput(packetOutput);
    m_packetOutput = packetOutput;
    m_sendPacketNum = 0;
    m_sendDataSize = 0;
    return 0;
}

void SendThreadStream::Finalize()
{
    FinalizeCore();
    m_packetOutput = 0;
}

void SendThreadStream::SetMonitoringData()
{
    const int sleepMilliSecond = m_sleepMilliSecond;
    common::g_sessionStateMonitoringContent.m_sendThreadSleepMilliSecond =
        (0 < sleepMilliSecond && sleepMilliSecond < 0xff) ? static_cast<unsigned char>(sleepMilliSecond) : 0xff;
}

Result SendThreadStream::ProcessOne()
{
    common::Packet* packet;
    Reader& reader = m_reader;
    if (static_cast<int>(reader.m_field0c) > 0)
    {
        packet = reader.Get(0);
    }
    else
    {
        packet = reader.PullOne();
        if (packet == 0)
            return cResultDataNotReady;
    }

    if (NetworkRttManager::sInstance != 0)
        NetworkRttManager::sInstance->WriteToPacket(packet);
    packet->m_packetData.SerializeHeader();
    ++m_sendPacketNum;
    m_sendDataSize += packet->m_size;
    Result result;
    if (isDropPacket())
    {
        reader.Release();
        result = 0;
    }
    else
    {
        if (m_latencyEmulator != 0)
            result = m_latencyEmulator->Write(*packet);
        else
            result = m_packetOutput->Write(*packet);
        if (result == 0)
            reader.Release();
    }
    return result;
}

}
}
}
