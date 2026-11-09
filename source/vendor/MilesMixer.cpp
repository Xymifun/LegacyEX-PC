#include "vendor/MilesMixer.hpp"

#include <stdarg.h>
#include <stddef.h>
#include <string.h>

#include "game/util/StringUtils.hpp"
#include "vendor/MilesMem.hpp"

#ifdef __ghs__
#include <ppc_ghs.h>
#endif

static const int MIX_ACCUMULATOR_SHIFT = 11;
static const int MIX_BYTE_SHIFT = 8;
static const int MIX_SAMPLE_MAX = 0x7fff;
static const int MIX_SAMPLE_MIN = -0x8000;
static const unsigned char MIX_BYTE_SIGN = 0x80;
static const unsigned char MIX_BYTE_MAX = 0xff;
static const unsigned char MIX_BYTE_MIN = 0;
static const unsigned int MIX_COPY_FLAG_16BIT = 1;
static const unsigned int MIX_COPY_FLAG_STEREO = 2;
static const int MIX_CHANNELS_STEREO = 2;
static const int MIX_SCALE_UNITY = 0x800;
static const int MIX_DEST_MONO = 1;
static const int MIX_DEST_STEREO = 2;
static const int MIX_SOURCE_MONO_BYTES = 2;
static const int MIX_SOURCE_STEREO_BYTES = 4;
static const int MIX_RESAMPLE_MONO_SHIFT = 15;
static const int MIX_RESAMPLE_STEREO_SHIFT = 14;
static const unsigned int MIX_FRACTION_MASK = 0xffff;
static const int MIX_FRACTION_BITS = 16;
static const unsigned int MIX_STEP_INTEGER_MASK = 0xffff0000;
static const unsigned int MIX_POSITION_MASK = 0x3fffffff;
static const unsigned int MIX_POSITION_ONE = 0x10000;
static const unsigned int MIX_POSITION_TWO = 0x20000;
static const int MIX_STATE_SHIFT = 30;
static const int MIX_STATE_PRODUCE = 1;
static const int MIX_STATE_CONSUME = 2;
static const unsigned int MIX_STATE_PRODUCE_FLAG = 0x40000000;
static const unsigned int MIX_STATE_CONSUME_FLAG = 0x80000000;
static const int MIX_BLEND_SHIFT = 17;
static const int MIX_BLEND_BITS = 15;
static const int MIX_ADPCM_HEADER_BYTES = 4;
static const int MIX_ADPCM_WORD_BYTES = 4;
static const unsigned int MIX_ADPCM_NIBBLES = 8;
static const unsigned int MIX_ADPCM_NIBBLE_MASK = 0xf;
static const int MIX_ADPCM_NIBBLE_SHIFT = 2;
static const int MIX_ADPCM_NIBBLE_BITS = 4;
static const int MIX_ADPCM_DELTA_SHIFT = 14;
static const unsigned int MIX_ADPCM_INDEX_BITS_MASK = 0x3fff;
static const int MIX_ADPCM_INDEX_SHIFT = 16;
static const unsigned int MIX_ADPCM_INDEX_MASK = 0xff;
static const int MIX_ADPCM_STEP_SHIFT = 6;
static const int MIX_ADPCM_CHANNEL_BITS = 16;
static const int MIX_ADPCM_CHANNEL_MASK = 0xffff;

enum MixOperation
{
    MIX_OP_DEST_STEREO = 0x01,
    MIX_OP_SRC_16 = 0x02,
    MIX_OP_FILTER = 0x04,
    MIX_OP_SRC_STEREO = 0x08,
    MIX_OP_RESAMPLE = 0x10,
    MIX_OP_VOLUME = 0x20,
    MIX_OP_COPY16 = 0x40
};

static const char MILES_EVENT_BUFFER_ERROR[] = "Insufficient Buffer Space";
static const char MILES_EVENT_VERSION_ERROR[] = "Invalid event version - required version is: 4";
static const int MILES_EVENT_VERSION = 4;
static const int MILES_EVENT_STEP_PREFIX = 2;
static const unsigned int MIX_SPEAKER_CONFIGURATION_STEREO = 2;

struct MIX_COPY_CHANNEL
{
    void* pData;
    unsigned int uLength;
    int iChannels;
    int iDestinationOffset;
};

static const char MIX_INTERFACE_NAME[] = "MSS mixer services";

static const unsigned int MIX_PROVIDER_VERSION = 0x130;

static const RIB_INTERFACE_ENTRY MIX_interface[] = {{0, "PROVIDER_property", (void*)MSS_mixer_property, 0},
                                                    {0, "MIXER_startup", (void*)MSS_mixer_startup, 0},
                                                    {0, "MIXER_startup", (void*)MSS_mixer_startup, 0},
                                                    {0, "MIXER_shutdown", (void*)MSS_mixer_shutdown, 0},
                                                    {0, "MIXER_flush", (void*)MSS_mixer_flush, 0},
                                                    {0, "MIXER_merge", (void*)MSS_mixer_merge, 0},
                                                    {0, "MIXER_copy", (void*)MSS_mixer_copy, 0},
                                                    {0, "MIXER_mc_copy", (void*)MSS_mixer_mc_copy, 0},
                                                    {0, "MIXER_adpcm_decode", (void*)MSS_mixer_adpcm_decode, 0}};

int MSS_mixer_property(unsigned int uProperty, void* pBefore, const void*, void*)
{
    switch (uProperty)
    {
    case RIB_PROVIDER_NAME:
        if (pBefore != NULL)
        {
            *(const char**)pBefore = "Default Miles Mixer";
            return 1;
        }
        break;
    case RIB_PROVIDER_VERSION:
        if (pBefore != NULL)
        {
            *(unsigned int*)pBefore = MIX_PROVIDER_VERSION;
            return 1;
        }
        break;
    }

    return 0;
}

extern "C" void MSS_mixer_startup()
{
}

extern "C" void MSS_mixer_shutdown()
{
}

extern "C" void MSS_mixer_flush(void* pDestination, int iLength)
{
    memset(pDestination, 0, iLength);
}

static inline short MIX_load_le16(const void* pSource)
{
#ifdef __ghs__
    short iValue;
    __lhbrx(pSource, iValue);
    return iValue;
#else
    const unsigned char* pBytes = (const unsigned char*)pSource;
    return (short)(pBytes[0] | (pBytes[1] << 8));
#endif
}

static inline int MIX_load_le32(const void* pSource)
{
#ifdef __ghs__
    int iValue;
    __lwbrx(pSource, iValue);
    return iValue;
#else
    const unsigned char* pBytes = (const unsigned char*)pSource;
    return (int)((unsigned int)pBytes[0] | ((unsigned int)pBytes[1] << 8) | ((unsigned int)pBytes[2] << 16)
                 | ((unsigned int)pBytes[3] << 24));
#endif
}

static inline void MIX_store_le16(void* pDestination, int iValue)
{
#ifdef __ghs__
    __sthbrx(pDestination, iValue);
#else
    unsigned char* pBytes = (unsigned char*)pDestination;
    pBytes[0] = (unsigned char)iValue;
    pBytes[1] = (unsigned char)(iValue >> 8);
#endif
}

static inline void MIX_store_le32(void* pDestination, int iValue)
{
#ifdef __ghs__
    __stwbrx(pDestination, iValue);
#else
    unsigned char* pBytes = (unsigned char*)pDestination;
    pBytes[0] = (unsigned char)iValue;
    pBytes[1] = (unsigned char)(iValue >> 8);
    pBytes[2] = (unsigned char)(iValue >> 16);
    pBytes[3] = (unsigned char)(iValue >> 24);
#endif
}

static inline void MIX_store_byte(unsigned char* pOutput, int iValue)
{
    int bOutput;

    if (iValue <= MIX_SAMPLE_MIN)
    {
        *pOutput = MIX_BYTE_MIN;
    }
    else
    {
        bOutput = MIX_BYTE_MAX;

        if (iValue < MIX_SAMPLE_MAX)
        {
            bOutput = (iValue >> MIX_BYTE_SHIFT) ^ MIX_BYTE_SIGN;
        }

        *pOutput = (unsigned char)bOutput;
    }
}

static inline void MIX_store_word(short* pOutput, int iValue)
{
    if (iValue <= MIX_SAMPLE_MIN)
    {
        iValue = MIX_SAMPLE_MIN;
    }
    else if (iValue >= MIX_SAMPLE_MAX)
    {
        iValue = MIX_SAMPLE_MAX;
    }

    *pOutput = (short)iValue;
}

extern "C" void MSS_mixer_mc_copy(void* pChannelList, int iChannelCount, void* pDestination, unsigned int uFlags,
                                  unsigned int uFrames, int iStride)
{
    MIX_COPY_CHANNEL* pChannels = (MIX_COPY_CHANNEL*)pChannelList;
    int iChannel;
    int iFrame;
    int iSample;
    int iLeft;
    int iRight;
    const int* pSource;
    unsigned char* pByte;
    short* pWord;

    if ((uFlags & MIX_COPY_FLAG_16BIT) != 0)
    {
        for (iChannel = 0; iChannel < iChannelCount; iChannel++)
        {
            pSource = (const int*)pChannels[iChannel].pData;
            pWord = (short*)pDestination + pChannels[iChannel].iDestinationOffset;

            if (pChannels[iChannel].iChannels == MIX_CHANNELS_STEREO)
            {
                for (iSample = 0; iSample < (int)(uFrames * 2); iSample += 2)
                {
                    iLeft = pSource[iSample] >> MIX_ACCUMULATOR_SHIFT;
                    iRight = pSource[iSample + 1] >> MIX_ACCUMULATOR_SHIFT;
                    MIX_store_word(pWord, iLeft);
                    MIX_store_word(pWord + 1, iRight);
                    pWord += iStride;
                }
            }
            else
            {
                for (iFrame = 0; iFrame < (int)uFrames; iFrame++)
                {
                    MIX_store_word(pWord, pSource[iFrame] >> MIX_ACCUMULATOR_SHIFT);
                    pWord += iStride;
                }
            }
        }
    }
    else
    {
        for (iChannel = 0; iChannel < iChannelCount; iChannel++)
        {
            pSource = (const int*)pChannels[iChannel].pData;
            pByte = (unsigned char*)pDestination + pChannels[iChannel].iDestinationOffset;

            if (pChannels[iChannel].iChannels == MIX_CHANNELS_STEREO)
            {
                for (iSample = 0; iSample < (int)(uFrames * 2); iSample += 2)
                {
                    iLeft = pSource[iSample] >> MIX_ACCUMULATOR_SHIFT;
                    iRight = pSource[iSample + 1] >> MIX_ACCUMULATOR_SHIFT;
                    MIX_store_byte(pByte, iLeft);
                    MIX_store_byte(pByte + 1, iRight);
                    pByte += iStride;
                }
            }
            else
            {
                for (iFrame = 0; iFrame < (int)uFrames; iFrame++)
                {
                    MIX_store_byte(pByte, pSource[iFrame] >> MIX_ACCUMULATOR_SHIFT);
                    pByte += iStride;
                }
            }
        }
    }
}

