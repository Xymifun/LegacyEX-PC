#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/StationIdTable.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/ConnectStationJob.hpp"
#include "nn/pia/transport/DisconnectStationJob.hpp"
#include "nn/pia/transport/IdentificationInfoTable.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProcessConnectionRequestJob.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/Common.hpp"

#include <coreinit/memory.h>

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
    return static_cast<unsigned int>(buffer[0]) << 24 | static_cast<unsigned int>(buffer[1]) << 16
           | static_cast<unsigned int>(buffer[2]) << 8 | static_cast<unsigned int>(buffer[3]);
}

enum
{
    cResponseTokenOffset = 0x11,
    cResponseInfoFieldOffset = 0x31,
    cResponsePlayerCountOffset = 0x35,
    cResponseParticipantCountOffset = 0x36,
    cResponseEntryCountOffset = 0x37,
    cResponseHeaderSize = 0x38,
    cResponseEntrySize = 0xc3,
    cResponseNameLength = 0x50,
    cResponseWideNameLength = 0x14,
    cResponseNameEncodingOffset = 0x50,
    cResponseSecondNameOffset = 0x51,
    cResponseSecondNameLength = 0x28,
    cResponseSecondWideNameLength = 10,
    cResponseSecondNameEncodingOffset = 0x79,
    cResponseFlagOffset = 0x7a,
    cResponseBytesOffset = 0x7b,
    cResponseBytesLength = 0x40,
    cResponsePrincipalOffset = 0xbb,
    cResponseFlagUnset = 0xff,
    cTraceStationProtocol = 0x8000
};

}

StationProtocol::StationProtocol() : m_timeoutMsec(10000)
{
}

StationProtocol::~StationProtocol()
{
}

Result StationProtocol::Initialize()
{
    return 0;
}

Result StationProtocol::StartupWithStationAddress(const common::StationAddress&)
{
    m_reconnectionCount = 0;
    return 0;
}

void StationProtocol::CleanupWithStationAddress()
{
    m_reconnectionCount = 0;
}

void StationProtocol::SendDenyingConnectionResponse(const common::StationAddress& address, unsigned char rejection,
                                                    unsigned long long principalId, unsigned int infoField)
{
    ProtocolId protocolId = mProtocolId;
    ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(protocolId, address, 17, false,
                                                                                  false);
    if (writer)
    {
        writer->WriteU8(cMessageTypeConnectionResponse, 0);
        writer->WriteU8(rejection, 1);
        writer->WriteU8(8, 2);
        writer->WriteU8(0, 3);
        writer->WriteU8(0, 4);
        writer->WriteU64(principalId, 5);
        writer->WriteU32(infoField, 13);
        mStationPacketHandler->Commit();
    }
}

void StationProtocol::SendAck(unsigned int value, StationIndex stationIndex)
{
    ProtocolId protocolId = mProtocolId;
    StationPacketHandler* handler = static_cast<StationPacketHandler*>(GetStationPacketHandler());
    ProtocolMessageWriter* writer = handler->AssignByStationIndex(protocolId, stationIndex, 8, false, false, true);
    if (writer)
    {
        unsigned char message[8] = {0};
        message[0] = cMessageTypeAck;
        common::serializeU32(message + 4, value);
        writer->SetPayload(message);
        mStationPacketHandler->Commit();
    }
}

void StationProtocol::SendAck(unsigned int value, const common::StationAddress& address)
{
    ProtocolId protocolId = mProtocolId;
    ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(protocolId, address, 8, false, false);
    if (writer)
    {
        unsigned char message[8] = {0};
        message[0] = cMessageTypeAck;
        common::serializeU32(message + 4, value);
        writer->SetPayload(message);
        mStationPacketHandler->Commit();
    }
}

