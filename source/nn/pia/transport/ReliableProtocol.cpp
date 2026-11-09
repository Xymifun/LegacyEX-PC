#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "platform/wiiu/pead.hpp"
#include <new>

namespace nn
{
namespace pia
{
namespace transport
{


static int g_reliableProtocolSendFailCount = 0;
static unsigned int g_reliableProtocolResendParam1 = 0;
static unsigned int g_reliableProtocolResendParam2 = 0;

ReliableProtocol::ReliableProtocol()
    : Protocol(),
      m_field14(0xfd),
      m_localStationId(0),
      m_field20(0),
      m_field24(0),
      m_dispatchIndex(0),
      m_throughputLimit(0),
      m_sendBudget(0)
{
    if (g_piaWatermarkManager != 0)
    {
        g_piaWatermarkManager->GetWatermark(3)->SetName("ReliableProtocol send buffer num");
        if (g_piaWatermarkManager != 0)
        {
            g_piaWatermarkManager->GetWatermark(4)->SetName("ReliableProtocol receive buffer num");
        }
    }
}

ReliableProtocol::~ReliableProtocol()
{
    Finalize();
}

void ReliableProtocol::Trace(unsigned long long) const
{
}

unsigned short ReliableProtocol::GetProtocolType() const
{
    return cProtocolType;
}

void ReliableProtocol::Finalize()
{
    m_field14 = 0xfd;
    m_localStationId = 0;
    m_field20 = 0;
    m_throughputLimit = 0;
    m_sendBudget = 0;
    if (m_field24 != 0)
    {
        unsigned char* array = m_field24;
        unsigned int count = pead::getArraySize(array) / sizeof(ReliableSlidingWindow);
        ReliableSlidingWindow* p = reinterpret_cast<ReliableSlidingWindow*>(array);
        for (; count != 0; --count)
        {
            p->~ReliableSlidingWindow();
            ++p;
        }
        pead::freeToSeadHeap(array);
        m_field24 = 0;
    }
}

Result ReliableProtocol::Initialize(unsigned int a1, unsigned int a2)
{
    if (m_field24 != 0)
        return ResultAlreadyInitialized();

    m_field14 = 0xfd;
    m_localStationId = 0;
    unsigned int stationNum = g_piaTransport->m_stationNum;
    m_field20 = stationNum;
    int count = static_cast<int>(stationNum) - 1;

    pead::Heap* heap = common::HeapManager::GetHeap();
    unsigned char* array = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(static_cast<unsigned int>(count) * sizeof(ReliableSlidingWindow), heap));
    if (array == 0)
    {
        m_field24 = 0;
    }
    else
    {
        ReliableSlidingWindow* p = reinterpret_cast<ReliableSlidingWindow*>(array);
        for (int i = count; i != 0; --i)
        {
            new (p) ReliableSlidingWindow();
            ++p;
        }
        m_field24 = array;
    }

    if (count != 0)
    {
        int windowIndex = 0;
        for (;;)
        {
            Result r = (reinterpret_cast<ReliableSlidingWindow*>(array) + windowIndex)->Initialize(a1, a2);
            if (r != 0)
            {
                Finalize();
                return r;
            }
            --count;
            ++windowIndex;
            if (count == 0)
                break;
            array = m_field24;
        }
    }

    m_sendBudget = 0;
    m_throughputLimit = 0x0fffffff;
    g_reliableProtocolResendParam1 = a1;
    g_reliableProtocolResendParam2 = a2;
    return 0;
}

Result ReliableProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    if (m_field24 == 0)
        return 0x10c0c;
    if (m_field14 != 0xfd)
        return 0x10c08;

    Station* station = StationManager::s_pInstance->GetStation(address);
    if (station == 0)
        return 0x10c07;

    int stationIndex = station->GetStationIndex();
    if (stationIndex < 0 || stationIndex >= static_cast<int>(m_field20))
        return 0x10c07;

