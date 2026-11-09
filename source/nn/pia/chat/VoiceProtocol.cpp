#include "vendor/__N_22_chat_VoiceProtocol_cpp_bf22a0fe.hpp"
#include <string.h>
#include "nn/pia/transport/UnreliableProtocol.hpp"
#include "nn/pia/ProtocolManager.hpp"
#include "nn/pia/chat/HandsFree.hpp"
#include "platform/wiiu/pead.hpp"
#include "game/core/CriticalSection.hpp"
#include "vendor/VCTAudio.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/SessionBeginMonitoringContent.hpp"
#include <new>
#include "nn/pia/Transport.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

extern OSFastMutex g_voiceProtocolMutex;

namespace
{
enum VoiceProtocolResult
{
    cResultAllocationFailure = 0x10c01,
    cResultAlreadyInitialized = 0x10c02,
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08,
    cResultNotInitialized = 0x10c0c
};

enum DispatchLimit
{
    cMonitoringDataInterval = 0x1f,
    cQualityAlertInterval = 0xf,
    cReceiveBufferSize = 0x584
};

const unsigned int cVctMode = 2;
const unsigned long long cTraceFlagVoiceData = 0x0000004000000000ULL;

enum SettingLimit
{
    cCodecNum = 7,
    cBufferSizeMin = 0x40,
    cBufferSizeMax = 0xc0,
    cBufferSizeAlign = 8,
    cSampleRateMin = 2,
    cSampleRateMax = 0x10,
    cOpusBufferUnit = 0x14
};

const int cResultC0B = 0xc0b;
const int cResult2C09 = 0x2c09;
const int cResult2C0A = 0x2c0a;

void lock_()
{
    EnterCriticalSection(&g_voiceProtocolMutex);
}

void unlock_()
{
    LeaveCriticalSection(&g_voiceProtocolMutex);
}

class ProtocolLock
{
public:
    ProtocolLock() { lock_(); }
    ~ProtocolLock() { unlock_(); }
};

bool isOpus(nn::pia::chat::Codec codec)
{
    return codec == nn::pia::chat::CODEC_5 || codec == nn::pia::chat::CODEC_6;
}

TMC_HF_EC_MODE convertEcMode(nn::pia::chat::EcMode mode)
{
    static const unsigned int conversion[4] = {0, 1, 2, 0};
    return static_cast<TMC_HF_EC_MODE>(conversion[static_cast<unsigned int>(mode)]);
}

int convertCid(unsigned short cid)
{
    if (cid == 0)
        return 0xfd;
    return static_cast<int>(cid) - 1;
}

unsigned short convertStationIndex(nn::pia::StationIndex index)
{
    return static_cast<unsigned short>(index + 1);
}
}

extern nn::pia::chat::VoiceProtocol* g_voiceProtocolInstance = 0;
OSFastMutex g_voiceProtocolMutex;