void StationProtocol::ParseConnectionRequestCommon(const ReceivedMessageAccessor& message, bool relay)
{
    if (StationManager::s_pInstance->m_localStation == 0)
        return;
    const unsigned char* data = message.m_message;
    if (data[2] != 8)
    {
        SendDenyingConnectionResponse(message.m_stationAddress, 2, 0, 0);
        return;
    }
    unsigned char secure = data[3];
    if (secure != 0 && secure != 1)
        return;
    unsigned int ackId = g_piaResendingMessageManager->ExtractAckIdFromMessage(data, message.m_messageSize);
    unsigned long long principalId = 0;
    principalId |= static_cast<unsigned long long>(data[4]) << 56;
    principalId |= static_cast<unsigned long long>(data[5]) << 48;
    principalId |= static_cast<unsigned long long>(data[6]) << 40;
    principalId |= static_cast<unsigned long long>(data[7]) << 32;
    principalId |= static_cast<unsigned long long>(data[8]) << 24;
    principalId |= static_cast<unsigned long long>(data[9]) << 16;
    principalId |= static_cast<unsigned long long>(data[10]) << 8;
    principalId |= static_cast<unsigned long long>(data[11]);
    if (principalId != g_stationConnectionInfoTable->m_localStationConnectionInfo.m_stationLocation.m_principalId)
        return;
    if (secure == 1
        && readU32(data + 12)
               != static_cast<unsigned int>(
                   g_stationConnectionInfoTable->m_localStationConnectionInfo.m_stationLocation.m_field18))
        return;
    unsigned char nonce = data[1];
    StationConnectionInfo info;
    info.Deserialize(data + cConnectionRequestHeaderSize);
    const StationLocation& location = info.m_stationLocation;
    Station* station = g_stationConnectionInfoTable->GetStation(info);
    bool reconnected = false;
    if (station == 0)
    {
        Station* reconnecting = g_stationConnectionInfoTable->GetReconnectingStation(info);
        if (reconnecting != 0)
        {
            reconnecting->m_field20 = 6;
            return;
        }
        if (!relay)
        {
            if (location.m_stationAddress.m_inetAddress.IsValid()
                && !info.m_localStationLocation.m_stationAddress.m_inetAddress.IsValid()
                && g_stationConnectionInfoTable->GetStation(location.m_stationAddress) != 0)
            {
                if (g_stationConnectionInfoTable->GetStation(static_cast<unsigned int>(location.m_field1c)) == 0)
                {
                    location.Trace(0x8000);
                    SendDenyingConnectionResponse(message.m_stationAddress, 1, location.m_principalId,
                                                  location.m_field18);
                    return;
                }
                if (g_stationConnectionInfoTable->GetStation(location.m_principalId) == 0)
                {
                    location.Trace(0x8000);
                    SendDenyingConnectionResponse(message.m_stationAddress, 1, location.m_principalId,
                                                  location.m_field18);
                    return;
                }
            }
            station = StationManager::s_pInstance->GetStation(message.m_stationAddress);
            if (location.m_stationAddress.IsValid() && station == 0)
            {
                station = g_stationConnectionInfoTable->GetStation(static_cast<unsigned int>(location.m_field1c));
                if (station != 0)
                {
                    station->Trace(0x8008000);
                    location.Trace(0x8008000);
                    if (secure == 0)
                    {
                        station->m_field20 = 6;
                        return;
                    }
                    common::StationAddress& stationAddress = station->m_stationAddress;
                    const common::StationAddress& sourceAddress = message.m_stationAddress;
                    stationAddress.Trace(0x8000);
                    sourceAddress.Trace(0x8000);
                    ++m_reconnectionCount;
                    stationAddress = sourceAddress;
                    reconnected = true;
                }
            }
        }
    }
    if (station == 0)
    {
        if (secure == 1)
            return;
        bool creatable = false;
        unsigned long long uniqueId = 0;
        if (StationManager::s_pInstance->GetValidStationNum() < g_piaTransport->m_stationNum)
        {
            if (location.m_principalId != 0)
                uniqueId = location.m_principalId;
            else if (info.m_localStationLocation.m_principalId != 0)
                uniqueId = info.m_localStationLocation.m_principalId;
            StationIdTable::Entry* entry = 0;
            if (uniqueId != 0)
                entry = g_piaTransport->m_stationIdTable->FindByUniqueIdCore(uniqueId);
            StationIdTable* idTable = g_piaTransport->m_stationIdTable;
            if (idTable->GetEntryNum() < idTable->m_availableEntryNum || entry != 0)
                creatable = true;
        }
        if (!creatable)
        {
            if (!relay)
            {
                SendDenyingConnectionResponse(message.m_stationAddress, 1, location.m_principalId, location.m_field18);
            }
            return;
        }
        if (uniqueId != 0)
        {
            for (Station** it = StationManager::s_pInstance->m_stations.data;
                 it != StationManager::s_pInstance->m_stations.data + StationManager::s_pInstance->m_stations.size;
                 ++it)
            {
                if ((*it)->m_stationId == uniqueId)
                {
                    if ((*it)->m_field20 == 5)
                        (*it)->m_field68 = 1;
                    else
                        (*it)->m_field20 = 6;
                }
            }
        }
        station = StationManager::s_pInstance->CreateStation();
        station->Startup(relay ? location.m_stationAddress : message.m_stationAddress);
        station->m_field55 = nonce;
        station->m_field20 = 3;
        common::Time now;
        now.SetNow();
        station->m_field54 = static_cast<unsigned char>(now.m_tick % 0xfe + 2);
        ConnectStationJob* connectJob = station->m_connectStationJob;
        if (connectJob->GetState() != common::Job::cStateNotEntried
            && connectJob->GetState() != common::Job::cStateCompleted)
        {
            connectJob->Cleanup();
            connectJob->Reset(true);
        }
    }
    else
    {
        if (!reconnected && secure == 0)
        {
            station->Trace(0x8008000);
            if (station->m_field55 == nonce)
                return;
            station->Trace(0x8000000);
            station->m_field20 = 6;
            return;
        }
        if (data[16] != station->m_field54)
            return;
        if (station->m_field55 == 0)
        {
            station->m_field55 = nonce;
            station->Trace(0x8000000);
        }
        else if (station->m_field55 != nonce)
        {
            station->Trace(0x8000000);
            station->m_field20 = 6;
            return;
        }
        if (station->m_field20 != 2)
        {
            if (relay)
            {
                if (station->m_field10 != cStationIndexInvalid)
                    SendAck(ackId, static_cast<StationIndex>(station->m_field10));
            }
            else
            {
                SendAck(ackId, station->m_stationAddress);
            }
            if (static_cast<int>(station->m_field10) > StationManager::cStationNumMax - 1)
            {
                station->m_lastReceiveTime = g_piaTransport->m_time;
            }
            return;
        }
        ConnectStationJob* connectJob = station->m_connectStationJob;
        if (connectJob->GetState() == common::Job::cStateCompleted
            || (connectJob->GetState() != common::Job::cStateWaiting
                && connectJob->GetState() != common::Job::cStateRunning))
            return;
    }
    if (g_stationConnectionInfoTable->AddToTable(station, info) != 0)
    {
        g_stationConnectionInfoTable->Trace(0x8000);
        return;
    }
    ProcessConnectionRequestJob* processJob = station->m_processConnectionRequestJob;
    unsigned int started;
    if (relay)
        started = processJob->StartupRelayConnection(station, m_timeoutMsec, secure == 1);
    else
        started = processJob->Startup(station, m_timeoutMsec, secure == 1);
    if (started != 0)
    {
        processJob->Ready(false);
        if (relay)
        {
            if (static_cast<int>(station->m_field10) <= StationManager::cStationNumMax - 1)
                SendAck(ackId, static_cast<StationIndex>(station->m_field10));
        }
        else
        {
            SendAck(ackId, station->m_stationAddress);
        }
        if (static_cast<int>(station->m_field10) > StationManager::cStationNumMax - 1)
        {
            station->m_lastReceiveTime = g_piaTransport->m_time;
            station->m_lastSendTime = g_piaTransport->m_time;
        }
    }
    else
    {
        g_stationConnectionInfoTable->EraseFromTable(station);
    }
}

