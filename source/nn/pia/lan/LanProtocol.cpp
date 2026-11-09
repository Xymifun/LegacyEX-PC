#include "nn/pia/lan/LanProtocol.hpp"

#include <new>
#include <stdint.h>

#include <coreinit/memory.h>

#include "nn/pia/common/AesImpl.hpp"
#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Hmac.hpp"
#include "nn/pia/common/InetAddress.hpp"
#include "nn/pia/common/Sha256Context.hpp"
#include "nn/pia/common/Socket.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/common.hpp"
#include "nn/pia/lan/LanInputStream.hpp"
#include "nn/pia/lan/LanGetHostMessage.hpp"
#include "nn/pia/lan/LanGetSessionMessage.hpp"
#include "nn/pia/lan/LanHostMessage.hpp"
#include "nn/pia/lan/LanMatchmakeSession.hpp"
#include "nn/pia/lan/LanMatchmakeUpdateJob.hpp"
#include "nn/pia/lan/LanOutputStream.hpp"
#include "nn/pia/lan/LanSelfKeepAliveMessage.hpp"
#include "nn/pia/lan/LanSessionMessage.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshLayerController.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn
{
namespace pia
{
namespace lan
{

namespace
{
const unsigned int cSelfKeepAliveIntervalMsec = 2000;
const unsigned int cRequestTimeoutMsec = 2000;
const unsigned int cSelfKeepAliveMessageType = 7;
const unsigned int cGetHostMessageType = 3;
const unsigned int cHostMessageType = 4;
const unsigned int cGetSessionMessageType = 5;
const unsigned int cSessionMessageType = 6;
const unsigned int cKeySize = 16;
const unsigned int cMatchmakeUpdateJobSize = 0xe90;
const unsigned int cSessionFragmentSize = 800;
const unsigned int cHostMessageHeaderSize = 0x10;
const unsigned int cSessionMessageHeaderSize = 0x18;
const unsigned int cMeshStateEstablished = 1;
const unsigned short cPortBase = 0xc000;
const int cPortCount = 4;
const int cBindAttemptCount = 4;
const unsigned short cBroadcastPort = 40000;
const unsigned int cStopWaitMsec = 5;
const int cMessageSendCount = 3;
enum
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultRequestTimeout = 0x6c4b
};
const unsigned long long cTraceBrief = 0x0100000000000000ULL;
const unsigned long long cTraceDetail = 0x0200000000000000ULL;
unsigned short s_nextPort = cPortBase;

inline LanMatchmakeSession* GetCurrentMatchmakeSession()
{
    return static_cast<LanMatchmakeSession*>(
        session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex]);
}

template <typename T> T* newObject()
{
    void* storage = pead::allocateFromSeadHeap(sizeof(T), common::HeapManager::GetHeap());
    return storage == 0 ? 0 : new (storage) T();
}

template <typename T> void deleteObject(T* object)
{
    object->~T();
    pead::freeToSeadHeap(object);
}
}

void LanProtocol::ReceivedFragmentData::Clear()
{
    OSBlockSet(m_data, 0, sizeof(m_data));
    m_id = 0;
    m_sequence = 0;
    m_receivedMask = 0;
    m_fragmentCount = 0;
    m_source.Clear();
}

void LanProtocol::ReceivedFragmentData::Receive(const void* data, unsigned int size, unsigned int id,
                                                unsigned short sequence, unsigned char index, unsigned char count,
                                                const common::StationAddress& source)
{
    if (m_id != id || m_sequence != sequence || m_fragmentCount != count || !(source == m_source))
    {
        Clear();
        m_id = id;
        m_sequence = sequence;
        m_fragmentCount = count;
        m_source = source;
    }
    OSBlockMove(m_data + m_fragmentSize * index, data, size, FALSE);
    m_receivedMask |= static_cast<unsigned char>(1 << index);
}

bool LanProtocol::ReceivedFragmentData::IsReceivedAll() const
{
    return m_receivedMask == (1 << m_fragmentCount) - 1;
}

LanProtocol::LanProtocol()
    : transport::Protocol(),
      m_started(0),
      m_address(0),
      m_broadcastAddress(0),
      m_port(0),
      m_filterPort(0),
      m_unknown24(0),
      m_socket(0),
      m_broadcastSocket(0),
      m_broadcastSocketBound(0),
      m_outputStream(0),
      m_inputStream(0),
      m_matchmakeUpdateJob(0),
      m_unknown618(0),
      m_receivedSessionInfo(),
      m_sessionMessageId(0),
      m_getHostContext(),
      m_getHostDeadline(),
      m_getSessionContext(),
      m_getSessionDeadline(),
      m_updateSessionContext(),
      m_selfKeepAliveTime(),
      m_cryptoSetting(),
      m_random()
{
    LanFacade::GetInstance()->RegisterLanProtocol(this);
    static_cast<LanInputStream*>(transport::g_piaThreadStreamManager->m_packetInput)->RegisterLanProtocol(this);
    static_cast<LanOutputStream*>(transport::g_piaThreadStreamManager->m_packetOutput)->RegisterLanProtocol(this);
}

LanProtocol::~LanProtocol()
{
    if (transport::g_piaThreadStreamManager != 0)
    {
        LanOutputStream* output = static_cast<LanOutputStream*>(transport::g_piaThreadStreamManager->m_packetOutput);
        if (output != 0)
            output->UnregisterLanProtocol();
        LanInputStream* input = static_cast<LanInputStream*>(transport::g_piaThreadStreamManager->m_packetInput);
        if (input != 0)
            input->UnregisterLanProtocol();
    }
    LanFacade::GetInstance()->Cleanup();
    LanFacade::GetInstance()->UnregisterLanProtocol();
}

Result LanProtocol::Initialize()
{
    m_started = 0;
    m_socket = newObject<common::Socket>();
    m_broadcastSocket = newObject<common::Socket>();
    void* storage = pead::allocateFromSeadHeap(cMatchmakeUpdateJobSize, common::HeapManager::GetHeap());
    if (storage != 0)
        m_matchmakeUpdateJob = new (storage) LanMatchmakeUpdateJob(this);
    else
        m_matchmakeUpdateJob = 0;
    m_cryptoSetting.m_mode = 1;
    m_sessionKeyRegistered = 0;
    m_sessionKeyRequested = 0;
    m_cryptoSettingRegistered = 0;
    m_randomSeedSet = 0;
    m_unknown24 = 0;
    m_broadcastSocketBound = 0;
    return 0;
}

void LanProtocol::Finalize()
{
    if (m_socket != 0)
    {
        deleteObject(m_socket);
        m_socket = 0;
    }
    if (m_broadcastSocket != 0)
    {
        deleteObject(m_broadcastSocket);
        m_broadcastSocket = 0;
    }
    if (m_matchmakeUpdateJob != 0)
    {
        m_matchmakeUpdateJob->~LanMatchmakeUpdateJob();
        pead::freeToSeadHeap(m_matchmakeUpdateJob);
        m_matchmakeUpdateJob = 0;
    }
}

void LanProtocol::Trace(unsigned long long) const
{
}

unsigned short LanProtocol::GetProtocolType() const
{
    return cProtocolType;
}

unsigned int LanProtocol::IsProtocolFilteringEnabled() const
{
    return 0;
}

void LanProtocol::ResetSelfKeepAliveTime()
{
    const common::TimeSpan interval = common::TimeSpan::FromMSec(cSelfKeepAliveIntervalMsec);
    common::Time now;
    now.SetNow();
    m_selfKeepAliveTime.m_tick = now.m_tick + interval.m_tick;
}

Result LanProtocol::Startup()
{
    m_getHostContext.Reset();
    m_getSessionContext.Reset();
    m_updateSessionContext.Reset();
    ResetSelfKeepAliveTime();
    m_unknown618 = 0;
    m_receivedSessionInfo.Clear();
    m_sessionMessageId = 0;
    return 0;
}

void LanProtocol::Cleanup()
{
    if (m_getHostContext.m_state == common::CallContext::cStateInProgress)
        m_getHostContext.SignalCancel();
    if (m_getSessionContext.m_state == common::CallContext::cStateInProgress)
        m_getSessionContext.SignalCancel();
    if (m_updateSessionContext.m_state == common::CallContext::cStateInProgress)
        m_updateSessionContext.SignalCancel();
}

bool LanProtocol::IsCryptoEnabled() const
{
    return m_cryptoSetting.m_mode != 0;
}

void LanProtocol::SendMessage(LanPayload* payload, const common::StationAddress& address, bool isPriority,
                              bool field602, unsigned long long)
{
    if (!IsCryptoEnabled() || m_sessionKeyRegistered != 0 || payload->m_type == cSelfKeepAliveMessageType)
    {
        transport::ProtocolMessageWriter* writer = mStationPacketHandler->AssignByStationAddress(
            transport::ProtocolId(cProtocolType, 0), address, payload->m_size, isPriority, field602);
        if (writer != 0)
        {
            writer->SetPayload(payload->m_data);
            mStationPacketHandler->Commit();
        }
    }
}

void LanProtocol::ParseSelfKeepAlive(const LanPayload* payload, const common::StationAddress&)
{
    LanSelfKeepAliveMessage message;
    message.Deserialize(payload->m_data);
}

void LanProtocol::ParseMessage(const LanPayload* payload, const common::StationAddress& source)
{
    switch (payload->m_type)
    {
    case cGetHostMessageType:
        ParseGetHostMessage(payload, source);
        break;
    case cHostMessageType:
        ParseHostMessage(payload, source);
        break;
    case cGetSessionMessageType:
        ParseGetSessionMessage(payload, source);
        break;
    case cSessionMessageType:
        ParseSessionMessage(payload, source);
        break;
    case cSelfKeepAliveMessageType:
        ParseSelfKeepAlive(payload, source);
        break;
    }
}

Result LanProtocol::Bind(const LanFacade::LanNetworkSetting& setting)
{
    if (setting.m_field00 == 0)
        return 0x10c07;

    m_address = setting.m_field00;
    m_broadcastAddress = (setting.m_field00 & setting.m_field04) | ~setting.m_field04;
    m_unknown24 = setting.m_field08;
    return 0;
}

void LanProtocol::Unbind()
{
    m_address = 0;
    m_broadcastAddress = 0;
    m_port = 0;
    m_filterPort = 0;
    m_unknown24 = 0;
}

void LanProtocol::RegisterCryptoSetting(const common::CryptoSetting& setting)
{
    m_cryptoSetting.m_mode = setting.m_mode;
    OSBlockMove(m_cryptoSetting.m_key, setting.m_key, cKeySize, FALSE);
    m_cryptoSettingRegistered = 1;
}

void LanProtocol::UnregisterCryptoSetting()
{
    m_cryptoSetting.m_mode = common::Crypto::cModeAesGcm;
    OSBlockSet(m_cryptoSetting.m_key, 0, cKeySize);
    m_cryptoSettingRegistered = 0;
}

unsigned int LanProtocol::RequestBrowseMessage(unsigned char type)
{
    return m_matchmakeUpdateJob->RequestBrowseMessage(type);
}

bool LanProtocol::IsBrowseMessageCompleted() const
{
    return m_matchmakeUpdateJob->IsBrowseMessageCompleted();
}

bool LanProtocol::RequestGetHostMessage()
{
    if (m_getHostContext.m_state != common::CallContext::cStateInProgress)
    {
        m_getHostContext.Reset();
        m_getHostContext.InitiateCall();
        if (!session::g_piaMesh->IsProcessingLeaveMesh())
            SendGetHostMessage();
        const common::TimeSpan interval = common::TimeSpan::FromMSec(cRequestTimeoutMsec);
        common::Time now;
        now.SetNow();
        m_getHostDeadline.m_tick = now.m_tick + interval.m_tick;
        return true;
    }
    return false;
}

bool LanProtocol::IsGetHostMessageCompleted() const
{
    return m_getHostContext.m_state == common::CallContext::cStateSuccess
           || m_getHostContext.m_state == common::CallContext::cStateFailure
           || m_getHostContext.m_state == common::CallContext::cStateCancelled;
}

bool LanProtocol::RequestUpdateSessionMessage()
{
    if (m_updateSessionContext.m_state != common::CallContext::cStateInProgress)
    {
        m_updateSessionContext.Reset();
        m_updateSessionContext.InitiateCall();
        return true;
    }
    return false;
}

bool LanProtocol::IsUpdateSessionMessageCompleted() const
{
    return m_updateSessionContext.m_state == common::CallContext::cStateSuccess
           || m_updateSessionContext.m_state == common::CallContext::cStateFailure
           || m_updateSessionContext.m_state == common::CallContext::cStateCancelled;
}

bool LanProtocol::RequestGetSessionMessage()
{
    if (m_getSessionContext.m_state != common::CallContext::cStateInProgress)
    {
        m_getSessionContext.Reset();
        m_getSessionContext.InitiateCall();
        SendGetSessionMessage();
        const common::TimeSpan interval = common::TimeSpan::FromMSec(cRequestTimeoutMsec);
        common::Time now;
        now.SetNow();
        m_getSessionDeadline.m_tick = now.m_tick + interval.m_tick;
        return true;
    }
    return false;
}

bool LanProtocol::IsGetSessionMessageCompleted() const
{
    return m_getSessionContext.m_state == common::CallContext::cStateSuccess
           || m_getSessionContext.m_state == common::CallContext::cStateFailure
           || m_getSessionContext.m_state == common::CallContext::cStateCancelled;
}

Result LanProtocol::GetSessionMessageResult() const
{
    return m_getSessionContext.m_result;
}

Result LanProtocol::CalculateCryptionKeyForBrowse(const unsigned char* input, unsigned int inputSize, void* output,
                                                  unsigned int outputSize)
{
    if (input == 0 || inputSize != cKeySize || output == 0 || outputSize != cKeySize)
        return 0x10c07;
    if (m_cryptoSetting.m_mode == 0)
        return 0x10c08;

    common::AesImpl<16> aes;
    aes.Initialize(m_cryptoSetting.m_key, cKeySize, true);
    aes.EncryptBlock(output, cKeySize, input, cKeySize);
    return 0;
}

unsigned long long LanProtocol::GetLocalPrincipalId() const
{
    return transport::g_stationConnectionInfoTable->m_localStationConnectionInfo.m_stationLocation.m_principalId;
}

Result LanProtocol::BindRandomPort()
{
    if (m_socket == 0)
        return cResultInvalidArgument;

    for (int attempt = 0; attempt < cBindAttemptCount; ++attempt)
    {
        Result openResult = m_socket->Open();
        if (openResult != 0)
            return cResultInvalidState;

        const unsigned short port = s_nextPort;
        ++s_nextPort;
        if (s_nextPort > cPortBase + cPortCount - 1)
            s_nextPort = cPortBase;

        Result bindResult = m_socket->Bind(port, true);
        if (bindResult == 0)
        {
            m_port = port;
            common::StationAddress address;
            address.SetInetAddress(common::InetAddress(m_address, m_port));
            transport::StationConnectionInfo info;
            info.m_stationLocation.SetStationAddress(address);
            info.m_stationLocation.m_field24 = 2;
            info.m_stationLocation.m_field23 = 1;
            info.m_stationLocation.m_field25 = 3;
            info.m_stationLocation.m_field18 = transport::g_piaTransport->GenerateConnectionId();
            info.m_stationLocation.m_principalId = (static_cast<unsigned long long>(m_address) << 32) | m_port;
            info.m_stationLocation.m_field1c = (m_address << 16) | m_port;
            info.m_localStationLocation.SetStationLocation(info.m_stationLocation);
            info.m_localStationLocation.m_field25 = 0;
            transport::StationConnectionInfo& localInfo =
                transport::g_stationConnectionInfoTable->m_localStationConnectionInfo;
            localInfo = info;
            m_inputStream->AddIgnoreAddress(address.m_inetAddress);

            Result broadcastOpenResult = m_broadcastSocket->Open();
            if (broadcastOpenResult == 0)
            {
                Result broadcastBindResult = m_broadcastSocket->Bind(cBroadcastPort, true);
                if (broadcastBindResult == 0)
                {
                    m_broadcastSocketBound = 1;
                }
                else
                {
                    m_broadcastSocketBound = 0;
                    m_broadcastSocket->Close();
                }
            }
            else
            {
                m_broadcastSocketBound = 0;
            }
            return 0;
        }
        m_socket->Close();
    }
    return cResultInvalidState;
}

Result LanProtocol::StartLanSession()
{
    if (transport::g_piaTransport == 0)
        return cResultInvalidState;

    m_outputStream = static_cast<LanOutputStream*>(transport::g_piaTransport->GetOutputStream());
    if (m_outputStream == 0)
        return cResultInvalidState;

    m_inputStream = static_cast<LanInputStream*>(transport::g_piaTransport->GetInputStream());
    if (m_inputStream == 0)
        return cResultInvalidState;

    if (m_address == 0)
        return cResultInvalidState;

    if (IsCryptoEnabled())
    {
        common::CryptoSetting setting;
        setting.m_mode = m_cryptoSetting.m_mode;
        pead::Random random;
        for (unsigned int index = 0; index < cKeySize / sizeof(unsigned int); ++index)
            common::serializeU32(setting.m_key + index * sizeof(unsigned int), random.getU32());
        transport::g_piaTransport->UpdateSetting(&setting);
    }
    m_sessionKeyRegistered = 0;
    m_sessionKeyRequested = 0;

    Result result = BindRandomPort();
    if (result != 0)
        return result;

    result = m_outputStream->ImportSocket(m_socket);
    if (result != 0)
        return result;

    result = m_inputStream->ImportSocket(m_socket);
    if (result != 0)
        return result;

    if (m_broadcastSocketBound != 0)
    {
        Result broadcastResult = m_inputStream->ImportBroadcastSocket(m_broadcastSocket);
        if (broadcastResult != 0)
            return broadcastResult;
    }

    result = m_outputStream->Startup();
    if (result != 0)
        return result;

    result = m_inputStream->Startup();
    if (result != 0)
        return result;

    m_randomSeedSet = 0;
    result = m_matchmakeUpdateJob->Startup(&m_filterPort);
    if (result != 0)
        return result;

    m_matchmakeUpdateJob->Ready(true);
    m_started = 1;
    return 0;
}

Result LanProtocol::RegisterSessionKey()
{
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    if (IsCryptoEnabled() && matchmakeSession->IsSessionParamRegistered())
    {
        common::CryptoSetting setting;
        setting.m_mode = m_cryptoSetting.m_mode;
        if (setting.m_mode != 0)
            CalculateCryptionKeyForSession(matchmakeSession->GetSessionInfoPtr()->GetSessionParam(),
                                           LanSessionInfo::cSessionParamSize, setting.m_key, cKeySize);
        session::g_piaSession->m_meshLayerController->UpdateCryptoSetting(setting);
        m_sessionKeyRegistered = 1;
    }
    return 0;
}

void LanProtocol::SendSelfKeepAliveMessage()
{
    LanSelfKeepAliveMessage message;
    unsigned int size = 0;
    Result result = message.Serialize(m_messageBuffer, &size);
    LanPayload payload(m_messageBuffer, size, message.m_messageType);
    common::InetAddress inetAddress(m_broadcastAddress, cPortBase);
    common::StationAddress address;
    address.SetInetAddress(inetAddress);
    SendMessage(&payload, address, true, false, cTraceDetail);
    address.Trace(cTraceDetail);
}

void LanProtocol::SendGetHostMessage()
{
    LanGetHostMessage message(
        static_cast<LanMatchmakeSession*>(
            session::g_piaSession->m_matchmakeSession[session::g_piaSession->m_currentSessionIndex])
            ->GetGatheringId());
    unsigned int size = 0;
    message.Serialize(m_messageBuffer, &size);
    LanPayload payload(m_messageBuffer, size, message.m_messageType);
    for (int index = 0; index < cPortCount; ++index)
    {
        common::InetAddress inetAddress(m_broadcastAddress, cPortBase + index);
        common::StationAddress address;
        address.SetInetAddress(inetAddress);
        SendMessage(&payload, address, true, false, cTraceDetail);
        address.Trace(cTraceDetail);
    }
}

void LanProtocol::SendGetSessionMessage()
{
    LanGetSessionMessage message(GetCurrentMatchmakeSession()->GetGatheringId());
    unsigned int size = 0;
    message.Serialize(m_messageBuffer, &size);
    LanPayload payload(m_messageBuffer, size, message.m_messageType);
    for (int index = 0; index < cPortCount; ++index)
    {
        common::InetAddress inetAddress(m_broadcastAddress, cPortBase + index);
        common::StationAddress address;
        address.SetInetAddress(inetAddress);
        SendMessage(&payload, address, true, false, cTraceDetail);
        address.Trace(cTraceDetail);
    }
}

void LanProtocol::SendHostMessage(const common::StationAddress& address)
{
    LanHostMessage message(GetCurrentMatchmakeSession()->GetGatheringId(), GetLocalStationConnectionInfo());
    unsigned int size = 0;
    message.Serialize(m_messageBuffer, &size);
    LanPayload payload(m_messageBuffer, size, message.m_messageType);
    for (int index = 0; index < cMessageSendCount; ++index)
        SendMessage(&payload, address, true, false, cTraceDetail);
    address.Trace(cTraceBrief);
    GetLocalStationConnectionInfo().Trace(cTraceDetail);
}

void LanProtocol::StopLanSession()
{
    if (!m_started)
        return;

    m_started = 0;
    m_matchmakeUpdateJob->m_cancelRequested = true;
    m_matchmakeUpdateJob->WaitForCompletion(cStopWaitMsec);
    m_matchmakeUpdateJob->Cleanup();
    m_sessionKeyRegistered = 0;
    m_sessionKeyRequested = 0;
    m_outputStream->Cleanup();
    m_inputStream->Cleanup();
    if (m_socket->m_openCount > 0)
        m_socket->Close();
    if (m_broadcastSocket->m_openCount > 0)
        m_broadcastSocket->Close();
    m_inputStream->ResetIgnoreAddress();
}

Result LanProtocol::CalculateCryptionKeyForSession(const unsigned char* input, unsigned int inputSize, void* output,
                                                   unsigned int outputSize)
{
    if (input == 0 || inputSize != LanSessionInfo::cSessionParamSize || output == 0 || outputSize != cKeySize)
        return cResultInvalidArgument;
    if (m_randomSeedSet == 0)
        return cResultInvalidState;
    if (m_cryptoSetting.m_mode == 0)
        return 0;

    unsigned char sessionParam[LanSessionInfo::cSessionParamSize];
    OSBlockMove(sessionParam, input, sizeof(sessionParam), FALSE);
    ++sessionParam[sizeof(sessionParam) - 1];
    common::Hmac hmac;
    common::Sha256Context context;
    hmac.Initialize(&context, m_cryptoSetting.m_key, cKeySize);
    unsigned char hash[LanSessionInfo::cSessionParamSize];
    hmac.Calc(hash, sessionParam, sizeof(sessionParam));
    OSBlockMove(output, hash, cKeySize, FALSE);
    return 0;
}

Result LanProtocol::CalculateCryptionKeyForBrowseReply(const unsigned char* input, unsigned int inputSize, void* output,
                                                       unsigned int outputSize)
{
    if (input == 0 || inputSize != LanSessionInfo::cSessionParamSize || output == 0 || outputSize != cKeySize)
        return cResultInvalidArgument;
    if (m_randomSeedSet == 0)
        return cResultInvalidState;
    if (m_cryptoSetting.m_mode == 0)
        return 0;

    unsigned char sessionParam[LanSessionInfo::cSessionParamSize];
    OSBlockMove(sessionParam, input, sizeof(sessionParam), FALSE);
    common::Hmac hmac;
    common::Sha256Context context;
    hmac.Initialize(&context, m_cryptoSetting.m_key, cKeySize);
    unsigned char hash[LanSessionInfo::cSessionParamSize];
    hmac.Calc(hash, sessionParam, sizeof(sessionParam));
    OSBlockMove(output, hash, cKeySize, FALSE);
    return 0;
}

Result LanProtocol::CalculateTokenHash(const unsigned char* input, unsigned int inputSize, void* output,
                                       unsigned int outputSize)
{
    if (input == 0 || inputSize != LanMatchmakeUpdateJob::cBrowseTokenSize || output == 0 || outputSize != cKeySize)
        return ResultInvalidArgument();
    if (m_cryptoSetting.m_mode == 0)
        return 0;

    common::Hmac hmac;
    common::Sha256Context context;
    hmac.Initialize(&context, m_cryptoSetting.m_key, cKeySize);
    unsigned char hash[LanSessionInfo::cSessionParamSize];
    hmac.Calc(hash, input, LanMatchmakeUpdateJob::cBrowseTokenSize);
    OSBlockMove(output, hash, cKeySize, FALSE);
    return 0;
}

Result LanProtocol::Dispatch()
{
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    if (m_cryptoSettingRegistered == 0)
        return 0;

    if (m_sessionKeyRequested != 0)
    {
        RegisterSessionKey();
        m_sessionKeyRequested = 0;
    }

    session::Session* currentSession = session::g_piaSession;
    if (currentSession->GetStatus() == session::Session::cConnectionStatusConnected
        || currentSession->GetStatus() == session::Session::cConnectionStatusDisconnectedNetwork
        || currentSession->GetStatus() == session::Session::cConnectionStatusDisconnectedSession)
        matchmakeSession->GetSessionInfoPtr()->SetCurrentParticipantNum(currentSession->GetParticipantNum());

    ReceiveMessages();

    if (m_getHostContext.m_state == common::CallContext::cStateInProgress)
    {
        common::Time now;
        now.SetNow();
        if (now <= m_getHostDeadline)
        {
            if (m_getHostContext.m_cancelRequested)
                m_getHostContext.SignalCancel();
        }
        else
        {
            m_getHostContext.SignalSuccess(0);
        }
    }

    if (m_updateSessionContext.m_state == common::CallContext::cStateInProgress)
    {
        if (matchmakeSession->IsMatchmakeOwner())
        {
            for (int index = 0; index < cPortCount; ++index)
            {
                common::StationAddress address;
                address.SetInetAddress(common::InetAddress(m_broadcastAddress, cPortBase + index));
                SendSessionMessage(address);
            }
        }
        m_updateSessionContext.SignalSuccess(0);
    }

    if (m_getSessionContext.m_state == common::CallContext::cStateInProgress)
    {
        common::Time now;
        now.SetNow();
        if (m_getSessionDeadline < now)
            m_getSessionContext.SignalFailure(cResultRequestTimeout);
    }

    if (m_unknown618 == 0)
    {
        common::Time now;
        now.SetNow();
        if (m_selfKeepAliveTime < now)
        {
            SendSelfKeepAliveMessage();
            ResetSelfKeepAliveTime();
        }
    }
    else
    {
        ResetSelfKeepAliveTime();
        m_unknown618 = 0;
    }
    return 0;
}

void LanProtocol::ReceiveMessages()
{
    transport::PacketHandler* handler = mStationPacketHandler;
    transport::PacketHandler::Iterator& iterator = handler->GetIterator(cProtocolType);
    while (!handler->IsEndIteration())
    {
        const transport::ProtocolMessageReader* reader = iterator.GetMessageReader();
        LanMessage message;
        message.Deserialize(reader->m_message + transport::ProtocolMessageReader::cMessageHeaderSize);
        OSBlockMove(m_messageBuffer, reader->m_message + transport::ProtocolMessageReader::cMessageHeaderSize,
                    reader->m_messageSize, FALSE);
        LanPayload payload;
        payload.m_data = m_messageBuffer;
        payload.m_size = static_cast<unsigned short>(reader->m_messageSize);
        payload.m_type = message.m_messageType;
        ParseMessage(&payload, reader->m_stationAddress);
        handler->NextIteration();
    }
}

void LanProtocol::SendSessionMessage(const common::StationAddress& address)
{
    LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
    unsigned int infoSize;
    matchmakeSession->GetSessionInfoPtr()->Serialize(m_sessionInfoBuffer, &infoSize, sizeof(m_sessionInfoBuffer));
    const unsigned char fragmentCount = static_cast<unsigned char>((infoSize - 1) / cSessionFragmentSize + 1);
    pead::Random random;
    const unsigned int id = random.getU32();
    unsigned int offset = 0;
    for (unsigned char index = 0; index < fragmentCount; ++index)
    {
        const unsigned int fragmentSize = index == fragmentCount - 1 ? infoSize - offset : cSessionFragmentSize;
        LanSessionMessage message(id, m_sessionMessageId, index, fragmentCount, m_sessionInfoBuffer + offset,
                                  fragmentSize);
        unsigned int size = 0;
        message.Serialize(m_messageBuffer, &size);
        LanPayload payload;
        payload.m_data = m_messageBuffer;
        payload.m_size = static_cast<unsigned short>(size);
        payload.m_type = message.m_messageType;
        for (int sendIndex = 0; sendIndex < cMessageSendCount; ++sendIndex)
            SendMessage(&payload, address, true, false, cTraceDetail);
        offset += cSessionFragmentSize;
    }
    ++m_sessionMessageId;
    address.Trace(cTraceDetail);
    matchmakeSession->GetSessionInfoPtr()->Trace(cTraceDetail);
}

void LanProtocol::ParseGetHostMessage(const LanPayload* payload, const common::StationAddress& source)
{
    LanGetHostMessage message;
    message.Deserialize(payload->m_data);
    if (payload->m_size == LanGetHostMessage::cSerializedSize)
    {
        LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
        if (session::g_piaMesh->GetMeshState() == cMeshStateEstablished
            && message.m_sessionId == matchmakeSession->GetGatheringId() && matchmakeSession->IsMatchmakeOwner())
            SendHostMessage(source);
    }
}

void LanProtocol::ParseGetSessionMessage(const LanPayload* payload, const common::StationAddress& source)
{
    LanGetSessionMessage message;
    message.Deserialize(payload->m_data);
    if (payload->m_size == LanGetSessionMessage::cSerializedSize)
    {
        LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
        const unsigned int meshState = session::g_piaMesh->GetMeshState();
        const bool leavingMesh = session::g_piaMesh->IsProcessingLeaveMesh();
        if (meshState == cMeshStateEstablished && !leavingMesh
            && message.m_sessionId == matchmakeSession->GetGatheringId() && matchmakeSession->IsMatchmakeOwner())
            SendSessionMessage(source);
    }
}

void LanProtocol::ParseHostMessage(const LanPayload* payload, const common::StationAddress&)
{
    LanHostMessage message;
    message.Deserialize(payload->m_data);
    if (payload->m_size == message.m_connectionInfo.GetSerializedSize() + cHostMessageHeaderSize)
    {
        LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
        if (message.m_sessionId == matchmakeSession->GetGatheringId())
        {
            const unsigned long long hostId = message.m_connectionInfo.m_stationLocation.m_principalId;
            const unsigned long long ownerId = matchmakeSession->GetMatchmakeOwnerUniqueId();
            matchmakeSession->SetMatchmakeOwnerUniqueId(hostId);
            matchmakeSession->SetMatchmakeOwner(GetLocalPrincipalId() == hostId);
            matchmakeSession->GetSessionInfoPtr()->SetStationConnectionInfo(message.m_connectionInfo);
            if (ownerId != hostId && ownerId != 0)
                session::g_piaSession->NotifyOwnershipChange(matchmakeSession->GetGatheringId(), hostId);
            m_getHostContext.SignalSuccess(0);
        }
    }
}

void LanProtocol::ParseSessionMessage(const LanPayload* payload, const common::StationAddress& source)
{
    LanSessionMessage message;
    message.Deserialize(payload->m_data);
    if (payload->m_size == message.m_fragmentSize + cSessionMessageHeaderSize)
    {
        LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
        if (!session::g_piaMesh->IsProcessingLeaveMesh())
        {
            m_receivedSessionInfo.Receive(message.m_fragment, message.m_fragmentSize, message.m_sessionId,
                                          message.m_sequence, message.m_index, message.m_count, source);
            if (m_receivedSessionInfo.IsReceivedAll())
            {
                matchmakeSession->GetSessionInfoBufferPtr()->Deserialize(m_receivedSessionInfo.m_data,
                                                                         sizeof(m_receivedSessionInfo.m_data));
                if (matchmakeSession->GetSessionInfoBufferPtr()->GetSessionId() == matchmakeSession->GetGatheringId()
                    && !matchmakeSession->IsMatchmakeOwner())
                {
                    matchmakeSession->GetSessionInfoPtr()->Deserialize(m_receivedSessionInfo.m_data,
                                                                       sizeof(m_receivedSessionInfo.m_data));
                    matchmakeSession->GetSessionInfoPtr()->Trace(cTraceBrief);
                    if (m_getSessionContext.m_state == common::CallContext::cStateInProgress)
                        m_getSessionContext.SignalSuccess(0);
                }
                matchmakeSession->GetSessionInfoPtr()->Trace(cTraceBrief);
            }
        }
    }
}

bool LanProtocol::RequestUpdateHostMessage()
{
    const bool established = session::g_piaMesh->GetMeshState() == cMeshStateEstablished;
    const bool leavingMesh = session::g_piaMesh->IsProcessingLeaveMesh();
    if (established && !leavingMesh)
    {
        LanMatchmakeSession* matchmakeSession = GetCurrentMatchmakeSession();
        matchmakeSession->GetSessionInfoPtr()->SetStationConnectionInfo(GetLocalStationConnectionInfo());
        matchmakeSession->SetMatchmakeOwnerUniqueId(GetLocalPrincipalId());
        matchmakeSession->SetMatchmakeOwner(true);
        for (int index = 0; index < cPortCount; ++index)
        {
            common::StationAddress address;
            address.SetInetAddress(common::InetAddress(m_broadcastAddress, cPortBase + index));
            SendHostMessage(address);
        }
        session::g_piaSession->NotifyOwnershipChange(matchmakeSession->GetGatheringId(), GetLocalPrincipalId());
    }
    return true;
}

unsigned char LanProtocol::IsCryptoSettingRegistered() const
{
    return m_cryptoSettingRegistered;
}

void LanProtocol::SetRandomSeed(unsigned long long first, unsigned long long second)
{
    m_random.UpdateSetting(first, second);
    m_randomSeedSet = 1;
}

transport::StationConnectionInfo& LanProtocol::GetLocalStationConnectionInfo()
{
    return transport::g_stationConnectionInfoTable->m_localStationConnectionInfo;
}

void LanProtocol::RequestRegisterSessionKey()
{
    m_sessionKeyRequested = 1;
}

}
}
}