namespace nn
{
namespace pia
{
namespace chat
{

VoiceProtocol::VoiceProtocol()
    : nn::pia::transport::Protocol(),
      m_unreliableProtocolId(0),
      m_pUnreliableProtocol(0),
      m_vctSession(0),
      m_audioBuffer(0),
      m_stationIndex(static_cast<StationIndex>(0xfd)),
      m_setting(),
      m_criticalSection(-1),
      m_dispatchCount(0),
      m_sentDataSize(0),
      m_receivedDataSize(0),
      m_startTick(0),
      m_handsFree(0),
      m_sendBuffer(0)
{
}

VoiceProtocol::~VoiceProtocol()
{
}

void VoiceProtocol::finalizeVct()
{
    VCT_Cleanup();
}

void VoiceProtocol::CleanupWithStationAddress()
{
    m_stationIndex = static_cast<StationIndex>(0xfd);
    finalizeVct();
}

bool VoiceProtocol::IsInCommunication() const
{
    return m_stationIndex != 0xfd;
}

void VoiceProtocol::ClearStreaming()
{
    VCT_ClearStreaming(m_vctSession);
}

unsigned int VoiceProtocol::GetAudioFrameLength() const
{
    return VCT_GetAudioFrameLength();
}

unsigned int VoiceProtocol::GetAudioDataSize() const
{
    return VCT_GetAudioDataSize();
}

bool VoiceProtocol::GetVadActivity() const
{
    return VCT_GetVADActivity();
}

void VoiceProtocol::Finalize()
{
    g_voiceProtocolInstance = 0;
    destroyProtocols();
    freeProc();
}

Result VoiceProtocol::Initialize(const Setting& setting)
{
    if (static_cast<unsigned int>(setting.codec) >= cCodecNum)
        return cResultInvalidArgument;
    if (setting.bufferSize < cBufferSizeMin || setting.bufferSize > cBufferSizeMax)
        return cResultInvalidArgument;
    if ((setting.bufferSize & (cBufferSizeAlign - 1)) != 0)
        return cResultInvalidArgument;
    if (setting.dataSize == 0)
        return cResultInvalidArgument;
    if (setting.sampleRate < cSampleRateMin || setting.sampleRate > cSampleRateMax)
        return cResultInvalidArgument;
    if (isOpus(setting.codec) && (setting.bufferSize % cOpusBufferUnit) != 0)
        return cResultInvalidArgument;
    if (!nn::pia::chat::IsInitialized())
        return cResultNotInitialized;
    if (!nn::pia::transport::IsInitialized())
        return cResultNotInitialized;
    if (g_voiceProtocolInstance != 0)
        return cResultAlreadyInitialized;
    if (nn::pia::transport::g_piaTransport == 0)
        return cResultInvalidState;
    if (!allocProc(setting.sampleRate))
        return cResultAllocationFailure;

    Result result = setupProtocols(setting.maxPlayers, setting.dataSize);
    if (result != 0)
    {
        freeProc();
        return result;
    }

    m_setting = setting;
    g_voiceProtocolInstance = this;
    OSFastMutex_Init(&g_voiceProtocolMutex, 0);
    return 0;
}

int VoiceProtocol::calcAudioBufferSize(unsigned short sampleRate) const
{
    return static_cast<int>(sampleRate * 0x6e80u) - 0x6e80;
}

bool VoiceProtocol::allocProc(unsigned short sampleRate)
{
    m_vctSession = common::NewArray<VCTSession>(sampleRate);
    if (m_vctSession != 0)
    {
        m_audioBuffer = common::NewArray<unsigned char>(calcAudioBufferSize(sampleRate), 0x20);
        if (m_audioBuffer != 0)
        {
            m_handsFree = common::NewObj<HandsFree>();
            if (m_handsFree != 0)
            {
                if (m_handsFree->Initialize(convertEcMode(EC_MODE_1), 1, 1) != 0)
                {
                    if (m_handsFree != 0)
                    {
                        common::DeleteObj(m_handsFree);
                        m_handsFree = 0;
                    }
                }
                else
                {
                    m_sendBuffer = common::NewArray<short>(cSendBufferSampleCount);
                    if (m_sendBuffer != 0)
                        return true;
                    m_handsFree->Finalize();
                    if (m_handsFree != 0)
                    {
                        common::DeleteObj(m_handsFree);
                        m_handsFree = 0;
                    }
                }
            }
            if (m_audioBuffer != 0)
            {
                common::DeleteArray(m_audioBuffer);
                m_audioBuffer = 0;
            }
        }
        if (m_vctSession != 0)
        {
            common::DeleteArray(m_vctSession);
            m_vctSession = 0;
        }
    }
    return false;
}

void VoiceProtocol::freeProc()
{
    if (m_sendBuffer != 0)
    {
        common::DeleteArray(m_sendBuffer);
        m_sendBuffer = 0;
    }
    if (m_handsFree != 0)
    {
        m_handsFree->Finalize();
        if (m_handsFree != 0)
        {
            common::DeleteObj(m_handsFree);
            m_handsFree = 0;
        }
    }
    if (m_vctSession != 0)
    {
        common::DeleteArray(m_vctSession);
        m_vctSession = 0;
    }
    if (m_audioBuffer != 0)
    {
        common::DeleteArray(m_audioBuffer);
        m_audioBuffer = 0;
    }
}

void VoiceProtocol::destroyProtocols()
{
    __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol(&m_pUnreliableProtocol, &m_unreliableProtocolId);
}
Result VoiceProtocol::setupProtocols(unsigned char maxPlayers, unsigned int dataSize)
{
    const int created = __N_22_chat_VoiceProtocol_cpp_bf22a0fe::createProtocol(&m_pUnreliableProtocol,
                                                                               &m_unreliableProtocolId, maxPlayers);
    if (created != 0)
        return created;
    const int initialized = m_pUnreliableProtocol->Initialize(dataSize);
    if (initialized != 0)
    {
        __N_22_chat_VoiceProtocol_cpp_bf22a0fe::destroyProtocol(&m_pUnreliableProtocol, &m_unreliableProtocolId);
        return initialized;
    }
    common::g_sessionBeginMonitoringContent.m_metric67 = dataSize;
    return 0;
}

Result VoiceProtocol::receiveCore(void* pData, unsigned int uiSize, unsigned short* pCids)
{
    ProtocolLock lock;
    Result result = VCT_ReceiveAudio(pData, uiSize, pCids);
    return result;
}

Result VoiceProtocol::AddConferenceClient(StationIndex stationIndex)
{
    ProtocolLock lock;
    switch (VCT_AddConferenceClient(ConvertID_Pia2VCT(stationIndex)))
    {
    case -2:
        return cResultInvalidState;
    case -6:
        return cResultInvalidState;
    case -1:
        return cResultInvalidArgument;
    case 0:
        return 0;
    default:
        return cResultC0B;
    }
}

Result VoiceProtocol::RemoveConferenceClient(StationIndex stationIndex)
{
    ProtocolLock lock;
    switch (VCT_RemoveConferenceClient(ConvertID_Pia2VCT(stationIndex)))
    {
    case -2:
        return cResultInvalidState;
    case -1:
        return cResult2C0A;
    case 0:
        return 0;
    default:
        return cResultC0B;
    }
}

Result VoiceProtocol::Send(const short* pData, unsigned int uiSamples, const short* pLeft, const short* pRight)
{
    if (m_handsFree == 0)
        return cResultNotInitialized;
    if (m_sendBuffer == 0)
        return cResultNotInitialized;
    if (pData == 0)
        return cResultInvalidArgument;
    if (pLeft == 0)
        return cResultInvalidArgument;
    if (pRight == 0)
        return cResultInvalidArgument;

    Result processResult = m_handsFree->Process(m_sendBuffer, pData, pLeft, pRight);
    if (processResult != 0)
        return processResult;
    const unsigned int scale = VCT_GetAudioScale(m_sendBuffer, uiSamples << 1);
    Result result = VCT_SendAudioEx(m_sendBuffer, scale);
    if (result == 0 || result == cResult2C0A)
        return 0;
    if (result == cResultInvalidState)
        return cResultInvalidState;
    if (result == cResultInvalidArgument)
        return cResultInvalidArgument;
    return cResultC0B;
}

void VoiceProtocol::Trace(unsigned long long value) const
{
}

unsigned short VoiceProtocol::GetProtocolType() const
{
    return cProtocolType;
}

Result VoiceProtocol::Receive(void* pData, unsigned int uiSize, StationIndex* pStationIndex)
{
    if (pData == 0)
        return ResultInvalidArgument();
    unsigned short cids[16];
    int result;
    if (pStationIndex == 0)
    {
        result = receiveCore(pData, uiSize, 0);
    }
    else
    {
        result = receiveCore(pData, uiSize, cids);
        if (result == 0)
        {
            for (int i = 0; i < m_setting.sampleRate; ++i)
                pStationIndex[i] = static_cast<StationIndex>(convertCid(cids[i]));
        }
    }
    if (result == 0)
        return 0;
    if (result == cResult2C09)
        return cResult2C09;
    if (result == cResultInvalidArgument)
        return cResultInvalidArgument;
    if (result == cResultInvalidState)
        return cResultInvalidState;
    return cResultC0B;
}

Result VoiceProtocol::GetAudioInfo(StationIndex stationIndex, AudioInfo* audioInfo) const
{
    const unsigned int index = stationIndex;
    if (index >= cStationIndexNum)
        return ResultInvalidArgument();
    if (audioInfo == 0)
        return ResultInvalidArgument();
    VCTAudioInfo source;
    VCT_GetAudioInfoFromCid(convertStationIndex(static_cast<StationIndex>(index)), &source);
    memcpy(audioInfo, &source, sizeof(source));
    return 0;
}
bool VoiceProtocol::IsClientInConference(StationIndex stationIndex) const
{
    return VCT_OnConferenceClient(ConvertID_Pia2VCT(stationIndex));
}
Result VoiceProtocol::AddConferenceClient(unsigned long long stationId)
{
    if (stationId == 0)
        return cResultInvalidArgument;
    return AddConferenceClient(transport::Conv2StationIndex(stationId));
}

Result VoiceProtocol::RemoveConferenceClient(unsigned long long stationId)
{
    if (stationId == 0)
        return cResultInvalidArgument;
    return RemoveConferenceClient(transport::Conv2StationIndex(stationId));
}

bool VoiceProtocol::IsClientInConference(unsigned long long stationId) const
{
    return IsClientInConference(transport::Conv2StationIndex(stationId));
}

Result VoiceProtocol::GetAudioInfo(unsigned long long stationId, AudioInfo* audioInfo) const
{
    return GetAudioInfo(transport::Conv2StationIndex(stationId), audioInfo);
}

int VoiceProtocol::SendDataCallback(unsigned short cid, const void* pData, unsigned int uiSize)
{
    return g_voiceProtocolInstance->SendDataCallbackImpl(ConvertID_VCT2Pia(cid), pData, uiSize);
}

int VoiceProtocol::SendMultiDataCallback(unsigned int bitmap, const void* pData, unsigned int uiSize)
{
    return g_voiceProtocolInstance->SendMultiDataCallbackImpl(bitmap, pData, uiSize);
}

int VoiceProtocol::SendDataCallbackImpl(unsigned int stationIndex, const void* pData, unsigned int uiSize)
{
    transport::UnreliableProtocol* pProtocol =
        transport::g_piaTransport->m_protocolManager.GetProtocol<transport::UnreliableProtocol>(m_unreliableProtocolId);
    Result result = pProtocol->Send(static_cast<StationIndex>(stationIndex), pData, uiSize);
    if (result == 0)
        m_sentDataSize += uiSize;
    return 1;
}

int VoiceProtocol::SendMultiDataCallbackImpl(unsigned int bitmap, const void* pData, unsigned int uiSize)
{
    transport::Transport* pTransport = transport::g_piaTransport;
    common::dump(cTraceFlagVoiceData, pData, uiSize);
    transport::UnreliableProtocol* pProtocol = pTransport->m_protocolManager.GetProtocol<transport::UnreliableProtocol>(
        m_unreliableProtocolId);
    Result result = pProtocol->SendToMulti(bitmap, pData, uiSize);
    if (result == 0)
        m_sentDataSize += uiSize * pead::BitFlagUtil::countOnBit(bitmap);
    return 1;
}

unsigned int VoiceProtocol::getPassedTime() const
{
    if (transport::g_piaTransport != 0)
    {
        long long elapsed = transport::g_piaTransport->m_time.m_tick - m_startTick;
        unsigned int msec = static_cast<unsigned int>(elapsed / common::TimeSpan::GetTicksPerMSec().m_tick);
        return msec;
    }
    return 0xffffffff;
}

Result VoiceProtocol::UpdateProtocolEvent(const transport::ProtocolEvent& event)
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(event.m_stationAddress);
    const unsigned int type = event.m_type;
    StationIndex index = station->GetStationIndex();
    if (type == 1)
        RemoveConferenceClient(index);
    return 0;
}

void VoiceProtocol::updateSessionEndMonitoringData()
{
    common::SessionStateMonitoringContent& state = common::g_sessionStateMonitoringContent;
    state.m_metric208 = VCT_GetAudioFrameLength();
    state.m_metric209 = VCT_GetNoiseSkipTime();
    state.m_metric210 = VCT_GetVADActiveGain();
    state.m_metric211 = VCT_GetVADClampGain();
    state.m_metric212 = VCT_GetVADInactiveGain();
    state.m_metric213 = VCT_GetVADReleaseTime();
    state.m_metric220 = m_sentDataSize;
    state.m_metric219 = m_receivedDataSize;
    state.m_metric216 = getPassedTime();
    state.m_metric217 = VCT_GetTotalPlayTime();
    state.m_metric218 = VCT_GetTotalSpeechTime();
}

void VoiceProtocol::initializeVct()
{
    VCTConfig config;
    config.m_pSessions = m_vctSession;
    config.m_SessionCount = m_setting.sampleRate;
    config.m_Mode = cVctMode;
    config.m_Unknown0C = 0;
    config.m_Cid = ConvertID_Pia2VCT(static_cast<unsigned short>(m_stationIndex));
    config.m_Codec = m_setting.codec;
    config.m_Flags = VCT_CONFIG_FLAG_VAD | VCT_CONFIG_FLAG_HPF | VCT_CONFIG_FLAG_SPARE_PACKET
                     | VCT_CONFIG_FLAG_QUALITY_ALERT | VCT_CONFIG_FLAG_DROP_RECOVER;
    config.m_BufferSize = m_setting.bufferSize;
    config.m_pAudioBuffer = m_audioBuffer;
    config.m_AudioBufferSize = calcAudioBufferSize(m_setting.sampleRate);
    config.m_Unknown20 = 0;
    config.m_SendCallback = SendDataCallback;
    config.m_Unknown28 = 0;
    config.m_SendMultiCallback = SendMultiDataCallback;
    VCT_Init(&config);
}

Result VoiceProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    transport::Station* station = transport::StationManager::s_pInstance->GetStation(address);
    if (station == 0)
        return cResultInvalidArgument;
    unsigned int index = station->m_field10;
    if (index >= transport::StationManager::cStationNumMax)
        return cResultInvalidArgument;
    if (m_stationIndex != cStationIndexInvalid)
        return cResultInvalidState;