void StationProtocol::ParseConnectionRequest(const ReceivedMessageAccessor& message)
{
    ParseConnectionRequestCommon(message, false);
}

void StationProtocol::ParseConnectionResponseCommon(const ReceivedMessageAccessor& message, bool relay)
{
    if (StationManager::s_pInstance->m_localStation == 0)
        return;
    unsigned int ackId = g_piaResendingMessageManager->ExtractAckIdFromMessage(message.m_message,
                                                                               message.m_messageSize);
    const unsigned char* data = message.m_message;
    unsigned char rejection = data[1];
    if (rejection != 2)
    {
        unsigned long long principalId = 0;
        principalId |= static_cast<unsigned long long>(data[5]) << 56;
        principalId |= static_cast<unsigned long long>(data[6]) << 48;
        principalId |= static_cast<unsigned long long>(data[7]) << 40;
        principalId |= static_cast<unsigned long long>(data[8]) << 32;
        principalId |= static_cast<unsigned long long>(data[9]) << 24;
        principalId |= static_cast<unsigned long long>(data[10]) << 16;
        principalId |= static_cast<unsigned long long>(data[11]) << 8;
        principalId |= static_cast<unsigned long long>(data[12]);
        if (principalId != g_stationConnectionInfoTable->m_localStationConnectionInfo.m_stationLocation.m_principalId)
            return;
        if (readU32(data + 13)
            != static_cast<unsigned int>(
                g_stationConnectionInfoTable->m_localStationConnectionInfo.m_stationLocation.m_field18))
            return;
    }
    Station* station = StationManager::s_pInstance->GetStation(message.m_stationAddress);
    if (station == 0)
    {
        Station* candidate = StationManager::s_pInstance->GetStationByStationIdWithoutConvert(
            message.m_sourceStationId);
        if (relay)
        {
            station = candidate;
        }
        else
        {
            if (candidate == 0)
                return;
            candidate->Trace(0x8000000);
            if (!candidate->m_stationAddress.m_inetAddress.IsPrivate()
                || message.m_stationAddress.m_inetAddress.IsPrivate())
            {
                if (candidate->m_stationAddress.m_inetAddress.IsPrivate())
                    return;
                if (!message.m_stationAddress.m_inetAddress.IsPrivate())
                    return;
            }
            StationConnectionInfo info;
            Result infoResult = g_stationConnectionInfoTable->GetStationConnectionInfo(candidate, &info);
            if (infoResult == 0)
            {
                const common::StationAddress& sourceAddress = message.m_stationAddress;
                common::StationAddress& candidateAddress = candidate->m_stationAddress;
                info.Trace(0x8000000);
                sourceAddress.Trace(0x8000000);
                if (info.m_stationLocation.m_stationAddress.m_inetAddress.m_address
                        == sourceAddress.m_inetAddress.m_address
                    || info.m_localStationLocation.m_stationAddress.m_inetAddress.m_address
                           == sourceAddress.m_inetAddress.m_address)
                {
                    candidateAddress.Trace(0x8000000);
                    candidateAddress = sourceAddress;
                    candidateAddress.Trace(0x8000000);
                    station = candidate;
                }
            }
        }
        if (station == 0)
            return;
    }
    if (rejection != 0)
    {
        station->m_connectStationJob->m_connectionRejection = rejection;
        return;
    }
    if (data[cResponseEntryCountOffset] > 1)
    {
        station->m_connectStationJob->m_connectionRejection = 2;
        return;
    }
    station->m_lastReceiveTime = g_piaTransport->m_time;
    station->m_lastSendTime = g_piaTransport->m_time;
    if (relay)
    {
        if (station->m_field10 != cStationIndexInvalid)
            SendAck(ackId, static_cast<StationIndex>(station->m_field10));
    }
    else
    {
        SendAck(ackId, station->m_stationAddress);
    }
    if (station->m_field20 != 4)
    {
        station->Trace(0x8000000);
        return;
    }
    unsigned char section = data[4];
    if (section > 2)
        return;
    IdentificationInfo info;
    OSBlockSet(info.m_field00.m_token, 0, sizeof(info.m_field00.m_token));
    info.m_fieldC8[0].Clear();
    info.m_field60[0].Clear();
    info.m_field108 = Station::PlayerInfo::cLanguageUnset;
    OSBlockSet(info.m_field20.m_bytes, 0, sizeof(info.m_field20.m_bytes));
    OSBlockMove(info.m_field00.m_token, data + cResponseTokenOffset, sizeof(info.m_field00.m_token), FALSE);
    info.m_field109 = data[3];
    info.m_field10c = common::deserializeU32(data + cResponseInfoFieldOffset);
    info.m_playerCount = data[cResponsePlayerCountOffset];
    info.m_participantCount = data[cResponseParticipantCountOffset];
    unsigned int count = data[cResponseEntryCountOffset];
    if (count > 1)
        return;
    unsigned int first;
    int remaining;
    if (section == 0)
    {
        first = 0;
        remaining = count;
    }
    else if (section == 1)
    {
        first = 0;
        remaining = 2;
    }
    else
    {
        first = section * 2 - 2;
        remaining = count - first;
        if (remaining > 2)
            remaining = 2;
    }
    unsigned char* flags = &info.m_field108;
    unsigned long long* principals = info.m_principalId;
    for (unsigned int index = first; remaining > 0; --remaining, ++index)
    {
        unsigned int offset = cResponseHeaderSize + (index - first) * cResponseEntrySize;
        if (data[offset + cResponseNameEncodingOffset] == common::String::EncodingType_Utf16)
        {
            wchar_t text[cResponseWideNameLength + 1];
            for (unsigned int n = 0; n < cResponseWideNameLength; ++n)
                text[n] = common::deserializeU16(data + offset + n * 2);
            text[cResponseWideNameLength] = 0;
            info.m_field60[index].Copy(common::String(text));
        }
        else
        {
            char text[cResponseNameLength + 1];
            for (unsigned int n = 0; n < cResponseNameLength; ++n)
                text[n] = common::deserializeU8(data + offset + n);
            text[cResponseNameLength] = 0;
            info.m_field60[index].Copy(common::String(text));
        }
        if (data[offset + cResponseSecondNameEncodingOffset] == common::String::EncodingType_Utf16)
        {
            wchar_t text[cResponseSecondWideNameLength + 1];
            for (unsigned int n = 0; n < cResponseSecondWideNameLength; ++n)
                text[n] = common::deserializeU16(data + offset + cResponseSecondNameOffset + n * 2);
            text[cResponseSecondWideNameLength] = 0;
            info.m_fieldC8[index].Copy(common::String(text));
        }
        else
        {
            char text[cResponseSecondNameLength + 1];
            for (unsigned int n = 0; n < cResponseSecondNameLength; ++n)
                text[n] = common::deserializeU8(data + offset + cResponseSecondNameOffset + n);
            text[cResponseSecondNameLength] = 0;
            info.m_fieldC8[index].Copy(common::String(text));
        }
        flags[index] = data[offset + cResponseFlagOffset];
        for (unsigned int n = 0; n < cResponseBytesLength; ++n)
            info.m_field20.m_bytes[index * cResponseBytesLength + n] = data[offset + cResponseBytesOffset + n];
        principals[index] = common::deserializeU64(data + offset + cResponsePrincipalOffset);
    }
    Result addResult = IdentificationInfoTable::GetInstance()->AddToTable(station, &info, section);
    if (addResult == 0)
    {
        Result getResult = IdentificationInfoTable::GetInstance()->GetIdentificationInfo(station, &info);
        if (getResult == 0)
        {
            station->m_field20 = 5;
            return;
        }
    }
    IdentificationInfoTable::GetInstance()->Trace(0x8000);
}