extern "C" void MSS_mixer_copy(void* pSource, unsigned int uLength, unsigned short* pDestination, unsigned int uFlags,
                               int iSuppressSwap)
{
    MIX_COPY_CHANNEL stereo;
    MIX_COPY_CHANNEL mono;
    unsigned short* pSample;
    int iCount;

    if ((uFlags & MIX_COPY_FLAG_STEREO) != 0)
    {
        stereo.pData = pSource;
        stereo.uLength = uLength;
        stereo.iChannels = MIX_CHANNELS_STEREO;
        stereo.iDestinationOffset = 0;

        MSS_mixer_mc_copy(&stereo, 1, pDestination, uFlags, (unsigned int)((int)uLength / 8), MIX_CHANNELS_STEREO);
    }
    else
    {
        mono.pData = pSource;
        mono.uLength = uLength;
        mono.iChannels = 1;
        mono.iDestinationOffset = 0;

        MSS_mixer_mc_copy(&mono, 1, pDestination, uFlags, (unsigned int)((int)uLength / 4), 1);
    }

    if (iSuppressSwap == 0 && (uFlags & MIX_COPY_FLAG_16BIT) != 0)
    {
        pSample = pDestination;

        for (iCount = (int)uLength / 2; iCount != 0; iCount = iCount - 2)
        {
            *pSample = (unsigned short)MIX_load_le16(pSample);
            pSample = pSample + 1;
        }
    }
}


struct MixMonoMonoFixed
{
    typedef int Unit;
    static const int DEST_UNITS = MIX_DEST_MONO;
    static const int SOURCE_BYTES = MIX_SOURCE_MONO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_MONO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int, int)
    {
        *pDestination += MIX_load_le16(pSource) * MIX_SCALE_UNITY;
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        *piLeft = MIX_load_le16(pSource);
        *piRight = 0;
    }

    static inline int Weight(int, int iStep) { return (MIX_SCALE_UNITY << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int, int, int) { *pDestination += iLeft * MIX_SCALE_UNITY; }

    static inline void Filtered(Unit* pDestination, int iLeft, int, int iWeightLeft, int)
    {
        *pDestination += iLeft * iWeightLeft;
    }

    static inline int LoadRight(const int*) { return 0; }

    static inline void SaveRight(int*, int) {}
};

struct MixMonoStereoFixed
{
    typedef int Unit;
    static const int DEST_UNITS = MIX_DEST_STEREO;
    static const int SOURCE_BYTES = MIX_SOURCE_MONO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_MONO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int, int)
    {
        pDestination[0] += MIX_load_le16(pSource) * MIX_SCALE_UNITY;
        pDestination[1] += MIX_load_le16(pSource) * MIX_SCALE_UNITY;
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        *piLeft = MIX_load_le16(pSource);
        *piRight = 0;
    }

    static inline int Weight(int, int iStep) { return (MIX_SCALE_UNITY << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int, int, int)
    {
        pDestination[0] += iLeft * MIX_SCALE_UNITY;
        pDestination[1] += iLeft * MIX_SCALE_UNITY;
    }

    static inline void Filtered(Unit* pDestination, int iLeft, int, int iWeightLeft, int)
    {
        pDestination[0] += iLeft * iWeightLeft;
        pDestination[1] += iLeft * iWeightLeft;
    }

    static inline int LoadRight(const int*) { return 0; }

    static inline void SaveRight(int*, int) {}
};

struct MixStereoMonoFixed
{
    typedef int Unit;
    static const int DEST_UNITS = MIX_DEST_MONO;
    static const int SOURCE_BYTES = MIX_SOURCE_STEREO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_STEREO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int, int)
    {
        *pDestination += ((short)MIX_load_le32(pSource) + (MIX_load_le32(pSource) >> 16)) * MIX_SCALE_UNITY;
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        int iPair = MIX_load_le32(pSource);
        *piLeft = (short)iPair + (iPair >> MIX_FRACTION_BITS);
        *piRight = 0;
    }

    static inline int Weight(int, int iStep) { return (MIX_SCALE_UNITY << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int, int, int) { *pDestination += iLeft * MIX_SCALE_UNITY; }

    static inline void Filtered(Unit* pDestination, int iLeft, int, int iWeightLeft, int)
    {
        *pDestination += iLeft * iWeightLeft;
    }

    static inline int LoadRight(const int*) { return 0; }

    static inline void SaveRight(int* piRight, int) { *piRight = 0; }
};

struct MixStereoStereoFixed
{
    typedef int Unit;
    static const int DEST_UNITS = MIX_DEST_STEREO;
    static const int SOURCE_BYTES = MIX_SOURCE_STEREO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_STEREO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int, int)
    {
        pDestination[1] += (MIX_load_le32(pSource) >> 16) * MIX_SCALE_UNITY;
        pDestination[0] += (short)MIX_load_le32(pSource) * MIX_SCALE_UNITY;
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        int iPair = MIX_load_le32(pSource);
        *piLeft = (short)iPair;
        *piRight = iPair >> MIX_FRACTION_BITS;
    }

    static inline int Weight(int, int iStep) { return (MIX_SCALE_UNITY << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int iRight, int, int)
    {
        pDestination[0] += iLeft * MIX_SCALE_UNITY;
        pDestination[1] += iRight * MIX_SCALE_UNITY;
    }

    static inline void Filtered(Unit* pDestination, int iLeft, int iRight, int iWeightLeft, int)
    {
        pDestination[0] += iLeft * iWeightLeft;
        pDestination[1] += iRight * iWeightLeft;
    }

    static inline int LoadRight(const int* piRight) { return *piRight; }

    static inline void SaveRight(int* piRight, int iRight) { *piRight = iRight; }
};

struct MixMonoMonoVolume
{
    typedef int Unit;
    static const int DEST_UNITS = MIX_DEST_MONO;
    static const int SOURCE_BYTES = MIX_SOURCE_MONO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_MONO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int iScaleLeft, int)
    {
        *pDestination += MIX_load_le16(pSource) * iScaleLeft;
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        *piLeft = MIX_load_le16(pSource);
        *piRight = 0;
    }

    static inline int Weight(int iScale, int iStep) { return (iScale << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int, int iScaleLeft, int)
    {
        *pDestination += iLeft * iScaleLeft;
    }

    static inline void Filtered(Unit* pDestination, int iLeft, int, int iWeightLeft, int)
    {
        *pDestination += iLeft * iWeightLeft;
    }

    static inline int LoadRight(const int*) { return 0; }

    static inline void SaveRight(int*, int) {}
};

struct MixMonoStereoVolume
{
    typedef int Unit;
    static const int DEST_UNITS = MIX_DEST_STEREO;
    static const int SOURCE_BYTES = MIX_SOURCE_MONO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_MONO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int iScaleLeft, int iScaleRight)
    {
        pDestination[1] += MIX_load_le16(pSource) * iScaleRight;
        pDestination[0] += MIX_load_le16(pSource) * iScaleLeft;
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        *piLeft = MIX_load_le16(pSource);
        *piRight = 0;
    }

    static inline int Weight(int iScale, int iStep) { return (iScale << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int, int iScaleLeft, int iScaleRight)
    {
        pDestination[1] += iLeft * iScaleRight;
        pDestination[0] += iLeft * iScaleLeft;
    }

    static inline void Filtered(Unit* pDestination, int iLeft, int, int iWeightLeft, int iWeightRight)
    {
        pDestination[0] += iLeft * iWeightLeft;
        pDestination[1] += iLeft * iWeightRight;
    }

    static inline int LoadRight(const int*) { return 0; }

    static inline void SaveRight(int* piRight, int) { *piRight = 0; }
};

struct MixStereoMonoVolume
{
    typedef int Unit;
    static const int DEST_UNITS = MIX_DEST_MONO;
    static const int SOURCE_BYTES = MIX_SOURCE_STEREO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_STEREO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int iScaleLeft, int iScaleRight)
    {
        *pDestination += (short)MIX_load_le32(pSource) * iScaleLeft + (MIX_load_le32(pSource) >> 16) * iScaleRight;
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        int iPair = MIX_load_le32(pSource);
        *piLeft = (short)iPair;
        *piRight = iPair >> MIX_FRACTION_BITS;
    }

    static inline int Weight(int iScale, int iStep) { return (iScale << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int iRight, int iScaleLeft, int iScaleRight)
    {
        *pDestination += iLeft * iScaleLeft + iRight * iScaleRight;
    }

    static inline void Filtered(Unit* pDestination, int iLeft, int iRight, int iWeightLeft, int iWeightRight)
    {
        *pDestination += iLeft * iWeightLeft + iRight * iWeightRight;
    }

    static inline int LoadRight(const int* piRight) { return *piRight; }

    static inline void SaveRight(int* piRight, int iRight) { *piRight = iRight; }
};

struct MixStereoStereoVolume
{
    typedef int Unit;
    static const int DEST_UNITS = MIX_DEST_STEREO;
    static const int SOURCE_BYTES = MIX_SOURCE_STEREO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_STEREO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int iScaleLeft, int iScaleRight)
    {
        pDestination[0] += (short)MIX_load_le32(pSource) * iScaleLeft;
        pDestination[1] += (MIX_load_le32(pSource) >> 16) * iScaleRight;
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        int iPair = MIX_load_le32(pSource);
        *piLeft = (short)iPair;
        *piRight = iPair >> MIX_FRACTION_BITS;
    }

    static inline int Weight(int iScale, int iStep) { return (iScale << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int iRight, int iScaleLeft, int iScaleRight)
    {
        pDestination[0] += iLeft * iScaleLeft;
        pDestination[1] += iRight * iScaleRight;
    }

    static inline void Filtered(Unit* pDestination, int iLeft, int iRight, int iWeightLeft, int iWeightRight)
    {
        pDestination[0] += iLeft * iWeightLeft;
        pDestination[1] += iRight * iWeightRight;
    }

    static inline int LoadRight(const int* piRight) { return *piRight; }

    static inline void SaveRight(int* piRight, int iRight) { *piRight = iRight; }
};

