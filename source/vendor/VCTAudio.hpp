#ifndef SOURCE_VENDOR_VCT_AUDIO_HPP
#define SOURCE_VENDOR_VCT_AUDIO_HPP

#include "nn/pia/Common.hpp"

#include <coreinit/fastmutex.h>

enum VCTCodec
{
    VCT_CODEC_0 = 0,
    VCT_CODEC_1,
    VCT_CODEC_2,
    VCT_CODEC_3,
    VCT_CODEC_4,
    VCT_CODEC_5,
    VCT_CODEC_6
};

const unsigned int VCT_TIMESTAMP_TICK_SCALE = 64;

struct _udp_packet;
struct VCTAudioPacket;

namespace nn
{
namespace pia
{
namespace chat
{
class AudioStream;
}
}
}

struct VCTSession
{
    unsigned int m_Mode;
    unsigned short m_Unknown04;
    unsigned short m_Cid;
    unsigned short m_OwnerCid;
    unsigned short m_ConferenceCids[16];
    unsigned short m_Unknown2A;
    unsigned int m_State;
    unsigned int m_Unknown30;
    unsigned int m_Unknown34;
    VCTSession* m_Next;
};

struct VCTAudioInfo
{
    unsigned int m_Unknown00;
    unsigned int m_Unknown04;
    unsigned int m_LastSequence;
    unsigned int m_Unknown0C;
    long long m_JitterOffset;
    long long m_JitterAverage;
    long long m_Skew;
    unsigned int m_Latency;
    unsigned int m_QueuedPacketCount;
    unsigned int m_DropCount;
    unsigned int m_ContinuousDropCount;
    unsigned int m_OutOfOrderCount;
    unsigned int m_DuplicateCount;
    unsigned int m_TooEarlyCount;
    unsigned int m_LateCount;
    unsigned int m_ExpiredCount;
    unsigned int m_Unknown4C;
    unsigned int m_Unknown50;
    unsigned int m_FadeRecoverCount;
    unsigned int m_SpareRecoverCount;
    unsigned int m_Unknown5C;
};

typedef int (*VCTSendCallback)(unsigned short cid, const void* pData, unsigned int uiSize);
typedef int (*VCTSendMultiCallback)(unsigned int uiCidMask, const void* pData, unsigned int uiSize);

enum
{
    VCT_CONFIG_FLAG_VAD = 0x01,
    VCT_CONFIG_FLAG_HPF = 0x04,
    VCT_CONFIG_FLAG_SPARE_PACKET = 0x08,
    VCT_CONFIG_FLAG_QUALITY_ALERT = 0x10,
    VCT_CONFIG_FLAG_DROP_RECOVER = 0x20
};

struct VCTConfig
{
    VCTSession* m_pSessions;
    unsigned int m_SessionCount;
    unsigned int m_Mode;
    unsigned int m_Unknown0C;
    unsigned short m_Cid;
    unsigned char m_Codec;
    unsigned char m_BufferSize;
    unsigned char m_Flags;
    unsigned char* m_pAudioBuffer;
    unsigned int m_AudioBufferSize;
    unsigned int m_Unknown20;
    VCTSendCallback m_SendCallback;
    unsigned int m_Unknown28;
    VCTSendMultiCallback m_SendMultiCallback;
};

struct VCTState
{
    unsigned short m_Cid;
    unsigned int m_Unknown04;
    VCTSendCallback m_SendCallback;
    unsigned int m_Unknown0C;
    unsigned int m_Unknown10;
    unsigned int m_Unknown14;
    unsigned int m_Mode;
    unsigned int m_Unknown1C;
    unsigned int m_Unknown20;
    VCTSendMultiCallback m_SendMultiCallback;
};

extern unsigned char g_vctInitialized;
extern VCTState g_vctState;
extern OSFastMutex g_vctDecoderMutex;
extern OSFastMutex g_vctAudioQueueMutex;
extern OSFastMutex g_vctPacketBufferMutex;

unsigned char VCT_IsInitialized();
bool VCT_HandleData(unsigned short connectionId, unsigned char* data, unsigned int dataSize);