void StationProtocol::ParseConnectionResponse(const ReceivedMessageAccessor& message)
{
    ParseConnectionResponseCommon(message, false);
}

unsigned int StationProtocol::SendSimpleMessage(unsigned char messageType, const common::StationAddress& address)
{
    ProtocolId protocolId = mProtocolId;
    ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(protocolId, address, 1, false, false);
    if (writer == 0)
        return 0;
    writer->WriteU8(messageType, 0);
    if (mStationPacketHandler->Commit() != 0)
        return 0;
    return 1;
}

void StationProtocol::SendDisconnectionResponse(const common::StationAddress& address)
{
    SendSimpleMessage(cMessageTypeDisconnectionResponse, address);
}

void StationProtocol::ParseDisconnectionRequest(const ReceivedMessageAccessor& message)
{
    if (StationManager::s_pInstance->m_localStation == 0)
        return;
    Station* station = StationManager::s_pInstance->GetStationByStationIdWithoutConvert(message.m_sourceStationId);
    if (station == 0)
        station = StationManager::s_pInstance->GetStation(message.m_stationAddress);
    if (station != 0)
    {
        if (station->m_stationAddress.IsValid())
            SendDisconnectionResponse(station->m_stationAddress);
        if (g_piaTransport->m_relayRouteManager != 0
            && static_cast<int>(station->m_field10) <= StationManager::cStationNumMax - 1)
            g_piaTransport->m_relayRouteManager->SearchRefugeRelayRoute(static_cast<StationIndex>(station->m_field10));
        station->m_field20 = 6;
        return;
    }
    if (message.m_stationAddress.IsValid())
        SendDisconnectionResponse(message.m_stationAddress);
}