struct MixMonoMonoCopy
{
    typedef short Unit;
    static const int DEST_UNITS = MIX_DEST_MONO;
    static const int SOURCE_BYTES = MIX_SOURCE_MONO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_MONO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int, int)
    {
        MIX_store_le16(pDestination, MIX_load_le16(pSource));
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        *piLeft = MIX_load_le16(pSource);
        *piRight = 0;
    }

    static inline int Weight(int, int iStep) { return (MIX_SCALE_UNITY << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int, int, int) { MIX_store_le16(pDestination, iLeft); }

    static inline void Filtered(Unit* pDestination, int iLeft, int, int iWeightLeft, int)
    {
        MIX_store_le16(pDestination, (iLeft * iWeightLeft) >> MIX_ACCUMULATOR_SHIFT);
    }

    static inline int LoadRight(const int*) { return 0; }

    static inline void SaveRight(int*, int) {}
};

struct MixMonoStereoCopy
{
    typedef short Unit;
    static const int DEST_UNITS = MIX_DEST_STEREO;
    static const int SOURCE_BYTES = MIX_SOURCE_MONO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_MONO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int, int)
    {
        MIX_store_le16(pDestination, MIX_load_le16(pSource));
        MIX_store_le16(pDestination + 1, MIX_load_le16(pSource));
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        *piLeft = MIX_load_le16(pSource);
        *piRight = 0;
    }

    static inline int Weight(int, int iStep) { return (MIX_SCALE_UNITY << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int, int, int)
    {
        MIX_store_le16(pDestination, iLeft);
        MIX_store_le16(pDestination + 1, iLeft);
    }

    static inline void Filtered(Unit* pDestination, int iLeft, int, int iWeightLeft, int)
    {
        MIX_store_le16(pDestination, (iLeft * iWeightLeft) >> MIX_ACCUMULATOR_SHIFT);
        MIX_store_le16(pDestination + 1, (iLeft * iWeightLeft) >> MIX_ACCUMULATOR_SHIFT);
    }

    static inline int LoadRight(const int*) { return 0; }

    static inline void SaveRight(int*, int) {}
};

struct MixStereoMonoCopy
{
    typedef short Unit;
    static const int DEST_UNITS = MIX_DEST_MONO;
    static const int SOURCE_BYTES = MIX_SOURCE_STEREO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_STEREO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int, int)
    {
        MIX_store_le16(pDestination, ((short)MIX_load_le32(pSource) + (MIX_load_le32(pSource) >> 16)) >> 1);
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        int iPair = MIX_load_le32(pSource);
        *piLeft = ((short)iPair + (iPair >> MIX_FRACTION_BITS)) >> 1;
        *piRight = 0;
    }

    static inline int Weight(int, int iStep) { return (MIX_SCALE_UNITY << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int, int, int) { MIX_store_le16(pDestination, iLeft); }

    static inline void Filtered(Unit* pDestination, int iLeft, int, int iWeightLeft, int)
    {
        MIX_store_le16(pDestination, (iLeft * iWeightLeft) >> MIX_ACCUMULATOR_SHIFT);
    }

    static inline int LoadRight(const int*) { return 0; }

    static inline void SaveRight(int* piRight, int) { *piRight = 0; }
};

struct MixStereoStereoCopy
{
    typedef short Unit;
    static const int DEST_UNITS = MIX_DEST_STEREO;
    static const int SOURCE_BYTES = MIX_SOURCE_STEREO_BYTES;
    static const int NEAREST_SHIFT = MIX_RESAMPLE_STEREO_SHIFT;

    static inline void Mix(Unit* pDestination, const unsigned char* pSource, int, int)
    {
        MIX_store_le16(pDestination, (short)MIX_load_le32(pSource));
        MIX_store_le16(pDestination + 1, MIX_load_le32(pSource) >> 16);
    }

    static inline void Load(const unsigned char* pSource, int* piLeft, int* piRight)
    {
        int iPair = MIX_load_le32(pSource);
        *piLeft = (short)iPair;
        *piRight = iPair >> MIX_FRACTION_BITS;
    }

    static inline int Weight(int, int iStep) { return (MIX_SCALE_UNITY << MIX_FRACTION_BITS) / iStep - 1; }

    static inline void Blend(Unit* pDestination, int iLeft, int iRight, int, int)
    {
        MIX_store_le16(pDestination, iLeft);
        MIX_store_le16(pDestination + 1, iRight);
    }

    static inline void Filtered(Unit* pDestination, int iLeft, int iRight, int iWeightLeft, int)
    {
        MIX_store_le16(pDestination, (iLeft * iWeightLeft) >> MIX_ACCUMULATOR_SHIFT);
        MIX_store_le16(pDestination + 1, (iRight * iWeightLeft) >> MIX_ACCUMULATOR_SHIFT);
    }

    static inline int LoadRight(const int* piRight) { return *piRight; }

    static inline void SaveRight(int* piRight, int iRight) { *piRight = iRight; }
};

