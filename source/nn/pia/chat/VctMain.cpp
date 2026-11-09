#include "vendor/VCTAudio.hpp"
#include "vendor/VCTQueue.hpp"

#include <coreinit/time.h>

struct VCTResult
{
    unsigned int status;
    unsigned int reserved;
    unsigned int payload;
};

namespace
{

const unsigned int VCT_AUDIO_SIGNATURE = 0x5f564354;
const unsigned int VCT_PACKET_TYPE_MASK = 0xf0;
const unsigned int VCT_AUDIO_PACKET_TYPE = 0x40;

unsigned int g_vctAudioPacketCount = 0;

int VCTi_HandleData(unsigned short connectionId, unsigned char* data, unsigned int dataSize, VCTResult* result)
{
    if (data != 0)
    {
        if (reinterpret_cast<const VCTAudioPacket*>(data)->m_Signature != VCT_AUDIO_SIGNATURE)
        {
            return 2;
        }
        if ((g_vctState.m_Mode != 0) && (g_vctInitialized != 0))
        {
            result->status = 0;
            result->payload = 0;
            const long long receiveTick = OSGetTime();
            if ((reinterpret_cast<const VCTAudioPacket*>(data)->m_Type & VCT_PACKET_TYPE_MASK) == VCT_AUDIO_PACKET_TYPE)
            {
                ++g_vctAudioPacketCount;
                vct_handle_audio(connectionId, data, dataSize, receiveTick);
                return 0;
            }
        }
    }
    return 0;
}

}

unsigned char VCT_IsInitialized()
{
    return g_vctInitialized;
}

bool VCT_HandleData(unsigned short connectionId, unsigned char* data, unsigned int dataSize)
{
    VCTResult result;
    return VCTi_HandleData(connectionId, data, dataSize, &result) != 2;
}

unsigned int VCT_GetAudioFrameLength()
{
    return VCTi_GetAudioFrameLength();
}

unsigned int VCT_GetAudioDataSize()
{
    return VCTi_GetAudioDataSize();
}

unsigned int VCT_GetAudioScale(const void* pAudio, unsigned int uiSize)
{
    if (pAudio == 0 || uiSize == 0)
    {
        return 0;
    }
    return HPFFilter2(pAudio, uiSize);
}