void StationProtocol::ParseDisconnectionResponse(const ReceivedMessageAccessor& message)
{
    if (StationManager::s_pInstance->m_localStation == 0)
        return;
    Station* station = StationManager::s_pInstance->GetStationByStationIdWithoutConvert(message.m_sourceStationId);
    if (station == 0)
        station = StationManager::s_pInstance->GetStation(message.m_stationAddress);
    if (station != 0 && station->m_disconnectStationJob->m_waiting)
        station->m_disconnectStationJob->m_waiting = false;
}

void StationProtocol::ParseAck(const ReceivedMessageAccessor& message)
{
    if (StationManager::s_pInstance->m_localStation == 0)
        return;
    unsigned int ackId = common::deserializeU32(message.m_message + 4);
    Station* station = StationManager::s_pInstance->GetStation(message.m_stationAddress);
    if (station == 0)
    {
        g_piaResendingMessageManager->StopResending(ackId);
        return;
    }
    g_piaResendingMessageManager->StopResending(ackId);
    if (static_cast<int>(station->m_field10) > StationManager::cStationNumMax - 1)
    {
        const common::Time now = g_piaTransport->m_time;
        station->m_lastReceiveTime = now;
    }
}

void StationProtocol::ParseRelayConnectionRequest(const ReceivedMessageAccessor& message)
{
    ParseConnectionRequestCommon(message, true);
}

