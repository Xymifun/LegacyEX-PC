#include "nn/pia/transport/PacketHandler.hpp"

#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/transport/PacketAnalyzer.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/Common.hpp"

#include <coreinit/memory.h>
#include <new>

namespace nn
{
namespace pia
{
namespace transport
{

PacketHandler::PacketHandler()
    : m_writer(0),
      m_reader(0),
      m_pendingWriter(0),
      m_iterator(this),
      m_sendPacketAnalyzer(0),
      m_sendUnicastPacketAnalyzer(0),
      m_sendBroadcastPacketAnalyzer(0),
      m_receivePacketAnalyzer(0),
      m_stationId(0),
      m_packetInput(0),
      m_packetOutput(0)
{
}

bool PacketHandler::checkDerivedRuntimeTypeInfo(const pead::RuntimeTypeInfo::Interface* type) const
{
    return checkDerivedRuntimeTypeInfoStatic(type);
}

const pead::RuntimeTypeInfo::Interface* PacketHandler::getRuntimeTypeInfo() const
{
    return getRuntimeTypeInfoStatic();
}

PacketHandler::~PacketHandler()
{
}

ProtocolMessageReader* PacketHandler::Iterator::GetMessageReader() const
{
    return m_handler->m_protocolMessageReader.m_message != 0 ? &m_handler->m_protocolMessageReader : 0;
}

PacketHandler::Iterator& PacketHandler::GetIterator(const ProtocolId& protocolId)
{
    m_field35c = protocolId.value();
    m_field35e = 0xffff;
    BeginIteration();
    return m_iterator;
}

PacketHandler::Iterator& PacketHandler::GetIterator(unsigned char protocolType)
{
    m_field35e = 0xff00;
    m_field35c = ProtocolId(protocolType, 0);
    BeginIteration();
    return m_iterator;
}

Result PacketHandler::Initialize(unsigned int maxProtocolNum)
{
    m_writer = 0;
    m_reader = 0;
    m_payloadSize = 0;
    m_packetSize = 0;
    m_sendPacketAnalyzer = common::NewObj<PacketAnalyzer>("Pia Send", maxProtocolNum);
    if (m_sendPacketAnalyzer == 0)
        return cResultOutOfMemory;
    m_sendUnicastPacketAnalyzer = common::NewObj<PacketAnalyzer>("Pia Send Unicast", maxProtocolNum);
    if (m_sendUnicastPacketAnalyzer == 0)
        return cResultOutOfMemory;
    m_sendBroadcastPacketAnalyzer = common::NewObj<PacketAnalyzer>("Pia Send Broadcast", maxProtocolNum);
    if (m_sendBroadcastPacketAnalyzer == 0)
        return cResultOutOfMemory;
    m_receivePacketAnalyzer = common::NewObj<PacketAnalyzer>("Pia Receive", maxProtocolNum);
    if (m_receivePacketAnalyzer == 0)
        return cResultOutOfMemory;
    m_cryptoSetting.m_mode = 0;
    return 0;
}

void PacketHandler::Finalize()
{
    if (m_receivePacketAnalyzer != 0)
    {
        common::DeleteObj(m_receivePacketAnalyzer);
        m_receivePacketAnalyzer = 0;
    }
    if (m_sendBroadcastPacketAnalyzer != 0)
    {
        common::DeleteObj(m_sendBroadcastPacketAnalyzer);
        m_sendBroadcastPacketAnalyzer = 0;
    }
    if (m_sendUnicastPacketAnalyzer != 0)
    {
        common::DeleteObj(m_sendUnicastPacketAnalyzer);
        m_sendUnicastPacketAnalyzer = 0;
    }
    if (m_sendPacketAnalyzer != 0)
    {
        common::DeleteObj(m_sendPacketAnalyzer);
        m_sendPacketAnalyzer = 0;
    }
    m_writer = 0;
    m_reader = 0;
    m_payloadSize = 0;
    m_packetSize = 0;
    m_cryptoSetting = common::CryptoSetting();
}

void PacketHandler::Cleanup()
{
    if (m_receivePacketAnalyzer != 0)
        m_receivePacketAnalyzer->Cleanup();
    if (m_sendBroadcastPacketAnalyzer != 0)
        m_sendBroadcastPacketAnalyzer->Cleanup();
    if (m_sendUnicastPacketAnalyzer != 0)
        m_sendUnicastPacketAnalyzer->Cleanup();
    if (m_sendPacketAnalyzer != 0)
        m_sendPacketAnalyzer->Cleanup();
    m_writer = 0;
    m_reader = 0;
    m_payloadSize = 0;
    m_packetSize = 0;
    m_cryptoSetting = common::CryptoSetting();
}

Result PacketHandler::Startup(PacketStream::Writer* writer, PacketStream::Reader* reader,
                              common::IPacketInput* packetInput, common::IPacketOutput* packetOutput,
                              unsigned long long* stationId, const common::CryptoSetting* cryptoSetting)
{
    unsigned int packetSize = (common::g_payloadSizeManager->m_mtuSize & ~3u)
                              - common::g_payloadSizeManager->m_signatureSize;

    if (writer == 0 || reader == 0 || packetInput == 0 || packetOutput == 0 || stationId == 0)
        return cResultInvalidArgument;
    if (cryptoSetting != 0 && cryptoSetting->m_mode > 1)
        return cResultInvalidArgument;
    if (m_writer != 0 || m_reader != 0)
        return cResultInvalidState;

    m_packetInput = packetInput;
    m_writer = writer;
    m_reader = reader;
    m_packetOutput = packetOutput;
    m_protocolMessageWriter.SetSource(0);
    m_pendingWriter = 0;

    if (packetSize < cPacketHeaderSize || packetSize > cPacketHeaderSize + cPayloadSizeMax)
        return cResultInvalidArgument;

    if (cryptoSetting != 0)
        m_cryptoSetting = *cryptoSetting;
    else
        m_cryptoSetting = common::CryptoSetting();

    switch (m_cryptoSetting.m_mode)
    {
    case 0:
        m_packetSize = packetSize;
        break;
    case 1:
    {
        packetSize -= cPacketHeaderSize;
        const unsigned int blockSize = common::Crypto::GetBlockSize(common::Crypto::cModeAesGcm);
        m_packetSize = (packetSize / blockSize) * blockSize + cPacketHeaderSize;
        break;
    }
    }
    m_payloadSize = m_packetSize - cPacketHeaderSize;

    m_sendPacketAnalyzer->Startup();
    m_sendPacketAnalyzer->ClearPacketAnalysisData();
    m_sendUnicastPacketAnalyzer->Startup();
    m_sendUnicastPacketAnalyzer->ClearPacketAnalysisData();
    m_sendBroadcastPacketAnalyzer->Startup();
    m_sendBroadcastPacketAnalyzer->ClearPacketAnalysisData();
    m_receivePacketAnalyzer->Startup();
    m_receivePacketAnalyzer->ClearPacketAnalysisData();

    m_stationId = stationId;
    return 0;
}

Result PacketHandler::UpdateSetting(const common::CryptoSetting* cryptoSetting)
{
    unsigned int packetSize = (common::g_payloadSizeManager->m_mtuSize & ~3u)
                              - common::g_payloadSizeManager->m_signatureSize;

    if (cryptoSetting != 0 && cryptoSetting->m_mode > 1)
        return cResultInvalidArgument;

    if (cryptoSetting != 0)
        m_cryptoSetting = *cryptoSetting;
    else
        m_cryptoSetting = common::CryptoSetting();

    switch (m_cryptoSetting.m_mode)
    {
    case 0:
        m_packetSize = packetSize;
        break;
    case 1:
    {
        packetSize -= cPacketHeaderSize;
        const unsigned int blockSize = common::Crypto::GetBlockSize(common::Crypto::cModeAesGcm);
        m_packetSize = (packetSize / blockSize) * blockSize + cPacketHeaderSize;
        break;
    }
    }
    m_payloadSize = m_packetSize - cPacketHeaderSize;
    return 0;
}

int PacketHandler::GetPayloadSizeLimit() const
{
    return static_cast<int>(m_payloadSize) - 24;
}

Result PacketHandler::GetPacketAnalysisData(PacketAnalysisData* send, PacketAnalysisData* unicast,
                                            PacketAnalysisData* broadcast, PacketAnalysisData* receive) const
{
    if (send == 0 && receive == 0)
        return cResultInvalidArgument;
    if (send != 0)
    {
        if (m_sendPacketAnalyzer == 0)
            return cResultInvalidState;
        const Result result = m_sendPacketAnalyzer->GetPacketAnalysisData(send);
        if (result != 0)
            return result;
    }
    if (unicast != 0)
    {
        if (m_sendUnicastPacketAnalyzer == 0)
            return cResultInvalidState;
        const Result result = m_sendUnicastPacketAnalyzer->GetPacketAnalysisData(unicast);
        if (result != 0)
            return result;
    }
    if (broadcast != 0)
    {
        if (m_sendBroadcastPacketAnalyzer == 0)
            return cResultInvalidState;
        const Result result = m_sendBroadcastPacketAnalyzer->GetPacketAnalysisData(broadcast);
        if (result != 0)
            return result;
    }
    if (receive != 0)
    {
        if (m_receivePacketAnalyzer == 0)
            return cResultInvalidState;
        const Result result = m_receivePacketAnalyzer->GetPacketAnalysisData(receive);
        if (result != 0)
            return result;
    }
    return 0;
}

void PacketHandler::ClearPacketAnalysisData()
{
    if (m_sendPacketAnalyzer != 0)
        m_sendPacketAnalyzer->ClearPacketAnalysisData();
    if (m_sendUnicastPacketAnalyzer != 0)
        m_sendUnicastPacketAnalyzer->ClearPacketAnalysisData();
    if (m_sendBroadcastPacketAnalyzer != 0)
        m_sendBroadcastPacketAnalyzer->ClearPacketAnalysisData();
    if (m_receivePacketAnalyzer != 0)
        m_receivePacketAnalyzer->ClearPacketAnalysisData();
}

unsigned char* PacketHandler::AssignPacketPayload(common::Packet* packet, unsigned int size)
{
    unsigned char* payload = packet->AssignPayload(size);
    const unsigned int alignedSize = size & ~3U;
    *reinterpret_cast<unsigned int*>(payload + alignedSize - 4) = 0;
    return payload;
}

bool PacketHandler::IsBundleCandidate(const common::Packet* packet, bool field602, unsigned int sizeLimit,
                                      bool field604)
{
    if (packet->m_field601 != 0)
        return false;
    if (m_cryptoSetting.m_mode != 0 && packet->m_field602 != field602)
        return false;
    if (packet->m_size > sizeLimit)
        return false;
    if (packet->m_field604 != field604)
        return false;
    return true;
}

common::Packet* PacketHandler::AssignPacket(const common::StationAddress& address, bool isPriority, bool field602)
{
    common::Packet* packet = m_writer->Assign();
    if (packet == 0)
    {
        m_writer->m_packetStream->Trace(0x80000);
        return 0;
    }
    packet->Reset();
    packet->m_field5f4 = address;
    packet->m_field601 = isPriority;
    packet->m_field602 = field602;
    return packet;
}

ProtocolMessageWriter* PacketHandler::AssignByStationAddress(const ProtocolId& protocolId,
                                                             const common::StationAddress& address, unsigned int size,
                                                             bool isPriority, bool field602)
{
    ProtocolMessageWriter* writer = 0;
    if (address.IsValid() && m_writer != 0 && m_pendingWriter == 0)
    {
        const unsigned int alignedSize = ((size + 23U) & ~3U) + 4U;
        if (alignedSize <= m_payloadSize)
        {
            const unsigned int sizeLimit = m_packetSize - alignedSize;
            common::Packet* packet = 0;
            if (!isPriority)
            {
                const PacketStream::Writer* stream = m_writer;
                for (PacketStream::Accessor::Iterator it = stream->Begin(); it != stream->End(); ++it)
                {
                    common::Packet* candidate = &*it;
                    if (IsBundleCandidate(candidate, field602, sizeLimit, true) && candidate->m_field5f4 == address)
                    {
                        packet = candidate;
                        break;
                    }
                }
            }
            if (packet == 0)
                packet = AssignPacket(address, isPriority, field602);
            if (packet != 0)
            {
                unsigned char* payload = AssignPacketPayload(packet, alignedSize);
                m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
                m_protocolMessageWriter.AddMessageBuffer(packet, payload, 0, true, false);
                writer = &m_protocolMessageWriter;
                m_pendingWriter = writer;
            }
        }
    }
    return writer;
}

Result PacketHandler::Commit()
{
    if (m_pendingWriter == 0)
        return cResultInvalidState;
    m_pendingWriter->Commit();
    m_pendingWriter = 0;
    return 0;
}

ProtocolMessageWriter* PacketHandler::ReserveMessageWriter()
{
    if (m_protocolMessageWriter.m_messageBufferNum != 0)
        m_pendingWriter = &m_protocolMessageWriter;
    return m_pendingWriter;
}

void PacketHandler::Trace(unsigned long long) const
{
}

void PacketHandler::RelayMessage(const ProtocolMessageReader&)
{
}

unsigned int PacketHandler::CheckPacket(const common::Packet& packet)
{
    return packet.m_size;
}

unsigned int PacketHandler::CheckMessage(const ProtocolMessageReader&)
{
    return 1;
}

unsigned int PacketHandler::CheckReceive(const ProtocolMessageReader&)
{
    return 1;
}

unsigned int PacketHandler::IsBroadcastSupported() const
{
    return 0;
}

bool PacketHandler::CheckFiltering(const common::StationAddress& address) const
{
    Station* station = StationManager::s_pInstance->GetStation(address);
    if (station == 0)
        return false;
    Result result;
    {
        StationConnectionInfo info;
        result = g_stationConnectionInfoTable->GetStationConnectionInfo(station, &info);
    }
    return result == 0;
}

void PacketHandler::BeginIteration()
{
    m_field360 = -1;
    m_field364 = 0;
    m_protocolMessageReader.Clear();
    NextIteration();
}

unsigned int PacketHandler::GenerateConnectionId() const
{
    common::Time now;
    now.SetNow();
    return static_cast<unsigned int>(now.m_tick);
}

bool PacketHandler::IsEndIteration() const
{
    return m_field360 >= static_cast<int>(m_reader->m_field0c);
}

void PacketHandler::NextIteration()
{
    if (IsEndIteration())
        return;
    for (;;)
    {
        if (m_protocolMessageReader.m_message != 0)
        {
            common::Packet* packet = m_reader->Get(m_field360);
            m_field364 += ((m_protocolMessageReader.m_messageSize + 23U) & ~3U) + 4U;
            if (m_field364 >= packet->m_size - cPacketHeaderSize)
                m_protocolMessageReader.Clear();
            else
                m_protocolMessageReader.Attach(*packet, m_field364);
        }
        else
        {
            ++m_field360;
            m_field364 = 0;
            if (m_field360 >= static_cast<int>(m_reader->m_field0c))
                return;
            common::Packet* packet = m_reader->Get(m_field360);
            if (packet->m_field614 == 0)
                continue;
            m_protocolMessageReader.Attach(*packet, m_field364);
        }
        if (m_protocolMessageReader.m_message == 0)
            continue;
        if ((m_protocolMessageReader.m_header.m_protocolId & m_field35e) != m_field35c)
            continue;
        if (((m_protocolMessageReader.m_header.m_flags >> 2) & 1) == 0 && g_piaProtocolMessageFilteringManager != 0
            && g_piaProtocolMessageFilteringManager->IsFilteringEnabled(
                   ProtocolId(m_protocolMessageReader.m_header.m_protocolId).GetProtocolType())
                   != 0)
        {
            const common::StationAddress& address = m_protocolMessageReader.m_stationAddress;
            if (!CheckFiltering(address))
            {
                address.Trace(0x800000);
                continue;
            }
        }
        if (CheckReceive(m_protocolMessageReader) != 0)
            return;
    }
}

void PacketHandler::BeginDispatch(const common::Time&)
{
    m_reader->PullAll();
    PacketStream::Reader* reader = m_reader;
    common::Crypto::Setting setting;
    if (m_cryptoSetting.m_mode == 1)
    {
        setting.m_mode = common::Crypto::cModeAesGcm;
        setting.m_key = m_cryptoSetting.m_key;
        setting.m_keyLength = sizeof(m_cryptoSetting.m_key);
    }
    else
    {
        setting.m_mode = common::Crypto::cModeNone;
        setting.m_key = 0;
        setting.m_keyLength = 0;
    }
    for (PacketStream::Accessor::Iterator it = reader->Begin(); it != reader->End(); ++it)
    {
        common::Packet* packet = &*it;
        if (packet->IsValid() == 0)
        {
            packet->m_field614 = 0;
            continue;
        }
        if (packet->m_packetData.m_header.m_version != 1)
        {
            unsigned char parameter[12] = {};
            if (m_cryptoSetting.m_mode == 1)
            {
                if (m_packetInput == 0
                    || m_packetInput->CalculateCryptoParameter(parameter, sizeof(parameter), packet) != 0)
                {
                    packet->m_field614 = 0;
                    continue;
                }
                setting.m_iv = parameter;
                setting.m_ivLength = sizeof(parameter);
            }
            if (packet->Decrypt(setting) != 0 || packet->m_packetData.m_header.m_version != 1)
            {
                packet->m_field614 = 0;
                continue;
            }
        }
        const unsigned int size = CheckPacket(*packet);
        if (size < cPacketHeaderSize)
        {
            packet->m_field614 = 0;
            continue;
        }
        packet->m_size = size;
        unsigned char accepted = 1;
        unsigned char anyAccepted = 0;
        unsigned char hasRelay = 0;
        unsigned int offset = 0;
        while (offset < packet->m_size - cPacketHeaderSize)
        {
            m_protocolMessageReader.Attach(*packet, offset);
            if (m_protocolMessageReader.m_message == 0)
            {
                if (!m_protocolMessageReader.m_isTerminator)
                    accepted = 0;
                break;
            }
            offset += ((m_protocolMessageReader.m_messageSize + 23U) & ~3U) + 4U;
            const bool broadcast = IsBroadcastSupported() != 0;
            const unsigned char result = static_cast<unsigned char>(CheckMessage(m_protocolMessageReader));
            if (broadcast)
                anyAccepted |= result;
            else
            {
                accepted &= result;
                if (accepted == 0)
                    break;
            }
            hasRelay |= (m_protocolMessageReader.m_header.m_flags >> 1) & 1;
        }
        if (IsBroadcastSupported() != 0)
            accepted &= anyAccepted;
        packet->m_field614 = accepted;
        if (accepted != 0 && hasRelay != 0)
        {
            for (offset = 0; offset < packet->m_size - cPacketHeaderSize;
                 offset += ((m_protocolMessageReader.m_messageSize + 23U) & ~3U) + 4U)
            {
                m_protocolMessageReader.Attach(*packet, offset);
                if (m_protocolMessageReader.m_message == 0)
                    break;
                if ((m_protocolMessageReader.m_header.m_flags & 2) != 0)
                    RelayMessage(m_protocolMessageReader);
            }
        }
    }
    if (m_receivePacketAnalyzer != 0)
    {
        reader = m_reader;
        for (PacketStream::Accessor::Iterator it = reader->Begin(); it != reader->End(); ++it)
            m_receivePacketAnalyzer->UpdatePacketAnalysisData(*it, false, false);
    }
}

void PacketHandler::EndDispatch(const common::Time&)
{
    if (m_pendingWriter != 0)
        return;
    if (m_sendPacketAnalyzer != 0)
    {
        for (PacketStream::Accessor::Iterator it = m_writer->Begin(); it != m_writer->End(); ++it)
        {
            const bool broadcast = it->m_stationIndex == 0xff;
            m_sendPacketAnalyzer->UpdatePacketAnalysisData(*it, true, broadcast);
            if (broadcast)
            {
                if (m_sendBroadcastPacketAnalyzer != 0)
                    m_sendBroadcastPacketAnalyzer->UpdatePacketAnalysisData(*it, true, true);
            }
            else if (m_sendUnicastPacketAnalyzer != 0)
            {
                m_sendUnicastPacketAnalyzer->UpdatePacketAnalysisData(*it, true, false);
            }
        }
    }
    if (m_cryptoSetting.m_mode != 0)
    {
        common::Crypto::Setting setting;
        if (m_cryptoSetting.m_mode == 1)
        {
            setting.m_mode = common::Crypto::cModeAesGcm;
            setting.m_key = m_cryptoSetting.m_key;
            setting.m_keyLength = sizeof(m_cryptoSetting.m_key);
        }
        for (PacketStream::Accessor::Iterator it = m_writer->Begin(); it != m_writer->End(); ++it)
        {
            if (it->m_field602 != 0)
                continue;
            common::Time now;
            now.SetNow();
            unsigned char parameter[12];
            const unsigned long long tick = now.m_tick;
            OSBlockMove(parameter, &tick, sizeof(tick), FALSE);
            if (m_cryptoSetting.m_mode == 1 && m_packetOutput != 0)
            {
                ++*m_stationId;
                common::serializeU64(it->m_packetData.m_header.m_cryptoNonce, *m_stationId);
                m_packetOutput->CalculateCryptoParameter(parameter, sizeof(parameter), &*it);
            }
            setting.m_iv = parameter;
            setting.m_ivLength = sizeof(parameter);
            it->Encrypt(setting);
        }
    }
    m_reader->Release();
    m_writer->Push();
}

}
}
}
