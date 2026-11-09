#ifndef SOURCE_VENDOR_BINK_AUDIO_DECODE_HPP
#define SOURCE_VENDOR_BINK_AUDIO_DECODE_HPP

struct rfft_complex
{
    float re;
    float im;
};

struct PlanElement
{
    unsigned short uOffset;
    unsigned short uCount;
};

typedef void (*RFFT_PASS_CALLBACK)(rfft_complex* pData, const PlanElement* pPlan, unsigned int uCount);

struct BINKAUDIODECOMP;

static const unsigned int BINKAUDIO_FLAG_DCT = 1;
static const unsigned int BINKAUDIO_FLAG_PLANAR = 2;

extern "C" unsigned int BinkAudioDecompressMemoryLE(unsigned int uSampleRate, unsigned int uChannels,
                                                    unsigned int uFlags);
extern "C" BINKAUDIODECOMP* BinkAudioDecompressOpenLE(void* pMemory, unsigned int uSampleRate, unsigned int uChannels,
                                                      unsigned int uFlags);
extern "C" void BinkAudioDecompressLE(BINKAUDIODECOMP* pState, void* pOutput, unsigned int* puOutputBytes,
                                      const void* pInput, const void** ppInputEnd, const void* pInputLimit,
                                      float* pWork, int* piSkipSamples);

extern "C" double fxptof(unsigned int uValue);
extern "C" void ClampToS16_c(double dScale, short* pOutput, const float* pInput, unsigned int uCount);
extern "C" void CrossFade(short* pOutput, const short* pOverlap, unsigned int uBytes, unsigned int uShift);

int is_pow2(unsigned int uValue);
void scalar_ripre(rfft_complex* pData, unsigned int uCount, unsigned int uTwiddleBase);
void scalar_cfpass(rfft_complex* pData, const PlanElement* pPlan, unsigned int uCount);
void scalar_radix4(rfft_complex* pOutput, const rfft_complex* pFirst, const rfft_complex* pSecond,
                   const rfft_complex* pThird, const rfft_complex* pFourth, unsigned int uStart, unsigned int uEnd,
                   const unsigned short* pOrder);
void scalar_2radix2(rfft_complex* pOutput, const rfft_complex* pFirst, const rfft_complex* pSecond,
                    const rfft_complex* pThird, const rfft_complex* pFourth, unsigned int uStart, unsigned int uEnd,
                    const unsigned short* pOrder);
void scalar_dct_merge(float* pOutput, const float* pInput, unsigned int uCount);
void scalar_dct3_modulate(float* pOutput, const float* pInput, unsigned int uCount, unsigned int uHalfCount,
                          const rfft_complex* pTwiddles);
void fft_driver(rfft_complex* pData, unsigned int uCount, RFFT_PASS_CALLBACK pPass);

extern "C" void radfft_init();
extern "C" void radfft_cfft(float* pOutput, const float* pInput, unsigned int uCount);
extern "C" void radfft_rifft(float* pOutput, float* pInput, unsigned int uCount);
extern "C" void radfft_idct(float* pOutput, float* pInput, unsigned int uCount);

#endif