void StationProtocol::ParseRelayConnectionResponse(const ReceivedMessageAccessor& message)
{
    ParseConnectionResponseCommon(message, true);
}

Result StationProtocol::ParseHelper(const ReceivedMessageAccessor& message)
{
    if (message.m_messageSize == 0)
        return 0x2c03;
    switch (message.m_message[0])
    {
    case cMessageTypeConnectionRequest:
        ParseConnectionRequest(message);
        break;
    case cMessageTypeConnectionResponse:
        ParseConnectionResponse(message);
        break;
    case cMessageTypeDisconnectionRequest:
        ParseDisconnectionRequest(message);
        break;
    case cMessageTypeDisconnectionResponse:
        ParseDisconnectionResponse(message);
        break;
    case cMessageTypeAck:
        ParseAck(message);
        break;
    case cMessageTypeRelayConnectionRequest:
        ParseRelayConnectionRequest(message);
        break;
    case cMessageTypeRelayConnectionResponse:
        ParseRelayConnectionResponse(message);
        break;
    default:
        return 0x2c03;
    }
    return 0;
}

Result StationProtocol::Dispatch()
{
    ReceivedMessageAccessor message;
    ProtocolId protocolId = mProtocolId;
    PacketHandler::Iterator& iterator = mStationPacketHandler->GetIterator(protocolId);
    while (!iterator.IsEndIteration())
    {
        ProtocolMessageReader* reader = iterator.GetMessageReader();
        message.m_message = reader->m_message + ProtocolMessageReader::cMessageHeaderSize;
        message.m_messageSize = reader->m_messageSize;
        message.m_sourceStationId = reader->m_header.m_source;
        message.m_stationAddress = reader->m_stationAddress;
        message.m_field1c = reader->m_sendingPassId;
        Result result = ParseHelper(message);
        if (result != 0)
            return result;
        iterator.NextIteration();
    }
    return 0;
}