    m_field14 = stationIndex;
    m_sendBudget = 0;
    m_localStationId = station->m_stationId;
    m_dispatchIndex = 0;
    g_reliableProtocolSendFailCount = 0;
    return 0;
}

void ReliableProtocol::CleanupWithStationAddress()
{
    if (m_field14 != 0xfd)
    {
        m_field14 = 0xfd;
        m_localStationId = 0;
        m_sendBudget = 0;
        int count = static_cast<int>(m_field20) - 1;
        for (int i = 0; i != count; ++i)
        {
            if (reinterpret_cast<ReliableSlidingWindow*>(m_field24)[i].IsInCommunication())
                reinterpret_cast<ReliableSlidingWindow*>(m_field24)[i].Cleanup();
        }
    }
}

Result ReliableProtocol::Dispatch()
{
    if (m_field14 == cStationIndexInvalid)
        return 0;

    if (m_throughputLimit != 0)
    {
        if (m_sendBudget <= static_cast<unsigned int>(mStationPacketHandler->GetPayloadSizeLimit()))
            m_sendBudget += m_throughputLimit;
        else
            m_sendBudget = mStationPacketHandler->GetPayloadSizeLimit() + m_throughputLimit;
    }
    else
        m_sendBudget = 0;

    PacketHandler::Iterator& iterator = mStationPacketHandler->GetIterator(GetProtocolId());
    for (iterator.BeginIteration(); !iterator.IsEndIteration(); iterator.NextIteration())
    {
        ProtocolMessageReader* reader = iterator.GetMessageReader();
        StationIndex stationIndex = g_piaTransport->ConvertToStationIndexUnsafe(reader->m_header.m_source);
        if (stationIndex < 0 || stationIndex >= static_cast<int>(m_field20) || stationIndex == m_field14)
            continue;

        ReliableSlidingWindow* window = GetSlidingWindow(stationIndex);
        if (!window->IsInCommunication())
            continue;

        Result result = window->AnalyzeProtocolMessage(*reader);
        if (result == 0 && g_piaWatermarkManager != 0)
        {
            unsigned int sendCount = window->m_sendBuffer.m_count;
            g_piaWatermarkManager->GetWatermark(3)->Update(sendCount);
            if (g_piaWatermarkManager != 0)
            {
                unsigned int receiveCount = window->m_receiveBuffer.m_count;
                g_piaWatermarkManager->GetWatermark(4)->Update(receiveCount);
            }
        }
    }

    unsigned int windowNum = m_field20 - 1;
    unsigned int startIndex = m_dispatchIndex + 1;
    if (startIndex >= windowNum)
        startIndex = 0;
    m_dispatchIndex = startIndex;

    unsigned int sentSize = 0;
    for (unsigned int i = m_dispatchIndex; i < windowNum; ++i)
    {
        if (reinterpret_cast<ReliableSlidingWindow*>(m_field24)[i].IsInCommunication())
        {
            reinterpret_cast<ReliableSlidingWindow*>(m_field24)[i].Dispatch(
                static_cast<StationPacketHandler*>(GetStationPacketHandler()), &sentSize, m_sendBudget);
            m_sendBudget -= sentSize;
        }
    }
    for (unsigned int i = 0; i < m_dispatchIndex; ++i)
    {
        if (reinterpret_cast<ReliableSlidingWindow*>(m_field24)[i].IsInCommunication())
        {
            reinterpret_cast<ReliableSlidingWindow*>(m_field24)[i].Dispatch(
                static_cast<StationPacketHandler*>(GetStationPacketHandler()), &sentSize, m_sendBudget);
            m_sendBudget -= sentSize;
        }
    }
    return 0;
}

Result ReliableProtocol::UpdateProtocolEvent(const ProtocolEvent& event)
{
    if (m_field14 == 0xfd)
        return ResultInvalidState();

    Station* station = StationManager::s_pInstance->GetStation(event.m_stationAddress);
    int stationIndex = station->GetStationIndex();

    if (stationIndex < 0 || stationIndex >= static_cast<int>(m_field20))
        return 0x10c07;
    if (stationIndex == m_field14)
        return 0x10c07;

    ReliableSlidingWindow* window = GetSlidingWindow(static_cast<StationIndex>(stationIndex));
    switch (event.m_type)
    {
    case 0:
    {
        unsigned short protocolIdValue = mProtocolId;
        return window->Startup(reinterpret_cast<StationPacketHandler*>(GetStationPacketHandler()), protocolIdValue,
                               m_localStationId, station->m_stationId);
    }
    case 1:
        window->Cleanup();
        return 0;
    }
    return 0x10c07;
}

ReliableSlidingWindow* ReliableProtocol::GetSlidingWindow(StationIndex stationIndex)
{
    int index = stationIndex < m_field14 ? stationIndex : stationIndex - 1;
    return reinterpret_cast<ReliableSlidingWindow*>(m_field24) + index;
}

const ReliableSlidingWindow* ReliableProtocol::GetSlidingWindow(StationIndex stationIndex) const
{
    int index = stationIndex < m_field14 ? stationIndex : stationIndex - 1;
    return reinterpret_cast<const ReliableSlidingWindow*>(m_field24) + index;
}

Result ReliableProtocol::Send(StationIndex stationIndex, const void* data, unsigned int size)
{
    unsigned char* array = m_field24;
    if (array == 0)
        return ResultInvalidState();
    if (m_field14 == 0xfd)
        return 0x2c26;
    if (stationIndex == m_field14)
        return ResultInvalidArgument();

    if (stationIndex == static_cast<StationIndex>(0xff))
    {
        int count = static_cast<int>(m_field20) - 1;
        if (count != 0)
        {
            int windowIndex = 0;
            int iterationsLeft = count;
            for (;;)
            {
                ReliableSlidingWindow* window = (reinterpret_cast<ReliableSlidingWindow*>(array) + windowIndex);
                if (window->IsInCommunication() && window->CanPushData(size) == 0)
                {
                    if (g_reliableProtocolSendFailCount != -1)
                        ++g_reliableProtocolSendFailCount;
                    return 0x4c0d;
                }
                --iterationsLeft;
                ++windowIndex;
                if (iterationsLeft == 0)
                    break;
                array = m_field24;
            }
        }

        if (count != 0)
        {
            int windowIndex = 0;
            do
            {
                ReliableSlidingWindow* window = (reinterpret_cast<ReliableSlidingWindow*>(m_field24) + windowIndex);
                if (window->IsInCommunication())
                {
                    Result r = window->PushData(data, size);
                    if (r != 0)
                        return r;
                    if (g_piaWatermarkManager != 0)
                    {
                        unsigned int trackedValue = window->m_sendBuffer.m_count;
                        g_piaWatermarkManager->GetWatermark(3)->Update(trackedValue);
                    }
                }
                --count;
                ++windowIndex;
            } while (count != 0);
        }
        return 0;
    }

    if (stationIndex < 0 || m_field20 <= static_cast<unsigned int>(stationIndex))
        return 0x10c07;

    ReliableSlidingWindow* window = GetSlidingWindow(stationIndex);
    if (!window->IsInCommunication())
        return 0x2c26;

    Result r = window->PushData(data, size);
    if (r == 0)
    {
        if (g_piaWatermarkManager != 0)
        {
            unsigned int trackedValue = window->m_sendBuffer.m_count;
            g_piaWatermarkManager->GetWatermark(3)->Update(trackedValue);
        }
    }
    else if (r == 0x4c0d && g_reliableProtocolSendFailCount != -1)
    {
        ++g_reliableProtocolSendFailCount;
    }
    return r;
}

Result ReliableProtocol::Send(unsigned long long stationId, const void* data, unsigned int size)
{
    StationIndex index = static_cast<StationIndex>(0);
    Result r = g_piaTransport->ConvertToStationIndex(&index, stationId);
    if (r == 0)
        return Send(index, data, size);
    return r;
}

Result ReliableProtocol::SendToAll(const void* data, unsigned int size)
{
    return Send(cStationIndexSentinelFF, data, size);
}

Result ReliableProtocol::Receive(unsigned long long* stationId, void* buffer, unsigned int* size,
                                 unsigned int bufferSize)
{
    if (stationId == 0)
        return ResultInvalidArgument();
    if (m_field14 == 0xfd)
        return 0x2c26;

    int count = static_cast<int>(m_field20) - 1;
    if (count != 0)
    {
        int windowIndex = 0;
        do
        {
            ReliableSlidingWindow* window = (reinterpret_cast<ReliableSlidingWindow*>(m_field24) + windowIndex);
            if (window->IsInCommunication())
            {
                unsigned long long windowStationId = window->m_remoteStationId;
                if (g_piaTransport->m_stationIdTable->IsJoinEventNotifiedByStationId(windowStationId))
                {
                    Result popResult = window->PopData(buffer, size, bufferSize);
                    if (popResult == 0)
                    {
                        if (g_piaWatermarkManager != 0)
                        {
                            unsigned int trackedValue = window->m_receiveBuffer.m_count;
                            g_piaWatermarkManager->GetWatermark(4)->Update(trackedValue);
                        }
                        *stationId = windowStationId;
                        return 0;
                    }
                    if (popResult != 0x2c09)
                        return popResult;
                }
            }
            --count;
            ++windowIndex;
        } while (count != 0);
    }
    return 0x2c09;
}

bool ReliableProtocol::IsInCommunication(StationIndex stationIndex) const
{
    if (m_field14 == 0xfd || stationIndex < 0 || stationIndex >= static_cast<int>(m_field20)
        || stationIndex == m_field14)
        return false;
    return GetSlidingWindow(stationIndex)->IsInCommunication();
}

bool ReliableProtocol::IsInCommunication(unsigned long long stationId) const
{
    return IsInCommunication(Conv2StationIndex(stationId));
}

void ReliableProtocol::SetThroughputLimit(unsigned int limit)
{
    m_throughputLimit = limit;
}

void ReliableProtocol::RegisterResendTimeCallback(int (*callback)(int, int))
{
    ReliableSlidingWindow::RegisterResendTimeCallback(callback);
}

}
}
}
