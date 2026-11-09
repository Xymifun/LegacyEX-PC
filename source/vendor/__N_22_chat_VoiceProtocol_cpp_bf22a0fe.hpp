#ifndef SOURCE_VENDOR___N_22_CHAT_VOICEPROTOCOL_CPP_BF22A0FE_HPP
#define SOURCE_VENDOR___N_22_CHAT_VOICEPROTOCOL_CPP_BF22A0FE_HPP

#include <stdint.h>
#include "nn/pia.h"
#include "nn/pia/Common.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "vendor/TmcHandsFree.hpp"
#include "vendor/VCTAudio.hpp"

namespace nn
{
namespace pia
{
namespace transport
{
StationIndex Conv2StationIndex(unsigned long long stationId);
unsigned char IsInitialized();
}
namespace chat
{

class HandsFree;

enum Codec
{
    CODEC_0 = 0,
    CODEC_1,
    CODEC_2,
    CODEC_3,
    CODEC_4,
    CODEC_5,
    CODEC_6
};
enum EcMode
{
    EC_MODE_0 = 0,
    EC_MODE_1,
    EC_MODE_2,
    EC_MODE_3
};

struct AudioInfo
{
    unsigned int m_Unknown0;
    unsigned int m_Unknown1;
    unsigned int m_Unknown2;
    unsigned int m_Unknown3;
    unsigned int m_Sequence;
    unsigned char m_UnknownTail[76];
};

class VoiceProtocol : public nn::pia::transport::Protocol
{
public:
    enum
    {
        cProtocolType = 0x78
    };

    struct Setting
    {
        Setting() : codec(CODEC_4), bufferSize(0x90), maxPlayers(10), dataSize(0x20), sampleRate(0x10) {}

        Codec codec;
        unsigned int bufferSize;
        unsigned char maxPlayers;
        unsigned int dataSize;
        unsigned short sampleRate;
    };

    VoiceProtocol();
    virtual ~VoiceProtocol();
    virtual void Trace(unsigned long long value) const;
    virtual unsigned short GetProtocolType() const;
    virtual Result StartupWithStationAddress(const nn::pia::common::StationAddress& address);
    virtual void CleanupWithStationAddress();
    virtual Result Dispatch();
    virtual Result UpdateProtocolEvent(const nn::pia::transport::ProtocolEvent& event);
    Result Initialize(const Setting& setting);
    void ClearStreaming();
    Result AddConferenceClient(StationIndex stationIndex);
    Result AddConferenceClient(unsigned long long stationId);
    Result RemoveConferenceClient(StationIndex stationIndex);
    Result RemoveConferenceClient(unsigned long long stationId);
    Result Send(const short* pData, unsigned int uiSamples, const short* pLeft, const short* pRight);
    Result Receive(void* pData, unsigned int uiSize, StationIndex* pStationIndex);
    void Finalize();
    Result GetAudioInfo(StationIndex stationIndex, AudioInfo* audioInfo) const;
    Result GetAudioInfo(unsigned long long stationId, AudioInfo* audioInfo) const;
    bool GetVadActivity() const;
    unsigned int GetAudioDataSize() const;
    bool IsInCommunication() const;
    unsigned int GetAudioFrameLength() const;
    bool IsClientInConference(StationIndex stationIndex) const;
    bool IsClientInConference(unsigned long long stationId) const;
    static int SendDataCallback(unsigned short cid, const void* pData, unsigned int uiSize);
    static int SendMultiDataCallback(unsigned int bitmap, const void* pData, unsigned int uiSize);

private:
    enum
    {
        cSendBufferSampleCount = 1536
    };

    int calcAudioBufferSize(unsigned short sampleRate) const;
    bool allocProc(unsigned short sampleRate);
    Result setupProtocols(unsigned char maxPlayers, unsigned int dataSize);
    void freeProc();
    void destroyProtocols();
    void initializeVct();
    void finalizeVct();
    int SendDataCallbackImpl(unsigned int stationIndex, const void* pData, unsigned int uiSize);
    int SendMultiDataCallbackImpl(unsigned int bitmap, const void* pData, unsigned int uiSize);
    unsigned int getPassedTime() const;
    void updateSessionEndMonitoringData();
    Result receiveCore(void* pData, unsigned int uiSize, unsigned short* pCids);

    unsigned short m_unreliableProtocolId;
    nn::pia::transport::UnreliableProtocol* m_pUnreliableProtocol;
    ::VCTSession* m_vctSession;
    unsigned char* m_audioBuffer;
    StationIndex m_stationIndex;
    Setting m_setting;
    nn::pia::common::CriticalSection m_criticalSection;
    unsigned int m_dispatchCount;
    unsigned long long m_sentDataSize;
    unsigned long long m_receivedDataSize;
    unsigned long long m_startTick;
    HandsFree* m_handsFree;
    short* m_sendBuffer;
};

unsigned char IsInitialized();
unsigned int ConvertID_Pia2VCT(unsigned int stationIndex);
unsigned int ConvertID_VCT2Pia(unsigned int connectionId);
}
}
}

extern nn::pia::chat::VoiceProtocol* g_voiceProtocolInstance;

namespace __N_22_chat_VoiceProtocol_cpp_bf22a0fe
{
template <typename T>
nn::pia::Result createProtocol(T** protocol, unsigned short* protocolId, unsigned char port);
template <typename T>
void destroyProtocol(T** protocol, unsigned short* protocolId);
VCTCodec convertCodec(nn::pia::chat::Codec codec);
nn::pia::chat::Codec convertCodec(VCTCodec codec);
nn::pia::chat::EcMode convertEcMode(TMC_HF_EC_MODE mode);
}
#endif