template <class Mixer>
static inline void MIX_merge_nearest(const void** ppSource, unsigned int* pFraction, const void* pSourceEnd,
                                     void** ppDestination, void* pDestinationEnd, int iStep, int iScaleLeft,
                                     int iScaleRight)
{
    typedef typename Mixer::Unit Unit;
    unsigned int uFraction = *pFraction;
    Unit* pDestination = (Unit*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;

    do
    {
        Mixer::Mix(pDestination, pSource, iScaleLeft, iScaleRight);
        pSource += (uFraction + iStep) >> Mixer::NEAREST_SHIFT;
        uFraction = (uFraction + iStep) & MIX_FRACTION_MASK;
        pDestination += Mixer::DEST_UNITS;
    } while (pDestination < (Unit*)pDestinationEnd && pSource < (const unsigned char*)pSourceEnd);

    *ppDestination = pDestination;
    *ppSource = pSource;
    *pFraction = uFraction;
}

template <class Mixer, bool bResumeChecksSource>
static inline void MIX_merge_filtered(const void** ppSource, unsigned int* pFraction, const void* pSourceEnd,
                                      void** ppDestination, void* pDestinationEnd, int* pLeftValue, int* pRightValue,
                                      int iStep, int iScaleLeft, int iScaleRight)
{
    typedef typename Mixer::Unit Unit;
    unsigned int uFraction = *pFraction;
    int iRight = Mixer::LoadRight(pRightValue);
    int iLeft = *pLeftValue;
    Unit* pDestination = (Unit*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    unsigned int uPosition;
    unsigned int uPrevious;
    int iWeightLeft;
    int iWeightRight;
    int iPreviousLeft;
    int iPreviousRight;
    int iCurrentLeft;
    int iCurrentRight;
    int iState;
    bool bCheckSource;

    if (((unsigned int)iStep & MIX_STEP_INTEGER_MASK) != 0)
    {
        iWeightLeft = Mixer::Weight(iScaleLeft, iStep);
        iWeightRight = Mixer::Weight(iScaleRight, iStep);
        iState = (int)(uFraction >> MIX_STATE_SHIFT);
        uPosition = uFraction & MIX_POSITION_MASK;

        if (iState >= MIX_STATE_CONSUME)
        {
            goto consume;
        }

        bCheckSource = bResumeChecksSource || iState != MIX_STATE_PRODUCE;

        if (iState == MIX_STATE_PRODUCE)
        {
            goto produce;
        }

        do
        {
            iLeft -= (iLeft * (int)uPosition) >> MIX_FRACTION_BITS;
            iRight -= (iRight * (int)uPosition) >> MIX_FRACTION_BITS;

            for (uPosition += iStep; uPosition >= MIX_POSITION_TWO; uPosition -= MIX_POSITION_ONE)
            {
                if (pSource >= (const unsigned char*)pSourceEnd)
                {
                    goto exhausted;
                }

            consume:
                Mixer::Load(pSource, &iCurrentLeft, &iCurrentRight);
                pSource += Mixer::SOURCE_BYTES;
                iLeft += iCurrentLeft;
                iRight += iCurrentRight;
            }

            uPosition &= MIX_FRACTION_MASK;

        produce:
            if (bCheckSource && pSource >= (const unsigned char*)pSourceEnd)
            {
                goto save;
            }

            bCheckSource = true;
            Mixer::Load(pSource, &iCurrentLeft, &iCurrentRight);
            pSource += Mixer::SOURCE_BYTES;
            Mixer::Filtered(pDestination, ((iCurrentLeft * (int)uPosition) >> MIX_FRACTION_BITS) + iLeft,
                            ((iCurrentRight * (int)uPosition) >> MIX_FRACTION_BITS) + iRight, iWeightLeft,
                            iWeightRight);
            pDestination += Mixer::DEST_UNITS;
            iLeft = iCurrentLeft;
            iRight = iCurrentRight;
        } while (pDestination < (Unit*)pDestinationEnd);

        uFraction = uPosition;
        goto done;

    exhausted:
        uPosition |= MIX_STATE_CONSUME_FLAG;

    save:
        *ppDestination = pDestination;
        *ppSource = pSource;
        *pFraction = uPosition | MIX_STATE_PRODUCE_FLAG;
        *pLeftValue = iLeft;
        Mixer::SaveRight(pRightValue, iRight);
        return;
    }
    else
    {
        uFraction <<= MIX_FRACTION_BITS;
        iStep <<= MIX_FRACTION_BITS;

        do
        {
            iPreviousLeft = iLeft;
            iPreviousRight = iRight;
            Mixer::Load(pSource, &iLeft, &iRight);

            do
            {
                Mixer::Blend(pDestination,
                             (((iLeft - iPreviousLeft) * (int)(uFraction >> MIX_BLEND_SHIFT)) >> MIX_BLEND_BITS)
                                 + iPreviousLeft,
                             (((iRight - iPreviousRight) * (int)(uFraction >> MIX_BLEND_SHIFT)) >> MIX_BLEND_BITS)
                                 + iPreviousRight,
                             iScaleLeft, iScaleRight);
                pDestination += Mixer::DEST_UNITS;

                if (pDestination >= (Unit*)pDestinationEnd)
                {
                    goto full;
                }

                uPrevious = uFraction;
                uFraction += iStep;
            } while (uFraction > uPrevious);
        } while ((pSource += Mixer::SOURCE_BYTES) < (const unsigned char*)pSourceEnd);

        uFraction >>= MIX_FRACTION_BITS;
        goto done;

    full:
        uPrevious = uFraction;
        uFraction += iStep;
        pSource += Mixer::SOURCE_BYTES;

        if (uFraction > uPrevious)
        {
            pSource -= Mixer::SOURCE_BYTES;
            iLeft = iPreviousLeft;
            iRight = iPreviousRight;
        }

        uFraction >>= MIX_FRACTION_BITS;
    }

done:
    *ppDestination = pDestination;
    *ppSource = pSource;
    *pFraction = uFraction;
    *pLeftValue = iLeft;
    Mixer::SaveRight(pRightValue, iRight);
}

void Merge_DestMono_SrcMono_Src16_NoVolume_NoResample(int** ppDestination, void* pDestinationEnd, const void** ppSource,
                                                      unsigned int* pFraction, const void* pSourceEnd, int iStep,
                                                      int iScaleLeft, int iScaleRight, int* pLeftValue,
                                                      int* pRightValue)
{
    int* pDestination = (int*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        *pDestination += MIX_load_le16(pSource) * MIX_SCALE_UNITY;
        pDestination += MIX_DEST_MONO;

        if (pDestination >= (int*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_MONO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_MONO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestStereo_SrcMono_Src16_NoVolume_NoResample(int** ppDestination, void* pDestinationEnd,
                                                        const void** ppSource, unsigned int* pFraction,
                                                        const void* pSourceEnd, int iStep, int iScaleLeft,
                                                        int iScaleRight, int* pLeftValue, int* pRightValue)
{
    int* pDestination = (int*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        int iSample = MIX_load_le16(pSource) * MIX_SCALE_UNITY;
        pDestination[0] += iSample;
        pDestination[1] += iSample;
        pDestination += MIX_DEST_STEREO;

        if (pDestination >= (int*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_MONO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_MONO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestMono_SrcStereo_Src16_NoVolume_NoResample(int** ppDestination, void* pDestinationEnd,
                                                        const void** ppSource, unsigned int* pFraction,
                                                        const void* pSourceEnd, int iStep, int iScaleLeft,
                                                        int iScaleRight, int* pLeftValue, int* pRightValue)
{
    int* pDestination = (int*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        *pDestination += ((short)MIX_load_le32(pSource) + (MIX_load_le32(pSource) >> 16)) * MIX_SCALE_UNITY;
        pDestination += MIX_DEST_MONO;

        if (pDestination >= (int*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_STEREO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_STEREO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestStereo_SrcStereo_Src16_NoVolume_NoResample(int** ppDestination, void* pDestinationEnd,
                                                          const void** ppSource, unsigned int* pFraction,
                                                          const void* pSourceEnd, int iStep, int iScaleLeft,
                                                          int iScaleRight, int* pLeftValue, int* pRightValue)
{
    int* pDestination = (int*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        int iSample = MIX_load_le32(pSource);
        pDestination[1] += (iSample >> 16) * MIX_SCALE_UNITY;
        pDestination[0] += (short)iSample * MIX_SCALE_UNITY;
        pDestination += MIX_DEST_STEREO;

        if (pDestination >= (int*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_STEREO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_STEREO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestMono_SrcMono_Src16_Volume_NoResample(int** ppDestination, void* pDestinationEnd, const void** ppSource,
                                                    unsigned int* pFraction, const void* pSourceEnd, int iStep,
                                                    int iScaleLeft, int iScaleRight, int* pLeftValue, int* pRightValue)
{
    int* pDestination = (int*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        *pDestination += MIX_load_le16(pSource) * iScaleLeft;
        pDestination += MIX_DEST_MONO;

        if (pDestination >= (int*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_MONO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_MONO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestStereo_SrcMono_Src16_Volume_NoResample(int** ppDestination, void* pDestinationEnd, const void** ppSource,
                                                      unsigned int* pFraction, const void* pSourceEnd, int iStep,
                                                      int iScaleLeft, int iScaleRight, int* pLeftValue,
                                                      int* pRightValue)
{
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int* pDestination = (int*)*ppDestination;
    int bFull = 0;

    do
    {
        int iSample = MIX_load_le16(pSource);
        pDestination[1] += iSample * iScaleRight;
        pDestination[0] += iSample * iScaleLeft;
        pDestination += MIX_DEST_STEREO;

        if (pDestination >= (int*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_MONO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_MONO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestMono_SrcStereo_Src16_Volume_NoResample(int** ppDestination, void* pDestinationEnd, const void** ppSource,
                                                      unsigned int* pFraction, const void* pSourceEnd, int iStep,
                                                      int iScaleLeft, int iScaleRight, int* pLeftValue,
                                                      int* pRightValue)
{
    int* pDestination = (int*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        *pDestination += (short)MIX_load_le32(pSource) * iScaleLeft + (MIX_load_le32(pSource) >> 16) * iScaleRight;
        pDestination += MIX_DEST_MONO;

        if (pDestination >= (int*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_STEREO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_STEREO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestStereo_SrcStereo_Src16_Volume_NoResample(int** ppDestination, void* pDestinationEnd,
                                                        const void** ppSource, unsigned int* pFraction,
                                                        const void* pSourceEnd, int iStep, int iScaleLeft,
                                                        int iScaleRight, int* pLeftValue, int* pRightValue)
{
    int* pDestination = (int*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        int iSample = MIX_load_le32(pSource);
        pDestination[1] += (iSample >> 16) * iScaleRight;
        pDestination[0] += (short)iSample * iScaleLeft;
        pDestination += MIX_DEST_STEREO;

        if (pDestination >= (int*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_STEREO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_STEREO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestMono_SrcMono_Src16_Copy16_NoResample(int** ppDestination, void* pDestinationEnd, const void** ppSource,
                                                    unsigned int* pFraction, const void* pSourceEnd, int iStep,
                                                    int iScaleLeft, int iScaleRight, int* pLeftValue, int* pRightValue)
{
    short* pDestination = (short*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        MIX_store_le16(pDestination, MIX_load_le16(pSource));
        pDestination += MIX_DEST_MONO;

        if (pDestination >= (short*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_MONO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_MONO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestStereo_SrcMono_Src16_Copy16_NoResample(int** ppDestination, void* pDestinationEnd, const void** ppSource,
                                                      unsigned int* pFraction, const void* pSourceEnd, int iStep,
                                                      int iScaleLeft, int iScaleRight, int* pLeftValue,
                                                      int* pRightValue)
{
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    short* pDestination = (short*)*ppDestination;
    int bFull = 0;

    do
    {
        short iSample = MIX_load_le16(pSource);
        MIX_store_le16(pDestination, iSample);
        MIX_store_le16(pDestination + 1, iSample);
        pDestination += MIX_DEST_STEREO;

        if (pDestination >= (short*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_MONO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_MONO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestMono_SrcStereo_Src16_Copy16_NoResample(int** ppDestination, void* pDestinationEnd, const void** ppSource,
                                                      unsigned int* pFraction, const void* pSourceEnd, int iStep,
                                                      int iScaleLeft, int iScaleRight, int* pLeftValue,
                                                      int* pRightValue)
{
    short* pDestination = (short*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        MIX_store_le16(pDestination, ((short)MIX_load_le32(pSource) + (MIX_load_le32(pSource) >> 16)) >> 1);
        pDestination += MIX_DEST_MONO;

        if (pDestination >= (short*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_STEREO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_STEREO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestStereo_SrcStereo_Src16_Copy16_NoResample(int** ppDestination, void* pDestinationEnd,
                                                        const void** ppSource, unsigned int* pFraction,
                                                        const void* pSourceEnd, int iStep, int iScaleLeft,
                                                        int iScaleRight, int* pLeftValue, int* pRightValue)
{
    short* pDestination = (short*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    int bFull = 0;

    do
    {
        MIX_store_le16(pDestination, (short)MIX_load_le32(pSource));
        MIX_store_le16(pDestination + 1, MIX_load_le32(pSource) >> 16);
        pDestination += MIX_DEST_STEREO;

        if (pDestination >= (short*)pDestinationEnd)
        {
            bFull = 1;
            break;
        }

        pSource += MIX_SOURCE_STEREO_BYTES;
    } while (pSource < (const unsigned char*)pSourceEnd);

    if (bFull)
    {
        pSource += MIX_SOURCE_STEREO_BYTES;
    }

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
}

void Merge_DestStereo_SrcStereo_Src16_Copy16_Resample(int** ppDestination, void* pDestinationEnd, const void** ppSource,
                                                      unsigned int* pFraction, const void* pSourceEnd, int iStep,
                                                      int iScaleLeft, int iScaleRight, int* pLeftValue,
                                                      int* pRightValue)
{
    unsigned int uFraction = *pFraction;
    short* pDestination = (short*)*ppDestination;
    const unsigned char* pSource = (const unsigned char*)*ppSource;

    do
    {
        MIX_store_le16(pDestination, (short)MIX_load_le32(pSource));
        MIX_store_le16(pDestination + 1, MIX_load_le32(pSource) >> 16);
        pSource += (uFraction + iStep) >> MIX_RESAMPLE_STEREO_SHIFT;
        uFraction = (uFraction + iStep) & MIX_FRACTION_MASK;
        pDestination += MIX_DEST_STEREO;
    } while (pDestination < (short*)pDestinationEnd && pSource < (const unsigned char*)pSourceEnd);

    *ppDestination = (int*)pDestination;
    *ppSource = pSource;
    *pFraction = uFraction;
}

void Merge_DestMono_SrcStereo_Src16_Copy16_DownFiltered(int** ppDestination, void* pDestinationEnd,
                                                        const void** ppSource, unsigned int* pFraction,
                                                        const void* pSourceEnd, int iStep, int iScaleLeft,
                                                        int iScaleRight, int* pLeftValue, int* pRightValue)
{
    MIX_merge_filtered<MixStereoMonoCopy, true>(ppSource, pFraction, pSourceEnd, (void**)ppDestination, pDestinationEnd,
                                                pLeftValue, pRightValue, iStep, iScaleLeft, iScaleRight);
}

void Merge_DestStereo_SrcStereo_Src16_Copy16_DownFiltered(int** ppDestination, void* pDestinationEnd,
                                                          const void** ppSource, unsigned int* pFraction,
                                                          const void* pSourceEnd, int iStep, int iScaleLeft,
                                                          int iScaleRight, int* pLeftValue, int* pRightValue)
{
    MIX_merge_filtered<MixStereoStereoCopy, false>(ppSource, pFraction, pSourceEnd, (void**)ppDestination,
                                                   pDestinationEnd, pLeftValue, pRightValue, iStep, iScaleLeft,
                                                   iScaleRight);
}

extern "C" void MSS_mixer_merge(void* pSource, unsigned int* pFraction, unsigned short* pSourceEnd,
                                void** ppDestination, unsigned short* pDestinationEnd, int* pLeftValue,
                                int* pRightValue, unsigned int uStep, int iScaleLeft, int iScaleRight,
                                unsigned int uOperation)
{
    const void** ppSource = (const void**)pSource;

    if (pDestinationEnd <= (unsigned short*)*ppDestination)
    {
        return;
    }

    switch (uOperation)
    {
    case MIX_OP_SRC_16:
    case MIX_OP_SRC_16 | MIX_OP_FILTER:
        Merge_DestMono_SrcMono_Src16_NoVolume_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                         pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                         pRightValue);
        return;

    case MIX_OP_SRC_16 | MIX_OP_DEST_STEREO:
    case MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_DEST_STEREO:
        Merge_DestStereo_SrcMono_Src16_NoVolume_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                           pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                           pRightValue);
        return;

    case MIX_OP_SRC_16 | MIX_OP_SRC_STEREO:
    case MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO:
        Merge_DestMono_SrcStereo_Src16_NoVolume_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                           pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                           pRightValue);
        return;

    case MIX_OP_SRC_16 | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
    case MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        Merge_DestStereo_SrcStereo_Src16_NoVolume_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                             pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                             pRightValue);
        return;

    case MIX_OP_RESAMPLE | MIX_OP_SRC_16:
        MIX_merge_nearest<MixMonoMonoFixed>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                            iScaleLeft, iScaleRight);
        return;

    case MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_DEST_STEREO:
        MIX_merge_nearest<MixMonoStereoFixed>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                              iScaleLeft, iScaleRight);
        return;

    case MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER:
        MIX_merge_filtered<MixMonoMonoFixed, true>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                   pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_DEST_STEREO:
        MIX_merge_filtered<MixMonoStereoFixed, true>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                     pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO:
        MIX_merge_nearest<MixStereoMonoFixed>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                              iScaleLeft, iScaleRight);
        return;

    case MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        MIX_merge_nearest<MixStereoStereoFixed>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                                iScaleLeft, iScaleRight);
        return;

    case MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO:
        MIX_merge_filtered<MixStereoMonoFixed, true>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                     pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        MIX_merge_filtered<MixStereoStereoFixed, false>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                        pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_VOLUME | MIX_OP_SRC_16:
    case MIX_OP_VOLUME | MIX_OP_SRC_16 | MIX_OP_FILTER:
        Merge_DestMono_SrcMono_Src16_Volume_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                       pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                       pRightValue);
        return;

    case MIX_OP_VOLUME | MIX_OP_SRC_16 | MIX_OP_DEST_STEREO:
    case MIX_OP_VOLUME | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_DEST_STEREO:
        Merge_DestStereo_SrcMono_Src16_Volume_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                         pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                         pRightValue);
        return;

    case MIX_OP_VOLUME | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO:
    case MIX_OP_VOLUME | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO:
        Merge_DestMono_SrcStereo_Src16_Volume_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                         pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                         pRightValue);
        return;

    case MIX_OP_VOLUME | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
    case MIX_OP_VOLUME | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        Merge_DestStereo_SrcStereo_Src16_Volume_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                           pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                           pRightValue);
        return;

    case MIX_OP_VOLUME | MIX_OP_RESAMPLE | MIX_OP_SRC_16:
        MIX_merge_nearest<MixMonoMonoVolume>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                             iScaleLeft, iScaleRight);
        return;

    case MIX_OP_VOLUME | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_DEST_STEREO:
        MIX_merge_nearest<MixMonoStereoVolume>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                               iScaleLeft, iScaleRight);
        return;

    case MIX_OP_VOLUME | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER:
        MIX_merge_filtered<MixMonoMonoVolume, true>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                    pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_VOLUME | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_DEST_STEREO:
        MIX_merge_filtered<MixMonoStereoVolume, true>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                      pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_VOLUME | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO:
        MIX_merge_nearest<MixStereoMonoVolume>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                               iScaleLeft, iScaleRight);
        return;

    case MIX_OP_VOLUME | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        MIX_merge_nearest<MixStereoStereoVolume>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                                 iScaleLeft, iScaleRight);
        return;

    case MIX_OP_VOLUME | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO:
        MIX_merge_filtered<MixStereoMonoVolume, false>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                       pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_VOLUME | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        MIX_merge_filtered<MixStereoStereoVolume, false>(ppSource, pFraction, pSourceEnd, ppDestination,
                                                         pDestinationEnd, pLeftValue, pRightValue, uStep, iScaleLeft,
                                                         iScaleRight);
        return;

    case MIX_OP_COPY16 | MIX_OP_SRC_16:
    case MIX_OP_COPY16 | MIX_OP_SRC_16 | MIX_OP_FILTER:
        Merge_DestMono_SrcMono_Src16_Copy16_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                       pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                       pRightValue);
        return;

    case MIX_OP_COPY16 | MIX_OP_SRC_16 | MIX_OP_DEST_STEREO:
    case MIX_OP_COPY16 | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_DEST_STEREO:
        Merge_DestStereo_SrcMono_Src16_Copy16_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                         pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                         pRightValue);
        return;

    case MIX_OP_COPY16 | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO:
    case MIX_OP_COPY16 | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO:
        Merge_DestMono_SrcStereo_Src16_Copy16_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                         pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                         pRightValue);
        return;

    case MIX_OP_COPY16 | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
    case MIX_OP_COPY16 | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        Merge_DestStereo_SrcStereo_Src16_Copy16_NoResample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                           pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                           pRightValue);
        return;

    case MIX_OP_COPY16 | MIX_OP_RESAMPLE | MIX_OP_SRC_16:
        MIX_merge_nearest<MixMonoMonoCopy>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                           iScaleLeft, iScaleRight);
        return;

    case MIX_OP_COPY16 | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_DEST_STEREO:
        MIX_merge_nearest<MixMonoStereoCopy>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                             iScaleLeft, iScaleRight);
        return;

    case MIX_OP_COPY16 | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER:
        MIX_merge_filtered<MixMonoMonoCopy, true>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                  pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_COPY16 | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_DEST_STEREO:
        MIX_merge_filtered<MixMonoStereoCopy, true>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd,
                                                    pLeftValue, pRightValue, uStep, iScaleLeft, iScaleRight);
        return;

    case MIX_OP_COPY16 | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO:
        MIX_merge_nearest<MixStereoMonoCopy>(ppSource, pFraction, pSourceEnd, ppDestination, pDestinationEnd, uStep,
                                             iScaleLeft, iScaleRight);
        return;

    case MIX_OP_COPY16 | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        Merge_DestStereo_SrcStereo_Src16_Copy16_Resample((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                         pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                         pRightValue);
        return;

    case MIX_OP_COPY16 | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO:
        Merge_DestMono_SrcStereo_Src16_Copy16_DownFiltered((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                           pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                           pRightValue);
        return;

    case MIX_OP_COPY16 | MIX_OP_RESAMPLE | MIX_OP_SRC_16 | MIX_OP_FILTER | MIX_OP_SRC_STEREO | MIX_OP_DEST_STEREO:
        Merge_DestStereo_SrcStereo_Src16_Copy16_DownFiltered((int**)ppDestination, pDestinationEnd, ppSource, pFraction,
                                                             pSourceEnd, uStep, iScaleLeft, iScaleRight, pLeftValue,
                                                             pRightValue);
        return;

    default:
        return;
    }
}

struct MIX_ADPCM_STATE
{
    int iBlockSize;
    unsigned int uNibblesLeft;
    int iBlockLeft;
    unsigned int uStep;
    int iUnused0;
    int iSample;
    int iUnused1;
    int iUnused2;
    unsigned int uWordLeft;
    unsigned int uWordRight;
};

static const int MIX_ADPCM_STEP_COUNT = 89;
static const int MIX_ADPCM_LAST_INDEX = MIX_ADPCM_STEP_COUNT - 1;
static const int MIX_ADPCM_DELTA_UNIT = 1 << MIX_ADPCM_DELTA_SHIFT;

#define MIX_ADPCM_MAGNITUDE(step, nibble) \
    (((step) >> 3) + (((nibble) & 4) ? (step) : 0) + (((nibble) & 2) ? ((step) >> 1) : 0) \
     + (((nibble) & 1) ? ((step) >> 2) : 0))

#define MIX_ADPCM_CLAMP_INDEX(index) \
    (((index) < 0) ? 0 : (((index) > MIX_ADPCM_LAST_INDEX) ? MIX_ADPCM_LAST_INDEX : (index)))

#define MIX_ADPCM_NEXT(index, adjust) (MIX_ADPCM_CLAMP_INDEX((index) + (adjust)) * (1 << MIX_ADPCM_STEP_SHIFT))

#define MIX_ADPCM_RISE(step, index, nibble, adjust) \
    ((unsigned int)(MIX_ADPCM_MAGNITUDE(step, nibble) * MIX_ADPCM_DELTA_UNIT) \
     | (unsigned int)MIX_ADPCM_NEXT(index, adjust))

#define MIX_ADPCM_FALL(step, index, nibble, adjust) \
    ((unsigned int)(-MIX_ADPCM_MAGNITUDE(step, nibble) * MIX_ADPCM_DELTA_UNIT) \
     | (unsigned int)MIX_ADPCM_NEXT(index, adjust))

#define MIX_ADPCM_ROW(step, index) \
    MIX_ADPCM_RISE(step, index, 0, -1), MIX_ADPCM_RISE(step, index, 1, -1), MIX_ADPCM_RISE(step, index, 2, -1), \
        MIX_ADPCM_RISE(step, index, 3, -1), MIX_ADPCM_RISE(step, index, 4, 2), MIX_ADPCM_RISE(step, index, 5, 4), \
        MIX_ADPCM_RISE(step, index, 6, 6), MIX_ADPCM_RISE(step, index, 7, 8), MIX_ADPCM_FALL(step, index, 0, -1), \
        MIX_ADPCM_FALL(step, index, 1, -1), MIX_ADPCM_FALL(step, index, 2, -1), MIX_ADPCM_FALL(step, index, 3, -1), \
        MIX_ADPCM_FALL(step, index, 4, 2), MIX_ADPCM_FALL(step, index, 5, 4), MIX_ADPCM_FALL(step, index, 6, 6), \
        MIX_ADPCM_FALL(step, index, 7, 8)

static unsigned int MIX_ADPCM_TABLE[] = {
    MIX_ADPCM_ROW(7, 0),      MIX_ADPCM_ROW(8, 1),      MIX_ADPCM_ROW(9, 2),      MIX_ADPCM_ROW(10, 3),
    MIX_ADPCM_ROW(11, 4),     MIX_ADPCM_ROW(12, 5),     MIX_ADPCM_ROW(13, 6),     MIX_ADPCM_ROW(14, 7),
    MIX_ADPCM_ROW(16, 8),     MIX_ADPCM_ROW(17, 9),     MIX_ADPCM_ROW(19, 10),    MIX_ADPCM_ROW(21, 11),
    MIX_ADPCM_ROW(23, 12),    MIX_ADPCM_ROW(25, 13),    MIX_ADPCM_ROW(28, 14),    MIX_ADPCM_ROW(31, 15),
    MIX_ADPCM_ROW(34, 16),    MIX_ADPCM_ROW(37, 17),    MIX_ADPCM_ROW(41, 18),    MIX_ADPCM_ROW(45, 19),
    MIX_ADPCM_ROW(50, 20),    MIX_ADPCM_ROW(55, 21),    MIX_ADPCM_ROW(60, 22),    MIX_ADPCM_ROW(66, 23),
    MIX_ADPCM_ROW(73, 24),    MIX_ADPCM_ROW(80, 25),    MIX_ADPCM_ROW(88, 26),    MIX_ADPCM_ROW(97, 27),
    MIX_ADPCM_ROW(107, 28),   MIX_ADPCM_ROW(118, 29),   MIX_ADPCM_ROW(130, 30),   MIX_ADPCM_ROW(143, 31),
    MIX_ADPCM_ROW(157, 32),   MIX_ADPCM_ROW(173, 33),   MIX_ADPCM_ROW(190, 34),   MIX_ADPCM_ROW(209, 35),
    MIX_ADPCM_ROW(230, 36),   MIX_ADPCM_ROW(253, 37),   MIX_ADPCM_ROW(279, 38),   MIX_ADPCM_ROW(307, 39),
    MIX_ADPCM_ROW(337, 40),   MIX_ADPCM_ROW(371, 41),   MIX_ADPCM_ROW(408, 42),   MIX_ADPCM_ROW(449, 43),
    MIX_ADPCM_ROW(494, 44),   MIX_ADPCM_ROW(544, 45),   MIX_ADPCM_ROW(598, 46),   MIX_ADPCM_ROW(658, 47),
    MIX_ADPCM_ROW(724, 48),   MIX_ADPCM_ROW(796, 49),   MIX_ADPCM_ROW(876, 50),   MIX_ADPCM_ROW(963, 51),
    MIX_ADPCM_ROW(1060, 52),  MIX_ADPCM_ROW(1166, 53),  MIX_ADPCM_ROW(1282, 54),  MIX_ADPCM_ROW(1411, 55),
    MIX_ADPCM_ROW(1552, 56),  MIX_ADPCM_ROW(1707, 57),  MIX_ADPCM_ROW(1878, 58),  MIX_ADPCM_ROW(2066, 59),
    MIX_ADPCM_ROW(2272, 60),  MIX_ADPCM_ROW(2499, 61),  MIX_ADPCM_ROW(2749, 62),  MIX_ADPCM_ROW(3024, 63),
    MIX_ADPCM_ROW(3327, 64),  MIX_ADPCM_ROW(3660, 65),  MIX_ADPCM_ROW(4026, 66),  MIX_ADPCM_ROW(4428, 67),
    MIX_ADPCM_ROW(4871, 68),  MIX_ADPCM_ROW(5358, 69),  MIX_ADPCM_ROW(5894, 70),  MIX_ADPCM_ROW(6484, 71),
    MIX_ADPCM_ROW(7132, 72),  MIX_ADPCM_ROW(7845, 73),  MIX_ADPCM_ROW(8630, 74),  MIX_ADPCM_ROW(9493, 75),
    MIX_ADPCM_ROW(10442, 76), MIX_ADPCM_ROW(11487, 77), MIX_ADPCM_ROW(12635, 78), MIX_ADPCM_ROW(13899, 79),
    MIX_ADPCM_ROW(15289, 80), MIX_ADPCM_ROW(16818, 81), MIX_ADPCM_ROW(18500, 82), MIX_ADPCM_ROW(20350, 83),
    MIX_ADPCM_ROW(22385, 84), MIX_ADPCM_ROW(24623, 85), MIX_ADPCM_ROW(27086, 86), MIX_ADPCM_ROW(29794, 87),
    MIX_ADPCM_ROW(32767, 88)};

static inline unsigned int MIX_adpcm_entry(unsigned int uStep)
{
    return *(const unsigned int*)((const unsigned char*)MIX_ADPCM_TABLE + uStep);
}

static inline int MIX_adpcm_clamp(int iSample)
{
    int iExcess = iSample - MIX_SAMPLE_MAX;
    unsigned int uBiased = (iExcess & (iExcess >> 31)) + (MIX_SAMPLE_MAX - MIX_SAMPLE_MIN);

    return (int)(uBiased & ~((int)uBiased >> 31)) + MIX_SAMPLE_MIN;
}

extern "C" void DecodeADPCM_MONO(void** ppDestination, const void** ppSource, int iDestinationLength, int iSourceLength,
                                 MIX_ADPCM_STATE* pState)
{
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    const unsigned char* pSourceEnd = pSource + iSourceLength;
    const unsigned char* pBlockEnd = pSource + pState->iBlockLeft;
    short* pDestination = (short*)*ppDestination;
    short* pDestinationEnd = (short*)((char*)pDestination + iDestinationLength);
    unsigned int uNibbles = pState->uNibblesLeft;
    unsigned int uStep = pState->uStep;
    unsigned int uWord = pState->uWordLeft;
    int iSample = pState->iSample;
    int iBlockSize = pState->iBlockSize;

    bool bDecode = false;
    bool bFetch = false;
    bool bDone = false;

    if (uNibbles != 0)
    {
        bDecode = true;
    }
    else if (pSource < pBlockEnd)
    {
        bFetch = true;
    }

    do
    {
        if (!bDecode && !bFetch)
        {
            pBlockEnd = pSource + iBlockSize;
            uStep = MIX_load_le32(pSource);
            pSource += MIX_ADPCM_HEADER_BYTES;
            iSample = (short)uStep;
            MIX_store_le16(pDestination, uStep);
            uStep = ((uStep >> MIX_ADPCM_INDEX_SHIFT) & MIX_ADPCM_INDEX_MASK) << MIX_ADPCM_STEP_SHIFT;
            pDestination++;
        }

        do
        {
            if (!bDecode)
            {
                if (!bFetch && pSource >= pSourceEnd)
                {
                    bDone = true;
                    break;
                }

                uWord = MIX_load_le32(pSource);
                uNibbles = MIX_ADPCM_NIBBLES;
                pSource += MIX_ADPCM_WORD_BYTES;
                uStep |= (uWord & MIX_ADPCM_NIBBLE_MASK) << MIX_ADPCM_NIBBLE_SHIFT;
            }

            bDecode = false;
            bFetch = false;

            do
            {
                if (pDestination >= pDestinationEnd)
                {
                    break;
                }

                uStep = MIX_adpcm_entry(uStep);
                iSample = MIX_adpcm_clamp(iSample + ((int)uStep >> MIX_ADPCM_DELTA_SHIFT));
                uWord >>= MIX_ADPCM_NIBBLE_BITS;
                uStep = (uStep & MIX_ADPCM_INDEX_BITS_MASK)
                        | ((uWord & MIX_ADPCM_NIBBLE_MASK) << MIX_ADPCM_NIBBLE_SHIFT);
                MIX_store_le16(pDestination, iSample);
                pDestination++;
                uNibbles--;
            } while (uNibbles != 0);

            if (pDestination >= pDestinationEnd)
            {
                bDone = true;
                break;
            }
        } while (pSource < pBlockEnd);

        if (bDone)
        {
            break;
        }
    } while (pSource < pSourceEnd);

    pState->iSample = iSample;
    pState->uWordLeft = uWord;
    pState->iBlockLeft = (int)(pBlockEnd - pSource);
    pState->uStep = uStep;
    pState->uNibblesLeft = uNibbles;
    *ppDestination = pDestination;
    *ppSource = pSource;
}

extern "C" void DecodeADPCM_MONO_8(void** ppDestination, const void** ppSource, int iDestinationLength,
                                   int iSourceLength, MIX_ADPCM_STATE* pState)
{
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    const unsigned char* pSourceEnd = pSource + iSourceLength;
    const unsigned char* pBlockEnd = pSource + pState->iBlockLeft;
    unsigned char* pDestination = (unsigned char*)*ppDestination;
    unsigned char* pDestinationEnd = pDestination + iDestinationLength;
    unsigned int uNibbles = pState->uNibblesLeft;
    unsigned int uStep = pState->uStep;
    unsigned int uWord = pState->uWordLeft;
    int iSample = pState->iSample;
    int iBlockSize = pState->iBlockSize;

    bool bDecode = false;
    bool bFetch = false;
    bool bDone = false;

    if (uNibbles != 0)
    {
        bDecode = true;
    }
    else if (pSource < pBlockEnd)
    {
        bFetch = true;
    }

    do
    {
        if (!bDecode && !bFetch)
        {
            pBlockEnd = pSource + iBlockSize;
            uStep = MIX_load_le32(pSource);
            pSource += MIX_ADPCM_HEADER_BYTES;
            iSample = (short)uStep;
            *pDestination = (uStep >> MIX_BYTE_SHIFT) + MIX_BYTE_SIGN;
            uStep = ((uStep >> MIX_ADPCM_INDEX_SHIFT) & MIX_ADPCM_INDEX_MASK) << MIX_ADPCM_STEP_SHIFT;
            pDestination++;
        }

        do
        {
            if (!bDecode)
            {
                if (!bFetch && pSource >= pSourceEnd)
                {
                    bDone = true;
                    break;
                }

                uWord = MIX_load_le32(pSource);
                uNibbles = MIX_ADPCM_NIBBLES;
                pSource += MIX_ADPCM_WORD_BYTES;
                uStep |= (uWord & MIX_ADPCM_NIBBLE_MASK) << MIX_ADPCM_NIBBLE_SHIFT;
            }

            bDecode = false;
            bFetch = false;

            do
            {
                if (pDestination >= pDestinationEnd)
                {
                    break;
                }

                uStep = MIX_adpcm_entry(uStep);
                iSample = MIX_adpcm_clamp(iSample + ((int)uStep >> MIX_ADPCM_DELTA_SHIFT));
                uWord >>= MIX_ADPCM_NIBBLE_BITS;
                uStep = (uStep & MIX_ADPCM_INDEX_BITS_MASK)
                        | ((uWord & MIX_ADPCM_NIBBLE_MASK) << MIX_ADPCM_NIBBLE_SHIFT);
                *pDestination = (iSample >> MIX_BYTE_SHIFT) + MIX_BYTE_SIGN;
                pDestination++;
                uNibbles--;
            } while (uNibbles != 0);

            if (pDestination >= pDestinationEnd)
            {
                bDone = true;
                break;
            }
        } while (pSource < pBlockEnd);

        if (bDone)
        {
            break;
        }
    } while (pSource < pSourceEnd);

    pState->iSample = iSample;
    pState->uWordLeft = uWord;
    pState->iBlockLeft = (int)(pBlockEnd - pSource);
    pState->uStep = uStep;
    pState->uNibblesLeft = uNibbles;
    *ppDestination = pDestination;
    *ppSource = pSource;
}

extern "C" void DecodeADPCM_STEREO(void** ppDestination, const void** ppSource, int iDestinationLength,
                                   int iSourceLength, MIX_ADPCM_STATE* pState)
{
    const unsigned char* pSource = (const unsigned char*)*ppSource;
    const unsigned char* pSourceEnd = pSource + iSourceLength;
    const unsigned char* pBlockEnd = pSource + pState->iBlockLeft;
    int* pDestination = (int*)*ppDestination;
    int* pDestinationEnd = (int*)((char*)pDestination + iDestinationLength);
    unsigned int uNibbles = pState->uNibblesLeft;
    unsigned int uStepLeft = pState->uStep & MIX_ADPCM_CHANNEL_MASK;
    unsigned int uStepRight = pState->uStep >> MIX_ADPCM_CHANNEL_BITS;
    unsigned int uWordLeft = pState->uWordLeft;
    unsigned int uWordRight = pState->uWordRight;
    unsigned int uEntryLeft;
    unsigned int uEntryRight;
    unsigned int uHeaderLeft;
    unsigned int uHeaderRight;
    int iSampleLeft = (short)pState->iSample;
    int iSampleRight = pState->iSample >> MIX_ADPCM_CHANNEL_BITS;
    int iBlockSize = pState->iBlockSize;

    bool bDecode = false;
    bool bFetch = false;
    bool bDone = false;

    if (uNibbles != 0)
    {
        bDecode = true;
    }
    else if (pSource < pBlockEnd)
    {
        bFetch = true;
    }

    do
    {
        if (!bDecode && !bFetch)
        {
            uHeaderLeft = MIX_load_le32(pSource);
            uHeaderRight = MIX_load_le32(pSource + MIX_ADPCM_HEADER_BYTES);
            pBlockEnd = pSource + iBlockSize;
            pSource += MIX_ADPCM_HEADER_BYTES * 2;
            iSampleLeft = (short)uHeaderLeft;
            iSampleRight = (short)uHeaderRight;
            uStepLeft = ((uHeaderLeft >> MIX_ADPCM_INDEX_SHIFT) & MIX_ADPCM_INDEX_MASK) << MIX_ADPCM_STEP_SHIFT;
            uStepRight = ((uHeaderRight >> MIX_ADPCM_INDEX_SHIFT) & MIX_ADPCM_INDEX_MASK) << MIX_ADPCM_STEP_SHIFT;
            MIX_store_le32(pDestination,
                           (iSampleRight << MIX_ADPCM_CHANNEL_BITS) | (iSampleLeft & MIX_ADPCM_CHANNEL_MASK));
            pDestination++;
        }

        do
        {
            if (!bDecode)
            {
                if (!bFetch && pSource >= pSourceEnd)
                {
                    bDone = true;
                    break;
                }

                uWordLeft = MIX_load_le32(pSource);
                uWordRight = MIX_load_le32(pSource + MIX_ADPCM_WORD_BYTES);
                uNibbles = MIX_ADPCM_NIBBLES;
                pSource += MIX_ADPCM_WORD_BYTES * 2;
                uStepLeft |= (uWordLeft & MIX_ADPCM_NIBBLE_MASK) << MIX_ADPCM_NIBBLE_SHIFT;
                uStepRight |= (uWordRight & MIX_ADPCM_NIBBLE_MASK) << MIX_ADPCM_NIBBLE_SHIFT;
            }

            bDecode = false;
            bFetch = false;

            do
            {
                if (pDestination >= pDestinationEnd)
                {
                    break;
                }

                uEntryLeft = MIX_adpcm_entry(uStepLeft);
                uEntryRight = MIX_adpcm_entry(uStepRight);
                iSampleLeft = MIX_adpcm_clamp(iSampleLeft + ((int)uEntryLeft >> MIX_ADPCM_DELTA_SHIFT));
                iSampleRight = MIX_adpcm_clamp(iSampleRight + ((int)uEntryRight >> MIX_ADPCM_DELTA_SHIFT));
                uWordLeft >>= MIX_ADPCM_NIBBLE_BITS;
                uWordRight >>= MIX_ADPCM_NIBBLE_BITS;
                uStepLeft = (uEntryLeft & MIX_ADPCM_INDEX_BITS_MASK)
                            | ((uWordLeft & MIX_ADPCM_NIBBLE_MASK) << MIX_ADPCM_NIBBLE_SHIFT);
                uStepRight = (uEntryRight & MIX_ADPCM_INDEX_BITS_MASK)
                             | ((uWordRight & MIX_ADPCM_NIBBLE_MASK) << MIX_ADPCM_NIBBLE_SHIFT);
                MIX_store_le32(pDestination,
                               (iSampleRight << MIX_ADPCM_CHANNEL_BITS) | (iSampleLeft & MIX_ADPCM_CHANNEL_MASK));
                pDestination++;
                uNibbles--;
            } while (uNibbles != 0);

            if (pDestination >= pDestinationEnd)
            {
                bDone = true;
                break;
            }
        } while (pSource < pBlockEnd);

        if (bDone)
        {
            break;
        }
    } while (pSource < pSourceEnd);

    pState->uNibblesLeft = uNibbles;
    pState->uWordRight = uWordRight;
    pState->uStep = (uStepRight << MIX_ADPCM_CHANNEL_BITS) | (uStepLeft & MIX_ADPCM_CHANNEL_MASK);
    pState->iBlockLeft = (int)(pBlockEnd - pSource);
    pState->uWordLeft = uWordLeft;
    pState->iSample = (iSampleRight << MIX_ADPCM_CHANNEL_BITS) | (iSampleLeft & MIX_ADPCM_CHANNEL_MASK);
    *ppDestination = pDestination;
    *ppSource = pSource;
}

extern "C" void MSS_mixer_adpcm_decode(unsigned int* ppDestination, unsigned int* ppSource, int iDestinationLength,
                                       int iSourceLength, unsigned int uOperation, int* pState)
{
    if ((uOperation & MIX_OP_SRC_16) != 0)
    {
        if ((uOperation & MIX_OP_SRC_STEREO) != 0)
        {
            DecodeADPCM_STEREO((void**)ppDestination, (const void**)ppSource, iDestinationLength, iSourceLength,
                               (MIX_ADPCM_STATE*)pState);
        }
        else
        {
            DecodeADPCM_MONO((void**)ppDestination, (const void**)ppSource, iDestinationLength, iSourceLength,
                             (MIX_ADPCM_STATE*)pState);
        }
    }
    else
    {
        DecodeADPCM_MONO_8((void**)ppDestination, (const void**)ppSource, iDestinationLength, iSourceLength,
                           (MIX_ADPCM_STATE*)pState);
    }
}

static inline int MIX_hex_digit(char cDigit)
{
    int iValue = (char)(cDigit - '0');

    if (iValue > 9)
    {
        iValue = (char)(iValue - ('a' - '0' - 10));
    }

    return iValue;
}

#define MIX_HEX_DIGIT(cValue, cDigit) \
    cValue = (cDigit) - '0'; \
    if (cValue > 9) \
    { \
        cValue -= 'a' - '0' - 10; \
    }

int copy_event_step(const unsigned char** ppEvent, char** ppOutput, char* pOutputEnd, const char* pFormat, ...)
{
    va_list pArguments;
    MilesEventString* pString;
    void* pValue;
    char cSize;
    int iLength;
    signed char iFirst;
    signed char iSecond;
    signed char iThird;
    signed char iFourth;

    va_start(pArguments, pFormat);
    *ppEvent += MILES_EVENT_STEP_PREFIX;

    for (; *pFormat != '\0'; pFormat++)
    {
        switch (*pFormat)
        {
        case 's':
            pString = va_arg(pArguments, MilesEventString*);
            pString->pText = (const char*)*ppEvent;
            pString->iLength = 0;

            while (**ppEvent != ';')
            {
                pString->iLength++;
                (*ppEvent)++;
            }

            (*ppEvent)++;

            if (*ppOutput + pString->iLength + 1 >= pOutputEnd)
            {
                AIL_set_error(MILES_EVENT_BUFFER_ERROR);
                return 0;
            }

            memcpy(*ppOutput, pString->pText, pString->iLength);
            pString->pText = *ppOutput;
            *ppOutput += pString->iLength;
            **ppOutput = '\0';
            (*ppOutput)++;
            break;

        case 'c':
            MIX_HEX_DIGIT(iFirst, **ppEvent);
            (*ppEvent)++;
            MIX_HEX_DIGIT(iSecond, **ppEvent);
            *ppEvent += 2;
            cSize = pFormat[1];
            pValue = va_arg(pArguments, void*);

            if (cSize == '4')
            {
                *(int*)pValue = iFirst * 16 + iSecond;
            }
            else
            {
                *(char*)pValue = (char)(iFirst * 16 + iSecond);
            }

            pFormat++;
            break;

        case 'u':
            iFirst = (char)(**ppEvent - '0');

            if (iFirst > 9)
            {
                iFirst += 9;
            }

            (*ppEvent)++;
            MIX_HEX_DIGIT(iSecond, **ppEvent);
            (*ppEvent)++;
            MIX_HEX_DIGIT(iThird, **ppEvent);
            (*ppEvent)++;
            MIX_HEX_DIGIT(iFourth, **ppEvent);
            *ppEvent += 2;
            *va_arg(pArguments, short*) = (short)((iFirst << 12) + (iSecond << 8) + (iThird << 4) + iFourth);
            break;

        case 'f':
            iLength = 0;
            *va_arg(pArguments, float*) = atof_with_len((const char*)*ppEvent, &iLength);
            *ppEvent += iLength + 1;
            break;

        case 'd':
            MIX_HEX_DIGIT(iFirst, **ppEvent);
            *ppEvent += 2;
            cSize = pFormat[1];
            pValue = va_arg(pArguments, void*);

            if (cSize == '4')
            {
                *(int*)pValue = iFirst;
            }
            else
            {
                *(char*)pValue = (char)iFirst;
            }

            pFormat++;
            break;

        default:
            break;
        }
    }

    va_end(pArguments);
    return 1;
}

static const unsigned char* MIX_event_terminator(const unsigned char* pCursor)
{
    unsigned char bTerminator = *pCursor;

    if (bTerminator != 0 && bTerminator != '\r' && bTerminator != '\n')
    {
        return pCursor;
    }

    return NULL;
}

extern "C" const unsigned char* AIL_next_event_step(const unsigned char* pEvent, EVENT_STEP_INFO** ppStep,
                                                    EVENT_STEP_INFO* pBuffer, unsigned int uBufferSize)
{
    MilesEventStep* pStep = (MilesEventStep*)pBuffer;
    const unsigned char* pCursor = pEvent;
    const unsigned char* pListStart;
    const unsigned char* pListEnd;
    char* pOutput;
    char* pOutputEnd;
    char* pItems;
    char* pItem;
    char* pScan;
    unsigned char bTerminator;
    unsigned int uPoint;
    int iResult;
    int iCount;
    int iLength;
    int iBlock;
    int iIndex;
    int iOffset;
    int iVersion;
    int iConsumed;
    int iPoint;

    if (*pEvent == 0)
    {
        return NULL;
    }

    if (uBufferSize < MILES_EVENT_HEADER_SIZE)
    {
        AIL_set_error(MILES_EVENT_BUFFER_ERROR);
        return NULL;
    }

    pOutput = (char*)pBuffer + MILES_EVENT_HEADER_SIZE;
    *ppStep = NULL;
    pOutputEnd = (char*)pBuffer + uBufferSize;
    pStep->uType = *pEvent - '0';

    switch (pStep->uType)
    {
    case 0:
        *ppStep = pBuffer;
        return MIX_event_terminator(pCursor);

    case 1:
        iResult = copy_event_step(
            &pCursor, &pOutput, pOutputEnd, "ssssssssd4d1d1uuc1c1sfffffd1d1", &pStep->start.text0, &pStep->start.text1,
            &pStep->start.text2, &pStep->start.text3, &pStep->start.text4, &pStep->start.text5, &pStep->start.text6,
            &pStep->start.text7, &pStep->start.iValue, &pStep->start.bValue0, &pStep->start.bValue1,
            &pStep->start.uValue0, &pStep->start.uValue1, &pStep->start.bValue2, &pStep->start.bValue3,
            &pStep->start.text8, &pStep->start.fValue[0], &pStep->start.fValue[1], &pStep->start.fValue[2],
            &pStep->start.fValue[3], &pStep->start.fValue[4], &pStep->start.bValue4, &pStep->start.bValue5);
        break;

    case 2:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "sssssc1d1fd1", &pStep->second.text0,
                                  &pStep->second.text1, &pStep->second.text2, &pStep->second.text3,
                                  &pStep->second.text4, &pStep->second.bValue1, &pStep->second.bValue2,
                                  &pStep->second.fValue, &pStep->second.bValue0);
        break;

    case 3:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "sd1", &pStep->third.text0, &pStep->third.bValue);
        break;

    case 4:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "s", &pStep->fourth.text0);
        break;

    case 5:
    case 6:
        pCursor += MILES_EVENT_STEP_PREFIX;
        pStep->list.name.pText = (const char*)pCursor;
        pStep->list.name.iLength = 0;

        while (*pCursor != ';')
        {
            pStep->list.name.iLength++;
            pCursor++;
        }

        pCursor++;

        if (pOutputEnd <= pOutput + pStep->list.name.iLength + 1)
        {
            AIL_set_error(MILES_EVENT_BUFFER_ERROR);
            return NULL;
        }

        memcpy(pOutput, pStep->list.name.pText, pStep->list.name.iLength);
        pStep->list.name.pText = pOutput;
        pOutput[pStep->list.name.iLength] = '\0';
        pOutput += pStep->list.name.iLength + 1;

        iCount = 0;
        pListStart = pCursor;
        pListEnd = pCursor;

        while (*pListEnd != ';')
        {
            if (*pListEnd == ':')
            {
                iCount++;
            }

            pListEnd++;
        }

        if (pCursor != pListEnd)
        {
            iCount++;
        }

        iLength = (int)(pListEnd - pCursor);
        iBlock = iLength + 1;
        pCursor = pListEnd;

        if (pOutputEnd <= pOutput + iCount * (int)sizeof(char*) + iBlock)
        {
            AIL_set_error(MILES_EVENT_BUFFER_ERROR);
            return NULL;
        }

        pStep->list.iCount = iCount;
        pStep->list.ppItems = (char**)pOutput;
        pOutput = (char*)((char**)pOutput + iCount);
        memcpy(pOutput, pListStart, iBlock);
        pOutput[iLength] = '\0';
        pItem = pOutput;
        pScan = pOutput;
        iIndex = 0;

        if (*pScan != '\0')
        {
            iOffset = 0;

            do
            {
                if (*pScan == ':')
                {
                    iIndex++;
                    *(char**)((char*)pStep->list.ppItems + iOffset) = pItem;
                    pItem = pScan + 1;
                    iOffset += (int)sizeof(char*);
                    *pScan = '\0';
                }

                pScan++;
            } while (*pScan != '\0');
        }

        if (pScan != pItem)
        {
            pStep->list.ppItems[iIndex] = pItem;
        }

        *ppStep = pBuffer;
        pCursor++;
        return MIX_event_terminator(pCursor);

    case 7:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "ss", &pStep->seventh.text1, &pStep->seventh.text0);
        break;

    case 8:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "sssd1", &pStep->eighth.text1, &pStep->eighth.text0,
                                  &pStep->eighth.text2, &pStep->eighth.bValue);
        break;

    case 9:
        pCursor += MILES_EVENT_STEP_PREFIX;
        iVersion = atoi_with_len((const char*)pCursor, 0, &iConsumed);
        pCursor += iConsumed + 1;

        if (iVersion != MILES_EVENT_VERSION)
        {
            AIL_set_error(MILES_EVENT_VERSION_ERROR);
            return NULL;
        }

        bTerminator = *pCursor;

        if (bTerminator == 0 || bTerminator == '\r' || bTerminator == '\n')
        {
            return NULL;
        }

        return AIL_next_event_step(pCursor, ppStep, pBuffer, uBufferSize);

    case 10:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "ssfsd1d1d1", &pStep->ramp.text0, &pStep->ramp.text1,
                                  &pStep->ramp.fValue, &pStep->ramp.text2, &pStep->ramp.bValue0, &pStep->ramp.bValue1,
                                  &pStep->ramp.bValue2);
        break;

    case 11:
        pCursor += MILES_EVENT_STEP_PREFIX;
        pStep->blend.name.pText = (const char*)pCursor;
        pStep->blend.name.iLength = 0;

        while (*pCursor != ';')
        {
            pStep->blend.name.iLength++;
            pCursor++;
        }

        pCursor++;

        if (pOutputEnd <= pOutput + pStep->blend.name.iLength + 1)
        {
            AIL_set_error(MILES_EVENT_BUFFER_ERROR);
            return NULL;
        }

        memcpy(pOutput, pStep->blend.name.pText, pStep->blend.name.iLength);
        iLength = pStep->blend.name.iLength;
        pStep->blend.name.pText = pOutput;
        pOutput[iLength] = '\0';
        pOutput += iLength + 1;
        pStep->blend.uPointCount = MIX_hex_digit(*pCursor);
        pCursor += MILES_EVENT_STEP_PREFIX;
        memset(pStep->blend.fRangeStart, 0, sizeof(pStep->blend.fRangeStart));
        memset(pStep->blend.fFadeInEnd, 0, sizeof(pStep->blend.fFadeInEnd));
        memset(pStep->blend.fFadeOutStart, 0, sizeof(pStep->blend.fFadeOutStart));
        memset(pStep->blend.fRangeEnd, 0, sizeof(pStep->blend.fRangeEnd));
        memset(pStep->blend.fPitchAtStart, 0, sizeof(pStep->blend.fPitchAtStart));
        memset(pStep->blend.fPitchAtEnd, 0, sizeof(pStep->blend.fPitchAtEnd));
        uPoint = 0;

        if (pStep->blend.uPointCount != 0)
        {
            do
            {
                iPoint = 0;
                pStep->blend.fRangeStart[uPoint] = atof_with_len((const char*)pCursor, &iPoint);
                pCursor += iPoint + 1;
                iPoint = 0;
                pStep->blend.fFadeInEnd[uPoint] = atof_with_len((const char*)pCursor, &iPoint);
                pCursor += iPoint + 1;
                iPoint = 0;
                pStep->blend.fFadeOutStart[uPoint] = atof_with_len((const char*)pCursor, &iPoint);
                pCursor += iPoint + 1;
                iPoint = 0;
                pStep->blend.fRangeEnd[uPoint] = atof_with_len((const char*)pCursor, &iPoint);
                pCursor += iPoint + 1;
                iPoint = 0;
                pStep->blend.fPitchAtStart[uPoint] = atof_with_len((const char*)pCursor, &iPoint);
                pCursor += iPoint + 1;
                iPoint = 0;
                pStep->blend.fPitchAtEnd[uPoint] = atof_with_len((const char*)pCursor, &iPoint);
                pCursor += iPoint + 1;
                uPoint = (uPoint + 1) & 0xff;
            } while (uPoint < pStep->blend.uPointCount);
        }

        *ppStep = pBuffer;
        return MIX_event_terminator(pCursor);

    case 12:
        *ppStep = pBuffer;
        pCursor += MILES_EVENT_STEP_PREFIX;
        return MIX_event_terminator(pCursor);

    case 13:
    case 14:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "s", &pStep->simple.text0);
        break;

    case 15:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "ssssd4d4d4c4d4", &pStep->wave.text0,
                                  &pStep->wave.text1, &pStep->wave.text2, &pStep->wave.text3, &pStep->wave.iValue0,
                                  &pStep->wave.iValue1, &pStep->wave.iValue2, &pStep->wave.iValue3,
                                  &pStep->wave.iValue4);
        break;

    case 16:
        iResult = copy_event_step(&pCursor, &pOutput, pOutputEnd, "sffd4d4fff", &pStep->last.text0,
                                  &pStep->last.fValue0, &pStep->last.fValue1, &pStep->last.iValue0,
                                  &pStep->last.iValue1, &pStep->last.fValue2, &pStep->last.fValue3,
                                  &pStep->last.fValue4);
        break;

    default:
        *ppStep = pBuffer;
        return MIX_event_terminator(pCursor);
    }

    if (iResult != 0)
    {
        *ppStep = pBuffer;
        return MIX_event_terminator(pCursor);
    }

    return NULL;
}

extern "C" int get_system_speaker_configuration(unsigned int* puConfiguration)
{
    *puConfiguration = MIX_SPEAKER_CONFIGURATION_STEREO;
    return 1;
}

extern "C" int MIX_RIB_MAIN(unsigned int uProvider, int iUpDown, RIB_ALLOC_PROVIDER_HANDLE_CALLBACK,
                            RIB_REGISTER_INTERFACE_CALLBACK pRegisterInterface,
                            RIB_UNREGISTER_INTERFACE_CALLBACK pUnregisterInterface)
{
    if (iUpDown != 0)
    {
        pRegisterInterface(uProvider, MIX_INTERFACE_NAME, (int)(sizeof(MIX_interface) / sizeof(MIX_interface[0])),
                           MIX_interface);
    }
    else
    {
        pUnregisterInterface(uProvider, NULL, 0, NULL);
    }

    return 1;
}
