#ifndef SOURCE_VENDOR_VCT_DROP_RECOVERY_HPP
#define SOURCE_VENDOR_VCT_DROP_RECOVERY_HPP

struct _udp_packet;

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

unsigned int vct_calc_energy(const void* pAudio, unsigned int uiSize);
void vct_recover_drop_copy_tail(const void* pPrevious, void* pOut, unsigned int uiSize);
void vct_recover_drop_white_noise(const void* pPrevious, void* pOut, unsigned int uiSize);
void vct_recover_drop_pink_noise(const void* pPrevious, void* pOut, unsigned int uiSize);
void vct_recover_drop(const void* pPrevious, void* pOut, unsigned int uiSize);
int vct_recover_drop_using_spare(nn::pia::chat::AudioStream* pStream, const _udp_packet* pPacket, void* pOut,
                                 unsigned int uiSize);
void vct_filter_fade_in(const short* pSrc, short* pDst, unsigned int uiCount, unsigned int uiFadeLength,
                        short sPrevious);
void vct_filter_fade_rate(const short* pSrc, short* pDst, unsigned int uiCount, float fStartGain, float fEndGain);
int vct_recover_drop_fade(const void* pPrevious, void* pOut, unsigned int uiSize, unsigned int uiStreamIndex);
int vct_recover_drop_stream(nn::pia::chat::AudioStream* pStream, void* pOut, unsigned int uiSize);

void VCT_SetDropRecoverContinuousDropFadeLength(unsigned char ucLength);
void VCT_SetDropRecoverMode(unsigned char ucMode);
unsigned char VCT_GetDropRecoverInterpolateSampleNum();

#endif
