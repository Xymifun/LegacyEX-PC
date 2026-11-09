#include "nn/pia/transport/UnreliableProtocol.hpp"
#include "nn/pia/ReceivedDataContainer.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/TransportSupport.hpp"
#include "platform/wiiu/pead.hpp"
#include <new>
#include <string.h>

namespace nn
{
namespace pia
{
namespace transport
{

unsigned int g_receivedDataCount = 0;

UnreliableProtocol::UnreliableProtocol()
    : Protocol(),
      m_receivedData(0),
      m_receivedDataContainer(0),
      m_stationIndex(cInvalidStationIndex)
{
    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(9)->SetName("UnreliableProtocol receive buffer num");
}

UnreliableProtocol::~UnreliableProtocol()
{
    if (m_receivedDataContainer != 0 || m_receivedData != 0)
        Finalize();
}

void UnreliableProtocol::Trace(unsigned long long) const
{
}

unsigned short UnreliableProtocol::GetProtocolType() const
{
    return cProtocolType;
}

Result UnreliableProtocol::Initialize(unsigned int count)
{
    if (count == 0)
        return ResultInvalidArgument();
    if (!IsInitialized())
        return ResultNotInitialized();
    if (m_receivedDataContainer != 0)
        return ResultAlreadyInitialized();

    m_receivedDataContainer = common::NewObj<ReceivedDataContainer>();
    if (m_receivedDataContainer == 0)
        return cResultOutOfMemory;
    m_receivedData = common::NewArray<unsigned char>(count * sizeof(ReceivedDataContainer::Node));
    if (m_receivedData == 0)
    {
        common::DeleteObj(m_receivedDataContainer);
        m_receivedDataContainer = 0;
        return cResultOutOfMemory;
    }
    m_receivedDataContainer->Initialize(m_receivedData, count);
    g_receivedDataCount = count;
    return 0;
}

void UnreliableProtocol::Finalize()
{
    CleanupWithStationAddress();
    if (m_receivedDataContainer != 0)
    {
        common::DeleteObj(m_receivedDataContainer);
        m_receivedDataContainer = 0;
    }
    if (m_receivedData != 0)
    {
        common::DeleteArray(static_cast<unsigned char*>(m_receivedData));
        m_receivedData = 0;
    }
}

Result UnreliableProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    Station* station = StationManager::s_pInstance->GetStation(address);
    if (station == 0)
        return ResultInvalidArgument();
    if (station->m_field10 >= StationManager::cStationNumMax)
        return cResultInvalidArgument;
    if (m_stationIndex != cInvalidStationIndex)
        return cResultInvalidState;
    m_stationIndex = station->m_field10;
    return 0;
}

void UnreliableProtocol::CleanupWithStationAddress()
{
    m_stationIndex = cInvalidStationIndex;
    if (m_receivedDataContainer != 0)
        m_receivedDataContainer->Clear();
}

Result UnreliableProtocol::Dispatch()
{
    if (mStationPacketHandler == 0)
        return ResultInvalidState();
    if (m_receivedDataContainer == 0)
        return ResultInvalidState();

    PacketHandler::Iterator& iterator = mStationPacketHandler->GetIterator(GetProtocolId());
    while (!iterator.IsEndIteration())
    {
        if (m_receivedDataContainer->GetFreeNum() != 0)
        {
            ProtocolMessageReader* reader = iterator.GetMessageReader();
            Payload* payload = m_receivedDataContainer->Alloc();
            payload->stationId = reader->m_header.m_source;
            payload->size = reader->m_messageSize;
            memcpy(payload->data, reader->m_message + ProtocolMessageReader::cMessageHeaderSize, reader->m_messageSize);
        }
        iterator.NextIteration();
    }
    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(9)->Update(m_receivedDataContainer->GetNodeNum());
    return 0;
}

Result UnreliableProtocol::UpdateProtocolEvent(const ProtocolEvent& event)
{
    Station* station = StationManager::s_pInstance->GetStation(event.m_stationAddress);
    unsigned long long stationId = station->m_stationId;
    if (event.m_type <= 1)
    {
        Payload* payload = m_receivedDataContainer->Front();
        while (payload != 0)
        {
            if (payload->stationId == stationId)
            {
                Payload* current = payload;
                payload = m_receivedDataContainer->Next(payload);
                m_receivedDataContainer->Destroy(current);
            }
            else
            {
                payload = m_receivedDataContainer->Next(payload);
            }
        }
    }
    return 0;
}

Result UnreliableProtocol::Receive(unsigned long long* stationId, void* buffer, unsigned int* size,
                                   unsigned int bufferSize)
{
    if (stationId == 0)
        return ResultInvalidArgument();
    if (buffer == 0)
        return ResultInvalidArgument();
    if (bufferSize == 0)
        return ResultInvalidArgument();
    if (size == 0)
        return ResultInvalidArgument();

    if (m_receivedDataContainer == 0)
        return ResultInvalidState();
    if (!IsInCommunication())
        return 0x2c26;
    if (m_receivedDataContainer->GetNodeNum() == 0)
        return cResultDataNotReady;
    if (m_receivedDataContainer->Front()->size > bufferSize)
        return 0x10c04;

    Payload* payload = m_receivedDataContainer->Front();
    while (payload != 0)
    {
        if (g_piaTransport->m_stationIdTable->IsJoinEventNotifiedByStationId(payload->stationId))
            break;
        Payload* current = payload;
        payload = m_receivedDataContainer->Next(payload);
        m_receivedDataContainer->Destroy(current);
    }
    if (payload == 0)
        return cResultDataNotReady;

    if (payload->size < bufferSize)
        bufferSize = payload->size;
    memcpy(buffer, payload->data, bufferSize);
    *size = bufferSize;
    *stationId = payload->stationId;
    m_receivedDataContainer->Destroy(payload);
    if (g_piaWatermarkManager != 0)
        g_piaWatermarkManager->GetWatermark(9)->Update(m_receivedDataContainer->GetNodeNum());
    return 0;
}

bool UnreliableProtocol::IsInCommunication() const
{
    return m_stationIndex != cInvalidStationIndex;
}

Result UnreliableProtocol::sendImpl(StationIndex stationIndex, const void* data, unsigned int size)
{
    if (!common::isValidDestinationStationIndex(stationIndex))
        return ResultInvalidArgument();
    if (data == 0)
    {
        if (size != 0)
            return ResultInvalidArgument();
    }
    else if (size > cReceivedDataSizeMax)
    {
        return ResultInvalidArgument();
    }
    if (m_receivedDataContainer == 0)
        return ResultInvalidState();
    if (stationIndex == static_cast<StationIndex>(cInvalidStationIndex))
        return ResultInvalidArgument();

    StationPacketHandler* handler = static_cast<StationPacketHandler*>(GetStationPacketHandler());
    if (handler == 0)
        return ResultInvalidState();
    if (size > static_cast<unsigned int>(handler->GetPayloadSizeLimit()))
        return ResultInvalidArgument();
    if (!IsInCommunication())
        return 0x2c26;
    if (g_piaTransport->m_transportStatus == static_cast<Result>(cResultTransportUnavailable))
        return static_cast<Result>(cResultTransportUnavailable);

    ProtocolMessageWriter* writer = handler->AssignByStationIndex(GetProtocolId(), stationIndex, size, false, false,
                                                                  true);
    if (writer == 0)
        return 0;
    writer->SetPayload(data);
    return handler->Commit();
}

Result UnreliableProtocol::Send(StationIndex stationIndex, const void* data, unsigned int size)
{
    return sendImpl(stationIndex, data, size);
}

Result UnreliableProtocol::Send(unsigned long long stationId, const void* data, unsigned int size)
{
    StationIndex index = static_cast<StationIndex>(0);
    Result result = g_piaTransport->ConvertToStationIndex(&index, stationId);
    if (result == 0)
        return Send(index, data, size);
    return result;
}

void UnreliableProtocol::SendToAll(const void* data, unsigned int size)
{
    Send(static_cast<StationIndex>(cStationIndexAll), data, size);
}

Result UnreliableProtocol::SendToMulti(unsigned int stationBitmap, const void* data, unsigned int size)
{
    if (stationBitmap == 0 || (data == 0 && size != 0))
        return cResultInvalidArgument;
    if (m_receivedDataContainer == 0)
        return cResultInvalidState;

    StationPacketHandler* handler = static_cast<StationPacketHandler*>(GetStationPacketHandler());
    if (handler == 0)
        return cResultInvalidState;
    if (size > static_cast<unsigned int>(handler->GetPayloadSizeLimit()))
        return ResultInvalidArgument();
    if (!IsInCommunication())
        return 0x2c26;

    ProtocolMessageWriter* writer = handler->AssignByStationBitmap(GetProtocolId(), stationBitmap, size, false, false,
                                                                   true);
    if (writer == 0)
        return 0;
    writer->SetPayload(data);
    return handler->Commit();
}

}
}
}