unsigned int StationProtocol::SendSimpleMessage(unsigned char messageType, StationIndex stationIndex, bool relay)
{
    ProtocolId protocolId = mProtocolId;
    StationPacketHandler* handler = static_cast<StationPacketHandler*>(GetStationPacketHandler());
    ProtocolMessageWriter* writer = handler->AssignByStationIndex(protocolId, stationIndex, 1, relay, false, true);
    if (writer == 0)
        return 0;
    writer->WriteU8(messageType, 0);
    Result result = mStationPacketHandler->Commit();
    if (result != 0)
        return 0;
    return 1;
}

void StationProtocol::SendDisconnectionRequest(StationIndex stationIndex, bool relay)
{
    SendSimpleMessage(cMessageTypeDisconnectionRequest, stationIndex, relay);
}

void StationProtocol::SendDisconnectionRequest(const common::StationAddress& address)
{
    SendSimpleMessage(cMessageTypeDisconnectionRequest, address);
}

unsigned int StationProtocol::GetConnectionRequestDataSize() const
{
    return g_stationConnectionInfoTable->GetLocalStationConnectionInfo().GetSerializedSize()
           + cConnectionRequestHeaderSize;
}

void StationProtocol::MakeConnectionRequestData(unsigned char* buffer, unsigned char nonce,
                                                unsigned long long principalId, unsigned int infoField,
                                                unsigned char field55, bool relay, bool secure) const
{
    buffer[0] = relay ? cMessageTypeRelayConnectionRequest : cMessageTypeConnectionRequest;
    buffer[1] = nonce;
    buffer[2] = 8;
    buffer[3] = secure;
    buffer[4] = static_cast<unsigned char>(principalId >> 56);
    buffer[5] = static_cast<unsigned char>((principalId >> 48) & 0xff);
    buffer[6] = static_cast<unsigned char>((principalId >> 40) & 0xff);
    buffer[7] = static_cast<unsigned char>((principalId >> 32) & 0xff);
    buffer[8] = static_cast<unsigned char>((principalId >> 24) & 0xff);
    buffer[9] = static_cast<unsigned char>((principalId >> 16) & 0xff);
    buffer[10] = static_cast<unsigned char>((principalId >> 8) & 0xff);
    buffer[11] = static_cast<unsigned char>(principalId);
    buffer[12] = static_cast<unsigned char>(infoField >> 24);
    buffer[13] = static_cast<unsigned char>((infoField >> 16) & 0xff);
    buffer[14] = static_cast<unsigned char>((infoField >> 8) & 0xff);
    buffer[15] = static_cast<unsigned char>(infoField);
    buffer[16] = field55;
    unsigned int serializedSize;
    g_stationConnectionInfoTable->GetLocalStationConnectionInfo().Serialize(
        buffer + cConnectionRequestHeaderSize, &serializedSize,
        GetConnectionRequestDataSize() - cConnectionRequestHeaderSize);
}