unsigned int VCTi_GetAudioFrameLength();
unsigned int VCTi_GetAudioDataSize();
long long VCTi_GetAudioFrameTicks();
unsigned int VCT_GetAudioFrameLength();
unsigned int VCT_GetAudioDataSize();
unsigned int VCT_GetNoiseSkipTime();
unsigned int VCT_GetTotalPlayTime();
unsigned int VCT_GetTotalSpeechTime();

void VCT_ResetVAD();
void VCT_EnableVAD(bool bEnabled);
unsigned int VCT_GetVADReleaseTime();
bool VCT_GetVADActivity();
unsigned int VCTi_GetVADStatus(unsigned int energy, unsigned int threshold, unsigned char* pStatus);
void vct_init_vad();
unsigned short VCT_GetVADActiveGain();
unsigned short VCT_GetVADInactiveGain();
unsigned int VCT_GetVADClampGain();
unsigned int VCT_IsEnableVADInvalidation();

bool vct_is_adpcm(VCTCodec codec);
bool vct_is_opus(VCTCodec codec);
void init_adpcm_state();
int vct_get_packet_size(unsigned char ucCodec);

unsigned int HPFFilter(void* pAudio, unsigned int uiSize);
unsigned int HPFFilter2(const void* pAudio, unsigned int uiSize);
unsigned int VCT_GetAudioScale(const void* pAudio, unsigned int uiSize);

bool VCT_IsValidCodec(VCTCodec codec);
bool VCT_IsValidSpareCodec(VCTCodec codec);
bool VCT_GetQualityAlert(unsigned short usCid);
void vct_check_quality_alert_time();

void VCT_EnableSparePacket(bool bEnabled);
void VCT_EnableHPF(bool bEnabled);
void VCT_EnableDropRecover(bool bEnabled);

nn::pia::Result VCT_SendAudioEx(const void* pAudio, unsigned int uiAudioScale);
int VCTi_SendMulti(unsigned int uiCidMask, const void* pData, unsigned int uiSize);
VCTAudioPacket* vct_prepare_send_buffer();
int vct_flush_send_buffer();
unsigned int vct_handle_audio(unsigned short usCid, unsigned char* pData, unsigned int uiDataSize,
                              long long llReceiveTick);
int vct_decode_audio_buffer();
nn::pia::Result VCT_ReceiveAudio(void* pData, unsigned int uiSize, unsigned short* pCids);
int VCTi_ReceiveAudioData(void* pData, unsigned int uiSize, unsigned int uiStreamIndex, unsigned short* pCid);
void vct_mix_audio(const short* pLeft, const short* pRight, short* pOut, unsigned int uiSize);

nn::pia::chat::AudioStream* vct_get_audio_stream(unsigned int uiStreamIndex);
nn::pia::chat::AudioStream* VCTi_GetStreamFromCid(unsigned short usCid);
unsigned int VCT_GetDropCount(unsigned short usCid);
unsigned int VCT_GetDropFrequency(unsigned short usCid);
void VCT_GetAudioInfoFromCid(unsigned short usCid, VCTAudioInfo* pAudioInfo);

VCTSession* vct_find_session(unsigned short usCid);
VCTSession* VCTi_CreateSession(unsigned short usCid);
int VCT_DeleteSession(VCTSession* pSession);

int VCT_StartStreaming(const VCTSession* pSession);
void VCT_ClearStreaming(const VCTSession* pSession);
void VCT_StopStreaming(const VCTSession* pSession);

int VCT_AddConferenceClient(unsigned short usCid);
int VCT_RemoveConferenceClient(unsigned short usCid);
bool VCT_OnConferenceClient(unsigned short usCid);
void vct_finish_audio();
void vct_finish_ssp();
void VCT_Cleanup();
unsigned char VCT_Init(const VCTConfig* pConfig);
int vct_init_ssp(const VCTConfig* pConfig);
int vct_init_audio_ex(VCTCodec codec, unsigned int uiFrameMilliseconds);
void VCTi_MakeAudioParams(unsigned int uiFrameMilliseconds);
void VCT_ResetDropFrequencyAll();

void VCT_ResetVAD();

#endif