    m_stationIndex = static_cast<StationIndex>(index);
    m_receivedDataSize = 0;
    m_sentDataSize = 0;
    m_dispatchCount = 0;
    m_startTick = transport::g_piaTransport->m_time.m_tick;
    initializeVct();
    return 0;
}

Result VoiceProtocol::Dispatch()
{
    ++m_dispatchCount;
    transport::Transport* pTransport = transport::g_piaTransport;
    if (pTransport == 0)
        return 0;
    if (m_stationIndex == cStationIndexInvalid)
        return 0;
    if (!VCT_IsInitialized())
        return 0;

    if ((m_dispatchCount & cMonitoringDataInterval) == 0)
        updateSessionEndMonitoringData();
    if ((m_dispatchCount & cQualityAlertInterval) == 0)
        vct_check_quality_alert_time();

    ProtocolLock lock;
    vct_flush_send_buffer();
    Result receiveResult;
    while (true)
    {
        unsigned long long stationId;
        unsigned int uiSize = 0;
        unsigned char data[cReceiveBufferSize];
        transport::UnreliableProtocol* pProtocol =
            pTransport->m_protocolManager.GetProtocol<transport::UnreliableProtocol>(m_unreliableProtocolId);
        receiveResult = pProtocol->Receive(&stationId, data, &uiSize, sizeof(data));
        if (receiveResult != 0)
            break;
        StationIndex stationIndex = transport::g_piaTransport->ConvertToStationIndexUnsafe(stationId);
        if (!transport::g_piaTransport->m_stationIdTable->IsJoinEventNotifiedByStationIndex(stationIndex))
            continue;
        common::dump(cTraceFlagVoiceData, data, uiSize);
        VCT_HandleData(ConvertID_Pia2VCT(static_cast<unsigned short>(stationIndex)), data, uiSize);
        m_receivedDataSize += uiSize;
    }
    while (vct_decode_audio_buffer() != 0)
    {
    }
    return 0;
}
}
}
}