void StationProtocol::MakeConnectionResponseData(unsigned char* buffer, unsigned char section,
                                                 unsigned long long principalId, unsigned int infoField,
                                                 bool relay) const
{
    IdentificationInfo info;
    IdentificationInfoTable::GetInstance()->GetLocalIdentificationInfo(&info);
    buffer[0] = relay ? cMessageTypeRelayConnectionResponse : cMessageTypeConnectionResponse;
    buffer[1] = 0;
    buffer[2] = 8;
    buffer[4] = section;
    buffer[5] = static_cast<unsigned char>(principalId >> 56);
    buffer[6] = static_cast<unsigned char>((principalId >> 48) & 0xff);
    buffer[7] = static_cast<unsigned char>((principalId >> 40) & 0xff);
    buffer[8] = static_cast<unsigned char>((principalId >> 32) & 0xff);
    buffer[9] = static_cast<unsigned char>((principalId >> 24) & 0xff);
    buffer[10] = static_cast<unsigned char>((principalId >> 16) & 0xff);
    buffer[11] = static_cast<unsigned char>((principalId >> 8) & 0xff);
    buffer[12] = static_cast<unsigned char>(principalId);
    buffer[13] = static_cast<unsigned char>(infoField >> 24);
    buffer[14] = static_cast<unsigned char>((infoField >> 16) & 0xff);
    buffer[15] = static_cast<unsigned char>((infoField >> 8) & 0xff);
    buffer[16] = static_cast<unsigned char>(infoField);
    buffer[3] = info.m_field109;
    OSBlockMove(buffer + cResponseTokenOffset, info.m_field00.m_token, sizeof(info.m_field00.m_token), FALSE);
    common::serializeU32(buffer + cResponseInfoFieldOffset, info.m_field10c);
    buffer[cResponsePlayerCountOffset] = info.m_playerCount;
    buffer[cResponseParticipantCountOffset] = info.m_participantCount;
    if (info.m_field108 == cResponseFlagUnset && info.m_field60[0].GetBytes() <= 0 && info.m_fieldC8[0].GetBytes() <= 0)
    {
        StationProtocol::Trace(cTraceStationProtocol);
    }
    buffer[cResponseEntryCountOffset] = 1;
    unsigned int first = section == 2 ? 2 : 0;
    int remaining;
    if (section == 0)
    {
        remaining = 1;
    }
    else
    {
        remaining = 3 - section * 2;
        if (remaining > 2)
            remaining = 2;
    }
    const unsigned char* flags = &info.m_field108;
    for (unsigned int index = first; remaining > 0; --remaining, ++index)
    {
        unsigned int offset = cResponseHeaderSize + (index - first) * cResponseEntrySize;
        const common::String& name = info.m_field60[index];
        if (name.GetEncodingType() == common::String::EncodingType_Utf16)
        {
            const unsigned short* characters = static_cast<const unsigned short*>(name.GetBuffer());
            for (int n = 0; n < cResponseNameLength / 2; ++n)
                common::serializeU16(buffer + offset + n * 2, *characters++);
        }
        else
        {
            const char* characters = name.GetCharArray();
            for (int n = 0; n < cResponseNameLength; ++n)
                common::serializeU8(buffer + offset + n, *characters++);
        }
        offset += cResponseNameLength;
        buffer[offset++] = name.GetEncodingType();
        const common::String& secondName = info.m_fieldC8[index];
        if (secondName.GetEncodingType() == common::String::EncodingType_Utf16)
        {
            const unsigned short* characters = static_cast<const unsigned short*>(secondName.GetBuffer());
            for (int n = 0; n < cResponseSecondNameLength / 2; ++n)
                common::serializeU16(buffer + offset + n * 2, *characters++);
        }
        else
        {
            const char* characters = secondName.GetCharArray();
            for (int n = 0; n < cResponseSecondNameLength; ++n)
                common::serializeU8(buffer + offset + n, *characters++);
        }
        offset += cResponseSecondNameLength;
        buffer[offset++] = secondName.GetEncodingType();
        buffer[offset++] = flags[index];
        for (int n = 0; n < cResponseBytesLength; ++n)
            buffer[offset++] = info.m_field20.m_bytes[index * cResponseBytesLength + n];
        common::serializeU64(buffer + offset, info.m_principalId[index]);
    }
}

void StationProtocol::Trace(unsigned long long) const
{
}

void StationProtocol::Finalize()
{
}

unsigned int StationProtocol::IsProtocolFilteringEnabled() const
{
    return 0;
}

unsigned short StationProtocol::GetProtocolType() const
{
    return cProtocolType;
}

}
}
}
