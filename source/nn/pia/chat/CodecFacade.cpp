#include "nn/pia/chat/CodecFacade.hpp"

#include "nn/pia/ChatSupport.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/chat/Adpcm.hpp"

#include <new>

namespace
{
const int G711_SEGMENT_COUNT = 8;
const int G711_BIAS = 0x84;
const int G711_CLIP = 0x1fdf;

const int s_ulawSegmentEnd[G711_SEGMENT_COUNT] = {0x3f, 0x7f, 0xff, 0x1ff, 0x3ff, 0x7ff, 0xfff, 0x1fff};

inline unsigned int search(int iValue, const int* pTable, unsigned int uiSize)
{
    for (unsigned int index = 0; index < uiSize; ++index)
    {
        if (iValue <= *pTable++)
        {
            return index;
        }
    }
    return uiSize;
}
}

namespace nn
{
namespace pia
{
namespace chat
{

CodecFacade* g_codecFacade = NULL;
unsigned char* g_codecStorage = NULL;

CodecFacade::CodecFacade() : m_state(0)
{
}

Result CodecFacade::CreateInstance()
{
    if (IsInitialized() == 0)
        return transport::cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return transport::cResultInvalidState;
    if (g_codecFacade != 0)
        return transport::cResultAlreadyExists;

    g_codecStorage = common::NewArray<unsigned char>(sizeof(CodecFacade), 8);
    if (g_codecStorage == 0)
        return transport::cResultOutOfMemory;
    g_codecFacade = new (g_codecStorage) CodecFacade();
    return 0;
}

void CodecFacade::DestroyInstance()
{
    if (g_codecFacade != 0)
    {
        g_codecFacade->~CodecFacade();
        g_codecFacade = 0;
    }
    if (g_codecStorage != 0)
    {
        common::DeleteArray(g_codecStorage);
        g_codecStorage = 0;
    }
}

CodecFacade::~CodecFacade()
{
    Finalize();
}

Result CodecFacade::Initialize()
{
    return 0;
}

void CodecFacade::Finalize()
{
}

void CodecFacade::Decode(const unsigned char* pData, short* pOut, unsigned int uiSize, VCTCodec codec)
{
    switch (codec)
    {
    case VCT_CODEC_0:
        vct_decode_8bit_raw(pData, pOut, uiSize);
        break;
    case VCT_CODEC_1:
        vct_decode_g711_ulaw(pData, pOut, uiSize);
        break;
    case VCT_CODEC_2:
    case VCT_CODEC_3:
    case VCT_CODEC_4:
        Adpcm::Decode(pData, pOut, uiSize, codec);
        break;
    default:
        break;
    }
}

void CodecFacade::Encode(unsigned char* pOut, const short* pAudio, unsigned int uiSize, void* pState, VCTCodec codec)
{
    switch (codec)
    {
    case VCT_CODEC_0:
        vct_encode_8bit_raw(pOut, pAudio, uiSize);
        break;
    case VCT_CODEC_1:
        vct_encode_g711_ulaw(pOut, pAudio, uiSize);
        break;
    case VCT_CODEC_2:
    case VCT_CODEC_3:
    case VCT_CODEC_4:
        Adpcm::Encode(pOut, pAudio, uiSize, static_cast<Adpcm::State*>(pState), codec);
        break;
    default:
        break;
    }
}

void CodecFacade::Trace(unsigned long long) const
{
}

}
}
}

void vct_encode_g711_ulaw(unsigned char* pOut, const short* pAudio, unsigned int uiCount)
{
    for (unsigned int index = 0; index < uiCount; ++index)
    {
        int iSample = pAudio[index] >> 2;
        unsigned char ucMask;
        if (iSample < 0)
        {
            iSample = -iSample;
            ucMask = 0x7f;
        }
        else
        {
            ucMask = 0xff;
        }
        if (iSample > G711_CLIP)
        {
            iSample = G711_CLIP;
        }
        iSample += G711_BIAS >> 2;

        unsigned int uiSegment = search(iSample, s_ulawSegmentEnd, G711_SEGMENT_COUNT);
        if (uiSegment >= G711_SEGMENT_COUNT)
        {
            pOut[index] = static_cast<unsigned char>(0x7f ^ ucMask);
        }
        else
        {
            pOut[index] = static_cast<unsigned char>(((uiSegment << 4) | ((iSample >> (uiSegment + 1)) & 0x0f))
                                                     ^ ucMask);
        }
    }
}

void vct_decode_g711_ulaw(const unsigned char* pData, short* pOut, unsigned int uiCount)
{
    for (unsigned int index = 0; index < uiCount; ++index)
    {
        const unsigned int uiValue = ~static_cast<unsigned int>(pData[index]);
        const int iMagnitude = static_cast<int>(((uiValue & 0x0f) * 8 + G711_BIAS) << ((uiValue >> 4) & 0x07));
        if ((uiValue & 0x80) != 0)
        {
            pOut[index] = static_cast<short>(G711_BIAS - iMagnitude);
        }
        else
        {
            pOut[index] = static_cast<short>(iMagnitude - G711_BIAS);
        }
    }
}

void vct_encode_8bit_raw(unsigned char* pOut, const short* pAudio, unsigned int uiCount)
{
    for (unsigned int index = 0; index < uiCount; ++index)
    {
        pOut[index] = static_cast<unsigned char>(pAudio[index] >> 8);
    }
}

void vct_decode_8bit_raw(const unsigned char* pData, short* pOut, unsigned int uiCount)
{
    for (unsigned int index = 0; index < uiCount; ++index)
    {
        pOut[index] = static_cast<short>(static_cast<signed char>(pData[index]) << 8);
    }
}
