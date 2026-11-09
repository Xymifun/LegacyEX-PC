#include "vendor/BinkADec.hpp"

#include <string.h>

#ifdef __ghs__
#include <ppc_ghs.h>
#endif

#include "vendor/BinkAudioDecode.hpp"

static const char BCFDEC_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\asi\\bcfdec\\bcfdec.cpp";

static const unsigned int ASI_ERROR_TEXT_BYTES = 256;
static const unsigned int BCFDEC_ALLOC_LINE = 0xCC;
static const unsigned int BCFDEC_SEEK_FAIL_LINE = 0xF6;
static const unsigned int BCFDEC_CLOSE_LINE = 0x11B;

static const unsigned int BCF_MAGIC = 0x42434631;
static const unsigned int BCF_MAX_VERSION = 2;
static const unsigned int BCF_VERSION_SEEK_PAIR = 2;
static const unsigned int BCF_SEEK_GRANULARITY_SHIFT = 16;
static const unsigned int BCF_SEEK_COUNT_MASK = 0xFFFF;
static const unsigned short BCF_FRAME_SYNC = 0x9999;
static const unsigned int BCF_FRAME_HEADER_BYTES = 4;
static const unsigned int BCF_FRAME_TRAILER_BYTES = 4;
static const unsigned int BCF_FRAME_OVERHEAD_BYTES = 8;
static const int BCF_FETCH_SEQUENTIAL = -1;
static const unsigned int BCF_PROVIDER_VERSION = 0x130;

static const unsigned int BCF_RATE_MEDIUM = 22050;
static const unsigned int BCF_RATE_LONG = 44100;
static const unsigned int BCF_PCM_BYTES_SHORT = 0x400;
static const unsigned int BCF_PCM_BYTES_MEDIUM = 0x800;
static const unsigned int BCF_PCM_BYTES_LONG = 0x1000;
static const unsigned int BCF_WORK_BYTES_PER_CHANNEL = 0x400;
static const unsigned int BCF_WORK_SLACK_BYTES = 0xA0;
static const unsigned int BCF_SEEK_UNIT_SHORT = 480;
static const unsigned int BCF_SEEK_UNIT_MEDIUM = 960;
static const unsigned int BCF_SEEK_UNIT_LONG = 1920;

static const unsigned int BCF_STATE_ALIGNMENT_MASK = 0x3F;
static const unsigned int BCF_SEEK_TABLE_OFFSET = 0x100;
static const unsigned int BCF_SEEK_CHUNK_ENTRIES = 0x80;
static const unsigned int BCF_SEEK_ENTRY_BYTES = 2;
static const unsigned int BCF_HEADER_BYTES = 0x18;

static const unsigned int BCF_SYNC_CANDIDATES = 8;
static const unsigned int BCF_SYNC_POSITION_SHIFT = 16;
static const unsigned int BCF_SYNC_LENGTH_MASK = 0xFFFF;
static const unsigned int BCF_PENDING_FLAG = 0x80000000;
static const unsigned int BCF_PENDING_MASK = 0x7FFFFFFF;
static const unsigned int BCF_SAMPLES_UNLIMITED = 0xFFFFFFFF;

static const unsigned int BCF_MULTICHANNEL_MINIMUM = 2;
static const unsigned int BCF_BLOCK_SAMPLES = 8;
static const unsigned int BCF_BLOCK_BYTES = 16;
static const unsigned int BCF_BLOCK_SHIFT = 4;
static const unsigned int BCF_RECIPROCAL_COUNT = 10;
static const unsigned short BCF_RECIPROCALS[BCF_RECIPROCAL_COUNT] = {0,      0,      0x8000, 0x5556, 0x4000,
                                                                     0x3334, 0x2AAB, 0x2493, 0x2000, 0};
static const unsigned int BCF_VISITED_BYTES = 256;
static const unsigned int BCF_WORK_FLOAT_COUNT = 4100;

static const unsigned int BCF_BITS_PER_SAMPLE = 16;
static const unsigned int BCF_BITRATE_SCALE = 8000;
static const unsigned int BCF_MILLISECONDS = 1000;
static const float BCF_PERCENT_SCALE = 100.0f;

static const unsigned int RIB_FUNCTION = 0;
static const unsigned int RIB_PROPERTY = 1;
static const unsigned int RIB_READ_ONLY = 0x80000000;
static const unsigned int RIB_ENTRY_CUSTOM = 1;
static const unsigned int RIB_ENTRY_DECIMAL = 2;
static const unsigned int RIB_ENTRY_HEXADECIMAL = 3;
static const unsigned int RIB_ENTRY_PERCENT = 5;
static const unsigned int RIB_ENTRY_STRING = 7;

static const unsigned int ASI_PROFILE_PROPERTY = 0x1001;
static const unsigned int ASI_SPU_FLAGS_PROPERTY = 0x40000000;
static const unsigned int ASI_INPUT_FILE_TYPES = 0;
static const unsigned int ASI_EDITOR_INPUT_FILE_TYPES = 1;
static const unsigned int ASI_OUTPUT_FILE_TYPES = 2;

static const unsigned int ASI_INPUT_BIT_RATE = 3;
static const unsigned int ASI_INPUT_SAMPLE_RATE = 4;
static const unsigned int ASI_INPUT_SAMPLE_WIDTH = 5;
static const unsigned int ASI_INPUT_CHANNELS = 6;
static const unsigned int ASI_OUTPUT_BIT_RATE = 7;
static const unsigned int ASI_OUTPUT_SAMPLE_RATE = 8;
static const unsigned int ASI_OUTPUT_SAMPLE_WIDTH = 9;
static const unsigned int ASI_OUTPUT_CHANNELS = 10;
static const unsigned int ASI_OUTPUT_RESERVOIR = 11;
static const unsigned int ASI_POSITION = 12;
static const unsigned int ASI_PERCENT_DONE = 13;
static const unsigned int ASI_STREAM_SEEK_POSITION = 14;
static const unsigned int ASI_EXACT_SEEK = 15;
static const unsigned int ASI_EXACT_GET_POSITION = 16;
static const unsigned int ASI_SEEK_TABLE_LOOKUP = 17;
static const unsigned int ASI_SET_LOOPING_SAMPLES = 18;
static const unsigned int ASI_CLEAR_LOOP_META = 19;

static const int ASI_STREAM_SEEK_RESET = -1;
static const int ASI_STREAM_SEEK_LOOP_START = -2;
static const unsigned int ASI_LOOP_UNSET = 0xFFFFFFFE;
static const unsigned int ASI_LOOP_CLEARED = 0xFFFFFFFF;
static const unsigned int ASI_CLEAR_LOOP_START = 1;
static const unsigned int ASI_CLEAR_LOOP_END = 2;

static const char ASI_CODEC_INTERFACE[] = "ASI codec";
static const char ASI_STREAM_INTERFACE[] = "ASI stream";

static char ASI_error_text[ASI_ERROR_TEXT_BYTES];

struct BcfBlock
{
    unsigned int auWords[4];
};

#ifdef __ghs__
static inline unsigned int BcfLoadLE32(const void* pSource)
{
    int iValue;
    __lwbrx(pSource, iValue);
    return (unsigned int)iValue;
}

static inline unsigned int BcfLoadLE16(const void* pSource)
{
    return *static_cast<const __bytereversed unsigned short*>(pSource);
}
#else
static inline unsigned int BcfLoadLE32(const void* pSource)
{
    const unsigned char* pBytes = (const unsigned char*)pSource;
    return (unsigned int)pBytes[0] | ((unsigned int)pBytes[1] << 8) | ((unsigned int)pBytes[2] << 16)
           | ((unsigned int)pBytes[3] << 24);
}

static inline unsigned int BcfLoadLE16(const void* pSource)
{
    const unsigned char* pBytes = (const unsigned char*)pSource;
    return (unsigned int)pBytes[0] | ((unsigned int)pBytes[1] << 8);
}
#endif

static inline unsigned int BcfSeekUnit(unsigned int uSampleRate)
{
    unsigned int uUnit = BCF_SEEK_UNIT_SHORT;

    if (uSampleRate < BCF_RATE_LONG)
    {
        if (uSampleRate >= BCF_RATE_MEDIUM)
        {
            uUnit = BCF_SEEK_UNIT_MEDIUM;
        }
    }
    else
    {
        uUnit = BCF_SEEK_UNIT_LONG;
    }

    return uUnit;
}

int PROVIDER_property(unsigned int uProperty, void* pBefore, const void* pNewValue, void* pAfter)
{
    if (pBefore != NULL && pNewValue == NULL && pAfter == NULL)
    {
        switch (uProperty)
        {
        case RIB_PROVIDER_NAME:
            *static_cast<const char**>(pBefore) = "MSS Bink Audio Decoder\0";
            return 1;
        case RIB_PROVIDER_VERSION:
            *static_cast<unsigned int*>(pBefore) = BCF_PROVIDER_VERSION;
            return 1;
        case ASI_INPUT_FILE_TYPES:
        case ASI_EDITOR_INPUT_FILE_TYPES:
            *static_cast<const char**>(pBefore) = "MSS Bink Audio Files\0*.BINKA\0";
            return 1;
        case ASI_OUTPUT_FILE_TYPES:
            *static_cast<const char**>(pBefore) = "Raw PCM files\0*.RAW\0";
            return 1;
        }
    }
    return 0;
}

const char* ASI_error()
{
    if (ASI_error_text[0] == '\0')
    {
        return NULL;
    }

    return ASI_error_text;
}

int ASI_startup()
{
    ASI_error_text[0] = '\0';
    return 0;
}

BcfStreamHandle ASI_stream_open(BCF_ALLOC_CALLBACK pAlloc, BCF_FREE_CALLBACK pFree, unsigned int uUser, BCF_FETCH_CALLBACK pFetch,
                    unsigned int uTotalSize)
{
    BcfHeader header;
    unsigned int auStreamBytes[BCF_MAX_STREAMS];
    unsigned int auStreamChannels[BCF_MAX_STREAMS];
    unsigned short ausSeekChunk[BCF_SEEK_CHUNK_ENTRIES];
    BcfStream* pStream;
    unsigned char* pBlock;
    unsigned char* pDecoder;
    unsigned int uSampleRate;
    unsigned int uSeekCount;
    unsigned int uSeekGranularity;
    unsigned int uFrameBytes;
    unsigned int uOutputCapacity;
    int iStreamCount;
    unsigned int uDecoderBytes;
    unsigned int uInputOffset;
    unsigned int uDecoderOffset;
    unsigned int uOutputOffset;
    int iStream;
    unsigned int uEntry;
    unsigned int uChunk;
    unsigned int uChunkEntries;
    unsigned int uOffset;
    unsigned int uFlags;

    if (pFetch(uUser, &header, sizeof(header), 0) != (int)sizeof(header))
    {
        return 0;
    }

    header.uMagic = BcfLoadLE32(&header.uMagic);
    if (header.uMagic != BCF_MAGIC || header.ucVersion > BCF_MAX_VERSION)
    {
        return 0;
    }

    uSampleRate = BcfLoadLE16(&header.usSampleRate);
    header.usSampleRate = (unsigned short)uSampleRate;
    header.uMaxFrameBytes = BcfLoadLE32(&header.uMaxFrameBytes);
    header.uTotalSamples = BcfLoadLE32(&header.uTotalSamples);
    header.uFileBytes = BcfLoadLE32(&header.uFileBytes);
    uSeekCount = BcfLoadLE32(&header.uSeekCount);
    uSeekGranularity = 0;
    if (header.ucVersion == BCF_VERSION_SEEK_PAIR)
    {
        uSeekGranularity = uSeekCount >> BCF_SEEK_GRANULARITY_SHIFT;
        uSeekCount = uSeekCount & BCF_SEEK_COUNT_MASK;
    }

    header.uSeekCount = uSeekCount;

    if (uSampleRate < BCF_RATE_LONG)
    {
        uFrameBytes = BCF_PCM_BYTES_SHORT;
        if (uSampleRate >= BCF_RATE_MEDIUM)
        {
            uFrameBytes = BCF_PCM_BYTES_MEDIUM;
        }

        uFrameBytes = uFrameBytes * header.ucChannels;
    }
    else
    {
        uFrameBytes = header.ucChannels * BCF_PCM_BYTES_LONG;
    }

    uOutputCapacity = uFrameBytes + (header.ucChannels * BCF_WORK_BYTES_PER_CHANNEL + BCF_WORK_SLACK_BYTES);
    iStreamCount = (header.ucChannels + 1) >> 1;
    header.uMaxFrameBytes = header.uMaxFrameBytes + BCF_FRAME_HEADER_BYTES;
    uDecoderBytes = 0;
    for (iStream = 0; iStream < iStreamCount; ++iStream)
    {
        auStreamChannels[iStream] = (header.ucChannels - iStream * 2 >= 2) ? 2 : 1;
        auStreamBytes[iStream] = BinkAudioDecompressMemoryLE(header.usSampleRate, auStreamChannels[iStream], 1);
        uDecoderBytes = uDecoderBytes + auStreamBytes[iStream];
    }

    uInputOffset = (uSeekCount * 4 + BCF_SEEK_TABLE_OFFSET + 4 + BCF_STATE_ALIGNMENT_MASK) & ~BCF_STATE_ALIGNMENT_MASK;
    uDecoderOffset = (uInputOffset + header.uMaxFrameBytes * 2 + BCF_STATE_ALIGNMENT_MASK) & ~BCF_STATE_ALIGNMENT_MASK;
    uOutputOffset = (uDecoderOffset + uDecoderBytes + BCF_STATE_ALIGNMENT_MASK) & ~BCF_STATE_ALIGNMENT_MASK;
    pBlock = static_cast<unsigned char*>(
        pAlloc((uOutputOffset + uOutputCapacity + BCF_STATE_ALIGNMENT_MASK) & ~BCF_STATE_ALIGNMENT_MASK, uUser,
               BCFDEC_FILE, BCFDEC_ALLOC_LINE));

    if (pBlock == NULL)
    {
        strcpy(ASI_error_text, "Out of memory");
        return 0;
    }

    pStream = reinterpret_cast<BcfStream*>(pBlock);
    memset(pStream, 0, sizeof(BcfStream));
    pStream->pSeekOffsets = reinterpret_cast<unsigned int*>(pBlock + BCF_SEEK_TABLE_OFFSET);
    pStream->pInput = pBlock + uInputOffset;
    pStream->pOutput = pBlock + uOutputOffset;
    pStream->uOutputCapacity = uOutputCapacity;
    pStream->pFetch = pFetch;
    pStream->uInputCapacity = header.uMaxFrameBytes << 1;
    pStream->uUser = uUser;
    pStream->uFrameBytes = uFrameBytes;
    pStream->pFree = pFree;
    pStream->uFileBytes = header.uFileBytes;
    pStream->header = header;
    pStream->uSeekGranularity = uSeekGranularity;
    pStream->uDataOffset = uSeekCount * BCF_SEEK_ENTRY_BYTES + BCF_HEADER_BYTES;

    uOffset = 0;
    for (uChunk = 0; uChunk < uSeekCount; uChunk = uChunk + uChunkEntries)
    {
        uChunkEntries = uSeekCount - uChunk;
        if (uChunkEntries > BCF_SEEK_CHUNK_ENTRIES)
        {
            uChunkEntries = BCF_SEEK_CHUNK_ENTRIES;
        }

        if (pStream->pFetch(uUser, ausSeekChunk, uChunkEntries * BCF_SEEK_ENTRY_BYTES, BCF_FETCH_SEQUENTIAL)
            != uChunkEntries * BCF_SEEK_ENTRY_BYTES)
        {
            pFree(pStream, pStream->uUser, BCFDEC_FILE, BCFDEC_SEEK_FAIL_LINE);
            return 0;
        }

        for (uEntry = 0; uEntry < uChunkEntries; ++uEntry)
        {
            pStream->pSeekOffsets[uChunk + uEntry] = uOffset;
            uOffset = uOffset + BcfLoadLE16(&ausSeekChunk[uEntry]);
        }
    }

    pStream->pSeekOffsets[uSeekCount] = uOffset;
    pStream->uDurationMs = (unsigned int)(((unsigned long long)pStream->header.uTotalSamples * BCF_MILLISECONDS)
                                          / pStream->header.usSampleRate);
    pStream->uStreamCount = iStreamCount;
    pStream->uSamplesRemaining = BCF_SAMPLES_UNLIMITED;
    pStream->uStreamPosition = pStream->uDataOffset;

    pDecoder = pBlock + uDecoderOffset;
    for (iStream = 0; iStream < iStreamCount; ++iStream)
    {
        pStream->auStreamChannels[iStream] = auStreamChannels[iStream];
        uFlags = 0;
        if (header.ucChannels > 2)
        {
            uFlags = BINKAUDIO_FLAG_PLANAR;
        }

        pStream->apDecoders[iStream] = BinkAudioDecompressOpenLE(
            pDecoder, header.usSampleRate, auStreamChannels[iStream], uFlags | BINKAUDIO_FLAG_DCT);
        pDecoder = pDecoder + auStreamBytes[iStream];
        pStream->auStreamBytes[iStream] = auStreamBytes[iStream];
    }

    return static_cast<BcfStreamHandle>(reinterpret_cast<intptr_t>(pStream));
}

int ASI_stream_close(BcfStreamHandle iStream)
{
    BcfStream* pStream = reinterpret_cast<BcfStream*>(iStream);

    pStream->pFree(pStream, pStream->uUser, BCFDEC_FILE, BCFDEC_CLOSE_LINE);
    return 0;
}

static void BcfTransposeBlock(unsigned short* pSamples, unsigned int uChannels)
{
    unsigned short uTemp;

    switch (uChannels)
    {
    case 2:
        uTemp = pSamples[1];
        pSamples[1] = pSamples[8];
        pSamples[8] = pSamples[4];
        pSamples[4] = pSamples[2];
        pSamples[2] = uTemp;
        uTemp = pSamples[3];
        pSamples[3] = pSamples[9];
        pSamples[9] = pSamples[12];
        pSamples[12] = pSamples[6];
        pSamples[6] = uTemp;
        uTemp = pSamples[5];
        pSamples[5] = pSamples[10];
        pSamples[10] = uTemp;
        uTemp = pSamples[7];
        pSamples[7] = pSamples[11];
        pSamples[11] = pSamples[13];
        pSamples[13] = pSamples[14];
        pSamples[14] = uTemp;
        break;

    case 3:
        uTemp = pSamples[1];
        pSamples[1] = pSamples[8];
        pSamples[8] = pSamples[18];
        pSamples[18] = pSamples[6];
        pSamples[6] = pSamples[2];
        pSamples[2] = pSamples[16];
        pSamples[16] = pSamples[13];
        pSamples[13] = pSamples[12];
        pSamples[12] = pSamples[4];
        pSamples[4] = pSamples[9];
        pSamples[9] = pSamples[3];
        pSamples[3] = uTemp;
        uTemp = pSamples[5];
        pSamples[5] = pSamples[17];
        pSamples[17] = pSamples[21];
        pSamples[21] = pSamples[7];
        pSamples[7] = pSamples[10];
        pSamples[10] = pSamples[11];
        pSamples[11] = pSamples[19];
        pSamples[19] = pSamples[14];
        pSamples[14] = pSamples[20];
        pSamples[20] = pSamples[22];
        pSamples[22] = pSamples[15];
        pSamples[15] = uTemp;
        break;

    case 4:
        uTemp = pSamples[1];
        pSamples[1] = pSamples[8];
        pSamples[8] = pSamples[2];
        pSamples[2] = pSamples[16];
        pSamples[16] = pSamples[4];
        pSamples[4] = uTemp;
        uTemp = pSamples[3];
        pSamples[3] = pSamples[24];
        pSamples[24] = pSamples[6];
        pSamples[6] = pSamples[17];
        pSamples[17] = pSamples[12];
        pSamples[12] = uTemp;
        uTemp = pSamples[5];
        pSamples[5] = pSamples[9];
        pSamples[9] = pSamples[10];
        pSamples[10] = pSamples[18];
        pSamples[18] = pSamples[20];
        pSamples[20] = uTemp;
        uTemp = pSamples[7];
        pSamples[7] = pSamples[25];
        pSamples[25] = pSamples[14];
        pSamples[14] = pSamples[19];
        pSamples[19] = pSamples[28];
        pSamples[28] = uTemp;
        uTemp = pSamples[11];
        pSamples[11] = pSamples[26];
        pSamples[26] = pSamples[22];
        pSamples[22] = pSamples[21];
        pSamples[21] = pSamples[13];
        pSamples[13] = uTemp;
        uTemp = pSamples[15];
        pSamples[15] = pSamples[27];
        pSamples[27] = pSamples[30];
        pSamples[30] = pSamples[23];
        pSamples[23] = pSamples[29];
        pSamples[29] = uTemp;
        break;

    case 5:
        uTemp = pSamples[1];
        pSamples[1] = pSamples[8];
        pSamples[8] = pSamples[25];
        pSamples[25] = pSamples[5];
        pSamples[5] = uTemp;
        uTemp = pSamples[2];
        pSamples[2] = pSamples[16];
        pSamples[16] = pSamples[11];
        pSamples[11] = pSamples[10];
        pSamples[10] = uTemp;
        uTemp = pSamples[3];
        pSamples[3] = pSamples[24];
        pSamples[24] = pSamples[36];
        pSamples[36] = pSamples[15];
        pSamples[15] = uTemp;
        uTemp = pSamples[4];
        pSamples[4] = pSamples[32];
        pSamples[32] = pSamples[22];
        pSamples[22] = pSamples[20];
        pSamples[20] = uTemp;
        uTemp = pSamples[6];
        pSamples[6] = pSamples[9];
        pSamples[9] = pSamples[33];
        pSamples[33] = pSamples[30];
        pSamples[30] = uTemp;
        uTemp = pSamples[7];
        pSamples[7] = pSamples[17];
        pSamples[17] = pSamples[19];
        pSamples[19] = pSamples[35];
        pSamples[35] = uTemp;
        uTemp = pSamples[12];
        pSamples[12] = pSamples[18];
        pSamples[18] = pSamples[27];
        pSamples[27] = pSamples[21];
        pSamples[21] = uTemp;
        uTemp = pSamples[13];
        pSamples[13] = pSamples[26];
        pSamples[26] = uTemp;
        uTemp = pSamples[14];
        pSamples[14] = pSamples[34];
        pSamples[34] = pSamples[38];
        pSamples[38] = pSamples[31];
        pSamples[31] = uTemp;
        uTemp = pSamples[23];
        pSamples[23] = pSamples[28];
        pSamples[28] = pSamples[29];
        pSamples[29] = pSamples[37];
        pSamples[37] = uTemp;
        break;

    case 6:
        uTemp = pSamples[1];
        pSamples[1] = pSamples[8];
        pSamples[8] = pSamples[17];
        pSamples[17] = pSamples[42];
        pSamples[42] = pSamples[7];
        pSamples[7] = pSamples[9];
        pSamples[9] = pSamples[25];
        pSamples[25] = pSamples[12];
        pSamples[12] = pSamples[2];
        pSamples[2] = pSamples[16];
        pSamples[16] = pSamples[34];
        pSamples[34] = pSamples[37];
        pSamples[37] = pSamples[14];
        pSamples[14] = pSamples[18];
        pSamples[18] = pSamples[3];
        pSamples[3] = pSamples[24];
        pSamples[24] = pSamples[4];
        pSamples[4] = pSamples[32];
        pSamples[32] = pSamples[21];
        pSamples[21] = pSamples[27];
        pSamples[27] = pSamples[28];
        pSamples[28] = pSamples[36];
        pSamples[36] = pSamples[6];
        pSamples[6] = uTemp;
        uTemp = pSamples[5];
        pSamples[5] = pSamples[40];
        pSamples[40] = pSamples[38];
        pSamples[38] = pSamples[22];
        pSamples[22] = pSamples[35];
        pSamples[35] = pSamples[45];
        pSamples[45] = pSamples[31];
        pSamples[31] = pSamples[13];
        pSamples[13] = pSamples[10];
        pSamples[10] = pSamples[33];
        pSamples[33] = pSamples[29];
        pSamples[29] = pSamples[44];
        pSamples[44] = pSamples[23];
        pSamples[23] = pSamples[43];
        pSamples[43] = pSamples[15];
        pSamples[15] = pSamples[26];
        pSamples[26] = pSamples[20];
        pSamples[20] = pSamples[19];
        pSamples[19] = pSamples[11];
        pSamples[11] = pSamples[41];
        pSamples[41] = pSamples[46];
        pSamples[46] = pSamples[39];
        pSamples[39] = pSamples[30];
        pSamples[30] = uTemp;
        break;

    case 7:
        uTemp = pSamples[1];
        pSamples[1] = pSamples[8];
        pSamples[8] = pSamples[9];
        pSamples[9] = pSamples[17];
        pSamples[17] = pSamples[26];
        pSamples[26] = pSamples[43];
        pSamples[43] = pSamples[14];
        pSamples[14] = pSamples[2];
        pSamples[2] = pSamples[16];
        pSamples[16] = pSamples[18];
        pSamples[18] = pSamples[34];
        pSamples[34] = pSamples[52];
        pSamples[52] = pSamples[31];
        pSamples[31] = pSamples[28];
        pSamples[28] = pSamples[4];
        pSamples[4] = pSamples[32];
        pSamples[32] = pSamples[36];
        pSamples[36] = pSamples[13];
        pSamples[13] = pSamples[49];
        pSamples[49] = pSamples[7];
        pSamples[7] = uTemp;
        uTemp = pSamples[3];
        pSamples[3] = pSamples[24];
        pSamples[24] = pSamples[27];
        pSamples[27] = pSamples[51];
        pSamples[51] = pSamples[23];
        pSamples[23] = pSamples[19];
        pSamples[19] = pSamples[42];
        pSamples[42] = pSamples[6];
        pSamples[6] = pSamples[48];
        pSamples[48] = pSamples[54];
        pSamples[54] = pSamples[47];
        pSamples[47] = pSamples[46];
        pSamples[46] = pSamples[38];
        pSamples[38] = pSamples[29];
        pSamples[29] = pSamples[12];
        pSamples[12] = pSamples[41];
        pSamples[41] = pSamples[53];
        pSamples[53] = pSamples[39];
        pSamples[39] = pSamples[37];
        pSamples[37] = pSamples[21];
        pSamples[21] = uTemp;
        uTemp = pSamples[5];
        pSamples[5] = pSamples[40];
        pSamples[40] = pSamples[45];
        pSamples[45] = pSamples[30];
        pSamples[30] = pSamples[20];
        pSamples[20] = pSamples[50];
        pSamples[50] = pSamples[15];
        pSamples[15] = pSamples[10];
        pSamples[10] = pSamples[25];
        pSamples[25] = pSamples[35];
        pSamples[35] = uTemp;
        uTemp = pSamples[11];
        pSamples[11] = pSamples[33];
        pSamples[33] = pSamples[44];
        pSamples[44] = pSamples[22];
        pSamples[22] = uTemp;
        break;

    case 8:
        uTemp = pSamples[1];
        pSamples[1] = pSamples[8];
        pSamples[8] = uTemp;
        uTemp = pSamples[2];
        pSamples[2] = pSamples[16];
        pSamples[16] = uTemp;
        uTemp = pSamples[3];
        pSamples[3] = pSamples[24];
        pSamples[24] = uTemp;
        uTemp = pSamples[4];
        pSamples[4] = pSamples[32];
        pSamples[32] = uTemp;
        uTemp = pSamples[5];
        pSamples[5] = pSamples[40];
        pSamples[40] = uTemp;
        uTemp = pSamples[6];
        pSamples[6] = pSamples[48];
        pSamples[48] = uTemp;
        uTemp = pSamples[7];
        pSamples[7] = pSamples[56];
        pSamples[56] = uTemp;
        uTemp = pSamples[10];
        pSamples[10] = pSamples[17];
        pSamples[17] = uTemp;
        uTemp = pSamples[11];
        pSamples[11] = pSamples[25];
        pSamples[25] = uTemp;
        uTemp = pSamples[12];
        pSamples[12] = pSamples[33];
        pSamples[33] = uTemp;
        uTemp = pSamples[13];
        pSamples[13] = pSamples[41];
        pSamples[41] = uTemp;
        uTemp = pSamples[14];
        pSamples[14] = pSamples[49];
        pSamples[49] = uTemp;
        uTemp = pSamples[15];
        pSamples[15] = pSamples[57];
        pSamples[57] = uTemp;
        uTemp = pSamples[19];
        pSamples[19] = pSamples[26];
        pSamples[26] = uTemp;
        uTemp = pSamples[20];
        pSamples[20] = pSamples[34];
        pSamples[34] = uTemp;
        uTemp = pSamples[21];
        pSamples[21] = pSamples[42];
        pSamples[42] = uTemp;
        uTemp = pSamples[22];
        pSamples[22] = pSamples[50];
        pSamples[50] = uTemp;
        uTemp = pSamples[23];
        pSamples[23] = pSamples[58];
        pSamples[58] = uTemp;
        uTemp = pSamples[28];
        pSamples[28] = pSamples[35];
        pSamples[35] = uTemp;
        uTemp = pSamples[29];
        pSamples[29] = pSamples[43];
        pSamples[43] = uTemp;
        uTemp = pSamples[30];
        pSamples[30] = pSamples[51];
        pSamples[51] = uTemp;
        uTemp = pSamples[31];
        pSamples[31] = pSamples[59];
        pSamples[59] = uTemp;
        uTemp = pSamples[37];
        pSamples[37] = pSamples[44];
        pSamples[44] = uTemp;
        uTemp = pSamples[38];
        pSamples[38] = pSamples[52];
        pSamples[52] = uTemp;
        uTemp = pSamples[39];
        pSamples[39] = pSamples[60];
        pSamples[60] = uTemp;
        uTemp = pSamples[46];
        pSamples[46] = pSamples[53];
        pSamples[53] = uTemp;
        uTemp = pSamples[47];
        pSamples[47] = pSamples[61];
        pSamples[61] = uTemp;
        uTemp = pSamples[55];
        pSamples[55] = pSamples[62];
        pSamples[62] = uTemp;
        break;

    default:
        break;
    }
}

void decode_bink_buffer(BcfStream* pStream)
{
    unsigned char abVisited[BCF_VISITED_BYTES];
    float afWork[BCF_WORK_FLOAT_COUNT];
    unsigned char aucBits[8] = {1, 2, 4, 8, 16, 32, 64, 128};
    unsigned char* pOutput = pStream->pOutput + pStream->uOutputBytes;
    BcfBlock* pBlocks = reinterpret_cast<BcfBlock*>(pOutput);
    unsigned char* pStreamOutput = pOutput;
    const unsigned char* pFrame = pStream->pInput + pStream->uInputPosition;
    const void* pPayload = pFrame + BCF_FRAME_HEADER_BYTES;
    const unsigned char* pPayloadEnd = pFrame + BCF_FRAME_HEADER_BYTES + BcfLoadLE16(pFrame + 2);
    unsigned int uSkipSamples = pStream->uSkipSamples;
    unsigned int uDecoded = 0;
    unsigned int uChannelBytes = 0;
    unsigned int uStreamBytes;
    unsigned int uStream;
    unsigned int uSamples;
    unsigned int uChannels;
    unsigned int uChannel;
    unsigned int uSample;
    unsigned int uBlocks;
    unsigned int uGroup;
    unsigned int uCounter;
    unsigned int uIndex;
    unsigned int uPrevious;
    unsigned int uNext;
    unsigned int uQuotient;
    unsigned int uConsumed;
    unsigned int uPending;
    const unsigned short* pSourceSamples;
    unsigned short* pDestinationSamples;
    BcfBlock saved;

    if (pStream->uStreamCount > 0)
    {
        for (uStream = 0; uStream < pStream->uStreamCount; ++uStream)
        {
            pStream->uSkipSamples = uSkipSamples;
            uStreamBytes = 0;
            BinkAudioDecompressLE(pStream->apDecoders[uStream], pStreamOutput, &uStreamBytes, pPayload, &pPayload,
                                  pPayloadEnd, afWork, reinterpret_cast<int*>(&pStream->uSkipSamples));
            if (static_cast<const unsigned char*>(pPayload) > pPayloadEnd)
            {
                pStream->iNeedSync = 1;
                pStream->uInputPosition = (unsigned int)(pPayloadEnd - pStream->pInput);
                if (pStream->uInputPosition > pStream->uInputFill)
                {
                    pStream->uInputPosition = pStream->uInputFill;
                }

                return;
            }

            if (pStream->uLoopEndOffset == 0
                || (pStream->uStreamPosition + pStream->uInputPosition - pStream->uInputFill) < pStream->uLoopEndOffset)
            {
                uSamples = pStream->uSamplesRemaining;
            }
            else
            {
                uSamples = pStream->uSamplesRemaining;
                if (uSamples == BCF_SAMPLES_UNLIMITED)
                {
                    uSamples = pStream->uLoopEndRemainder;
                    pStream->uSamplesRemaining = uSamples;
                }
            }

            uChannels = pStream->auStreamChannels[uStream];
            if (uSamples != BCF_SAMPLES_UNLIMITED)
            {
                uSample = uStreamBytes / (uChannels << 1);
                if (uSamples < uSample)
                {
                    uStreamBytes = uStreamBytes - uChannels * (uSample - uSamples) * 2;
                    uSample = uSamples;
                }

                if (uStream == pStream->uStreamCount - 1)
                {
                    pStream->uSamplesRemaining = uSamples - uSample;
                    uChannels = pStream->auStreamChannels[uStream];
                }
            }

            uChannelBytes = uStreamBytes;
            if (uChannels > 1)
            {
                uChannelBytes = uStreamBytes >> 1;
            }

            uDecoded = uDecoded + uStreamBytes;
            pStreamOutput = pStreamOutput + uStreamBytes;
        }

        uChannels = pStream->header.ucChannels;
        if (uChannelBytes != 0 && uChannels > BCF_MULTICHANNEL_MINIMUM)
        {
            if (uChannelBytes < BCF_BLOCK_BYTES)
            {
                memcpy(abVisited, pOutput, uChannels * uChannelBytes);
                uSamples = uChannelBytes >> 1;
                for (uChannel = 0; uChannel < uChannels; ++uChannel)
                {
                    pSourceSamples = reinterpret_cast<const unsigned short*>(abVisited
                                                                             + uChannel * (uChannelBytes & ~1u));
                    pDestinationSamples = reinterpret_cast<unsigned short*>(pOutput) + uChannel;
                    for (uSample = 0; uSample < uSamples; ++uSample)
                    {
                        pDestinationSamples[uSample * uChannels] = pSourceSamples[uSample];
                    }
                }
            }
            else
            {
                uBlocks = uChannelBytes >> BCF_BLOCK_SHIFT;
                memset(abVisited, 0, uBlocks * uChannels >> 3);
                uIndex = 0;
                for (uGroup = 0; uGroup < uBlocks; ++uGroup)
                {
                    for (uCounter = 0; uCounter < uChannels; ++uCounter)
                    {
                        if ((abVisited[uIndex >> 3] & aucBits[uIndex & 7]) == 0)
                        {
                            saved = pBlocks[uIndex];
                            uQuotient = (uIndex * BCF_RECIPROCALS[uChannels]) >> 16;
                            uNext = (uIndex - uQuotient * uChannels) * uBlocks + uQuotient;
                            uPrevious = uIndex;
                            while (uNext != uIndex)
                            {
                                pBlocks[uPrevious] = pBlocks[uNext];
                                abVisited[uPrevious >> 3] = abVisited[uPrevious >> 3] | aucBits[uPrevious & 7];
                                uPrevious = uNext;
                                uQuotient = (uNext * BCF_RECIPROCALS[uChannels]) >> 16;
                                uNext = (uNext - uQuotient * uChannels) * uBlocks + uQuotient;
                            }

                            pBlocks[uPrevious] = saved;
                            abVisited[uPrevious >> 3] = abVisited[uPrevious >> 3] | aucBits[uPrevious & 7];
                        }

                        ++uIndex;
                    }

                    BcfTransposeBlock(reinterpret_cast<unsigned short*>(pBlocks + uGroup * uChannels), uChannels);
                }
            }
        }
    }

    uConsumed = (unsigned int)(static_cast<const unsigned char*>(pPayload) - pStream->pInput);
    pStream->uOutputBytes = pStream->uOutputBytes + uDecoded;
    pStream->uInputPosition = uConsumed;
    if (uConsumed == pStream->uInputFill)
    {
        if ((pStream->uPendingSeek & BCF_PENDING_FLAG) != 0)
        {
            pStream->uStreamPosition = pStream->uPendingSeek & BCF_PENDING_MASK;
            pStream->iSeekPending = 1;
        }

        uPending = pStream->uPendingSkip;
        if ((uPending & BCF_PENDING_FLAG) != 0)
        {
            pStream->uSkipSamples = uPending & BCF_PENDING_MASK;
        }

        if (uPending != 0 || pStream->uPendingSeek != 0)
        {
            pStream->uSamplesRemaining = BCF_SAMPLES_UNLIMITED;
        }

        pStream->uPendingSeek = 0;
        pStream->uPendingSkip = 0;
    }
}

void FindFrameBoundary(BcfStreamHandle iStream)
{
    BcfStream* pStream = reinterpret_cast<BcfStream*>(iStream);
    unsigned int auCandidates[BCF_SYNC_CANDIDATES];
    unsigned char* pBuffer;
    unsigned char* pFrame;
    unsigned int uFill;
    unsigned int uWindow;
    unsigned int uCount;
    unsigned int uIndex;
    unsigned int uPayload;
    unsigned int uHeader;
    unsigned int uLength;
    unsigned int uCandidate;
    unsigned int uNext;
    int iRead;

    if (pStream->uInputPosition != 0)
    {
        memcpy(pStream->pInput, pStream->pInput + pStream->uInputPosition,
               pStream->uInputFill - pStream->uInputPosition);
        pStream->uInputFill -= pStream->uInputPosition;
        pStream->uInputPosition = 0;
    }

    for (uIndex = 0; uIndex < BCF_SYNC_CANDIDATES; ++uIndex)
    {
        auCandidates[uIndex] = 0;
    }

    pBuffer = pStream->pInput;
    uFill = pStream->uInputFill;

    for (;;)
    {
        uIndex = 0;
        uWindow = pStream->header.uMaxFrameBytes;
        uCount = 0;
        for (uPayload = BCF_FRAME_HEADER_BYTES; uPayload < uFill; ++uPayload)
        {
            pFrame = pBuffer + uIndex;
            uHeader = BcfLoadLE32(pFrame);
            if ((uHeader & BCF_SYNC_LENGTH_MASK) == BCF_FRAME_SYNC)
            {
                uLength = uHeader >> BCF_SYNC_POSITION_SHIFT;
                if (uLength <= uWindow)
                {
                    if (uIndex + uLength + BCF_FRAME_OVERHEAD_BYTES <= uFill)
                    {
                        if ((BcfLoadLE32(pFrame + BCF_FRAME_HEADER_BYTES + uLength) & BCF_SYNC_LENGTH_MASK)
                            == BCF_FRAME_SYNC)
                        {
                            memmove(pBuffer, pFrame, uFill - uIndex);
                            pStream->iNeedSync = 0;
                            pStream->uInputFill -= uIndex;
                            return;
                        }
                    }
                    else if (uCount < BCF_SYNC_CANDIDATES)
                    {
                        auCandidates[uCount] = uLength | (uPayload << BCF_SYNC_POSITION_SHIFT);
                        uFill = pStream->uInputFill;
                        pBuffer = pStream->pInput;
                        ++uCount;
                        uWindow = pStream->header.uMaxFrameBytes;
                    }
                }
            }

            ++uIndex;
        }

        uWindow += BCF_FRAME_TRAILER_BYTES;
        if (uWindow < uFill)
        {
            memmove(pBuffer, pBuffer + uWindow, uFill - uWindow);
            pStream->uInputFill -= uWindow;
            iRead = pStream->pFetch(pStream->uUser, pStream->pInput + pStream->uInputFill,
                                    pStream->uInputCapacity - pStream->uInputFill, BCF_FETCH_SEQUENTIAL);
        }
        else
        {
            uWindow = 0;
            iRead = pStream->pFetch(pStream->uUser, pBuffer + uFill, pStream->uInputCapacity - uFill,
                                    BCF_FETCH_SEQUENTIAL);
        }

        if (iRead == 0)
        {
            pStream->iNeedSync = 0;
            pStream->uInputFill = 0;
            return;
        }

        uFill = pStream->uInputFill + iRead;
        pBuffer = pStream->pInput;
        pStream->uInputFill = uFill;

        for (uIndex = 0; uIndex < uCount; ++uIndex)
        {
            uCandidate = auCandidates[uIndex];
            uPayload = uCandidate >> BCF_SYNC_POSITION_SHIFT;
            if (uWindow <= uPayload)
            {
                uNext = ((uCandidate & BCF_SYNC_LENGTH_MASK) + uPayload) - uWindow;
                if (uNext + BCF_FRAME_HEADER_BYTES < uFill
                    && (BcfLoadLE32(pBuffer + uNext) & BCF_SYNC_LENGTH_MASK) == BCF_FRAME_SYNC)
                {
                    memmove(pBuffer, pBuffer + (uPayload - BCF_FRAME_HEADER_BYTES), uFill - uPayload);
                    pStream->iNeedSync = 0;
                    pStream->uInputFill -= uPayload;
                    return;
                }
            }
        }
    }
}

void SeekStreamForward(BcfStream* pStream, int iBytes)
{
    unsigned int uRemaining = iBytes;

    while (uRemaining != 0)
    {
        unsigned int uChunk = uRemaining;
        if (uRemaining > pStream->uInputCapacity)
        {
            uChunk = pStream->uInputCapacity;
        }

        pStream->pFetch(pStream->uUser, pStream->pInput, uChunk, BCF_FETCH_SEQUENTIAL);
        pStream->uStreamPosition += uChunk;
        uRemaining -= uChunk;
    }
}

static void RestartFrameSearch(BcfStream* pStream)
{
    pStream->uInputPosition = 0;
    pStream->iResyncPending = 0;
    pStream->uInputFill = 0;
    FindFrameBoundary(static_cast<BcfStreamHandle>(reinterpret_cast<intptr_t>(pStream)));
}

int ASI_stream_process(BcfStreamHandle iStream, void* pBuffer, int iBytes)
{
    BcfStream* pStream = reinterpret_cast<BcfStream*>(iStream);
    unsigned char* pDestination = static_cast<unsigned char*>(pBuffer);
    const unsigned char* pFrame;
    const unsigned char* pFrameEnd;
    const unsigned int* pEntry;
    const unsigned int* pEnd;
    unsigned int uFill;
    unsigned int uPosition;
    unsigned int uNeeded;
    unsigned int uAvailable;
    unsigned int uRoom;
    unsigned int uLimit;
    unsigned int uOffset;
    unsigned int uCopy;
    unsigned int uFrameSize;
    unsigned int uLength;
    unsigned int uMoved;
    int iCopied = 0;
    int iCapacity;
    int iRead;
    int bResync;

    if (pStream->iNeedSync == 0)
    {
        if (pStream->iSeekPending != 0)
        {
            if (pStream->uDataOffset < pStream->uStreamPosition)
            {
                pEntry = pStream->pSeekOffsets;
                pEnd = pEntry + pStream->header.uSeekCount;
                uOffset = pStream->uStreamPosition - pStream->uDataOffset;
                for (; pEnd != pEntry; ++pEntry)
                {
                    if (*pEntry <= uOffset)
                    {
                        if (uOffset <= *pEntry)
                        {
                            pStream->iSeekPending = 0;
                            break;
                        }

                        if (pEntry + 1 == pEnd)
                        {
                            SeekStreamForward(pStream, pStream->header.uFileBytes - pStream->uStreamPosition);
                            pStream->iSeekPending = 0;
                            break;
                        }

                        if (uOffset < pEntry[1])
                        {
                            SeekStreamForward(pStream, pEntry[1] - uOffset);
                            pStream->iSeekPending = 0;
                            break;
                        }
                    }
                }
            }
            else
            {
                SeekStreamForward(pStream, pStream->uDataOffset - pStream->uStreamPosition);
                pStream->iSeekPending = 0;
            }
        }
    }
    else
    {
        FindFrameBoundary(iStream);
    }

    for (;;)
    {
        if (iBytes == 0)
        {
            return iCopied;
        }

        if (pStream->uOutputBytes != 0)
        {
            uAvailable = pStream->uOutputBytes - pStream->uOutputPosition;
            uCopy = iBytes;
            if (uAvailable < (unsigned int)iBytes)
            {
                uCopy = uAvailable;
            }

            memcpy(pDestination, pStream->pOutput + pStream->uOutputPosition, uCopy);
            iCopied = iCopied + uCopy;
            if (uCopy == (unsigned int)iBytes)
            {
                pStream->uOutputPosition = pStream->uOutputPosition + uCopy;
                return iCopied;
            }

            pStream->uOutputPosition = 0;
            pDestination = pDestination + uCopy;
            iBytes = iBytes - uCopy;
            pStream->uOutputBytes = 0;
        }

        uFill = pStream->uInputFill;
        uPosition = pStream->uInputPosition;
        uNeeded = pStream->header.uMaxFrameBytes;
        if ((int)(uFill - uPosition) > (int)(BCF_FRAME_HEADER_BYTES - 1))
        {
            pFrame = pStream->pInput + uPosition;
            if (BcfLoadLE16(pFrame) == BCF_FRAME_SYNC)
            {
                uFrameSize = BcfLoadLE16(pFrame + 2) + BCF_FRAME_OVERHEAD_BYTES;
                if (uFrameSize < uNeeded && uFrameSize != 0)
                {
                    uNeeded = uFrameSize;
                }
            }
        }

        if ((int)(uFill - uPosition) < (int)uNeeded)
        {
            iCapacity = pStream->uInputCapacity;
            if (uFill < uPosition)
            {
                pStream->uInputPosition = uFill;
                uPosition = uFill;
                uFill = pStream->uInputFill;
            }

            if (iCapacity - uPosition < uNeeded && uPosition != 0)
            {
                memcpy(pStream->pInput, pStream->pInput + uPosition, uFill - uPosition);
                uMoved = pStream->uInputPosition;
                uPosition = 0;
                pStream->uInputPosition = 0;
                uFill = pStream->uInputFill - uMoved;
                iCapacity = pStream->uInputCapacity;
                pStream->uInputFill = uFill;
            }

            uRoom = iCapacity - uFill;
            if (pStream->iResyncPending != 0)
            {
                uRoom = 0;
            }

            if ((pStream->uPendingSeek != 0 || pStream->uPendingSkip != 0)
                && (uLimit = ((uNeeded + uPosition) - uFill) - BCF_FRAME_TRAILER_BYTES, uLimit < uRoom))
            {
                uRoom = uLimit;
            }

            if (uRoom != 0)
            {
                iRead = pStream->pFetch(pStream->uUser, pStream->pInput + uFill, uRoom, BCF_FETCH_SEQUENTIAL);
                uPosition = pStream->uInputPosition;
                uFill = pStream->uInputFill + iRead;
                pStream->uInputFill = uFill;
                pStream->uStreamPosition = pStream->uStreamPosition + iRead;
            }

            if ((uRoom != 0 && (int)(uNeeded - BCF_FRAME_TRAILER_BYTES) > (int)(uFill - uPosition))
                || uFill == uPosition)
            {
                if (pStream->iResyncPending == 0)
                {
                    return iCopied;
                }

                RestartFrameSearch(pStream);
                continue;
            }
        }

        bResync = 0;
        if (uFill == uPosition)
        {
            bResync = pStream->iResyncPending != 0;
        }
        else
        {
            pFrame = pStream->pInput + uPosition;
            if (BcfLoadLE16(pFrame) != BCF_FRAME_SYNC)
            {
                pStream->iNeedSync = 1;
                bResync = pStream->iResyncPending != 0;
            }
            else
            {
                uLength = BcfLoadLE16(pFrame + 2);
                pFrameEnd = pFrame + BCF_FRAME_HEADER_BYTES + uLength;
                if (pStream->pInput + uFill < pFrameEnd)
                {
                    bResync = pStream->iResyncPending != 0;
                }
                else if (pFrameEnd < pStream->pInput + uFill && BcfLoadLE16(pFrameEnd) != BCF_FRAME_SYNC)
                {
                    pStream->iNeedSync = 1;
                    bResync = pStream->iResyncPending != 0;
                }
                else if (uLength + BCF_FRAME_HEADER_BYTES == 0)
                {
                    bResync = pStream->iResyncPending != 0;
                }
                else
                {
                    decode_bink_buffer(pStream);
                }
            }
        }

        if (bResync)
        {
            RestartFrameSearch(pStream);
            continue;
        }

        if (pStream->iNeedSync != 0)
        {
            return iCopied;
        }

        if (pStream->uSamplesRemaining == 0)
        {
            return iCopied;
        }
    }
}

int ASI_stream_seek(BcfStreamHandle iStream, int iPosition)
{
    BcfStream* pStream = reinterpret_cast<BcfStream*>(iStream);

    if (iPosition == ASI_STREAM_SEEK_RESET)
    {
        pStream->uOutputPosition = 0;
        pStream->uOutputBytes = 0;
        pStream->uInputPosition = 0;
        pStream->iSeekPending = 1;
        pStream->uSamplesRemaining = BCF_SAMPLES_UNLIMITED;
        pStream->uInputFill = 0;
    }
    else if (iPosition == ASI_STREAM_SEEK_LOOP_START)
    {
        if (pStream->uLoopStartRemainder != 0)
        {
            if (pStream->uInputPosition == pStream->uInputFill)
            {
                pStream->uSkipSamples = pStream->uLoopStartRemainder;
            }
            else
            {
                pStream->uPendingSkip = pStream->uLoopStartRemainder | BCF_PENDING_FLAG;
            }
        }
    }
    else
    {
        pStream->uStreamPosition = iPosition;
        pStream->uInputPosition = 0;
        pStream->uSamplesRemaining = BCF_SAMPLES_UNLIMITED;
        pStream->uOutputBytes = 0;
        pStream->uOutputPosition = 0;
        pStream->uInputFill = 0;
    }

    return 0;
}

void SeekTableLookup(BcfStream* pStream, unsigned int uSample, int* piOffset, int* piRemainder, int* piBase)
{
    unsigned int uFrame = BCF_SEEK_UNIT_SHORT;
    unsigned int uEntry;
    unsigned int uUnit;

    if (pStream->header.usSampleRate >= BCF_RATE_LONG)
    {
        uFrame = BCF_SEEK_UNIT_LONG;
    }
    else if (pStream->header.usSampleRate >= BCF_RATE_MEDIUM)
    {
        uFrame = BCF_SEEK_UNIT_MEDIUM;
    }

    if (uSample > pStream->header.uTotalSamples)
    {
        uSample = pStream->header.uTotalSamples - 1;
    }

    uUnit = uFrame * pStream->uSeekGranularity;
    uEntry = 0;
    if (uSample >= uUnit)
    {
        uEntry = (uSample - uUnit) / uUnit + 1;
        if (uEntry >= pStream->header.uSeekCount)
        {
            uEntry = pStream->header.uSeekCount - 1;
        }

        uSample = (uSample - uUnit) - (uEntry - 1) * uUnit;
    }

    if (piOffset != NULL)
    {
        *piOffset = pStream->pSeekOffsets[uEntry] + pStream->uDataOffset;
    }

    if (piRemainder != NULL)
    {
        *piRemainder = uSample;
    }

    if (piBase != NULL)
    {
        *piBase = uUnit + (uEntry - 1) * uFrame * pStream->uSeekGranularity;
    }
}

int ASI_stream_property(BcfStreamHandle iStream, unsigned int uProperty, void* pBefore, const void* pNewValue, void* pAfter)
{
    BcfStream* pStream = reinterpret_cast<BcfStream*>(iStream);
    unsigned int* puBefore = static_cast<unsigned int*>(pBefore);
    const unsigned int* puNew = static_cast<const unsigned int*>(pNewValue);
    unsigned int* puAfter = static_cast<unsigned int*>(pAfter);
    int iStartOffset = 0;
    int iStartRemainder = 0;
    int iEndOffset = 0;
    int iEndRemainder = 0;
    unsigned int uSample;
    unsigned int uStart;
    unsigned int uEnd;
    unsigned int uUnit;
    unsigned int uTarget;
    unsigned int uBuffered;
    unsigned int uLow;
    unsigned int uCursor;
    unsigned int uHigh;
    unsigned int uStep;
    unsigned int uMilliseconds;
    unsigned int uPendingMilliseconds;
    unsigned int uSampleRate;
    unsigned int uChannels;
    int iRemaining;
    float fIndex;

    if (uProperty != ASI_STREAM_SEEK_POSITION && uProperty != ASI_EXACT_SEEK && uProperty != ASI_EXACT_GET_POSITION
        && uProperty != ASI_SEEK_TABLE_LOOKUP && uProperty != ASI_SET_LOOPING_SAMPLES
        && uProperty != ASI_CLEAR_LOOP_META)
    {
        if (pBefore == NULL)
        {
            return 0;
        }

        if (pNewValue != NULL || pAfter != NULL)
        {
            return 0;
        }
    }

    switch (uProperty)
    {
    case ASI_INPUT_BIT_RATE:
        *puBefore = (unsigned int)(((unsigned long long)pStream->uFileBytes * BCF_BITRATE_SCALE)
                                   / pStream->uDurationMs);
        return 1;
    case ASI_INPUT_SAMPLE_RATE:
    case ASI_OUTPUT_SAMPLE_RATE:
        *puBefore = pStream->header.usSampleRate;
        return 1;
    case ASI_INPUT_SAMPLE_WIDTH:
    case ASI_OUTPUT_SAMPLE_WIDTH:
        *puBefore = BCF_BITS_PER_SAMPLE;
        return 1;
    case ASI_INPUT_CHANNELS:
    case ASI_OUTPUT_CHANNELS:
        *puBefore = pStream->header.ucChannels;
        return 1;
    case ASI_OUTPUT_BIT_RATE:
        *puBefore = pStream->header.ucChannels * pStream->header.usSampleRate * BCF_BITS_PER_SAMPLE;
        return 1;
    case ASI_OUTPUT_RESERVOIR:
        iRemaining = pStream->uOutputBytes - pStream->uOutputPosition;
        if (iRemaining == 0)
        {
            iRemaining = pStream->uInputFill - pStream->uInputPosition;
        }

        *puBefore = iRemaining;
        return 1;
    case ASI_POSITION:
        *puBefore = pStream->uStreamPosition;
        return 1;
    case ASI_PERCENT_DONE:
        if (pStream->uStreamPosition != 0 && pStream->uFileBytes != 0)
        {
            *static_cast<float*>(pBefore) = ((float)pStream->uStreamPosition * BCF_PERCENT_SCALE)
                                            / (float)pStream->uFileBytes;
            return 1;
        }

        *static_cast<float*>(pBefore) = 0.0f;
        return 1;
    case ASI_STREAM_SEEK_POSITION:
        if (puNew != NULL)
        {
            if (pStream->uInputPosition != pStream->uInputFill)
            {
                pStream->uPendingSeek = *puNew | BCF_PENDING_FLAG;
                return 1;
            }

            pStream->uStreamPosition = *puNew;
            pStream->iSeekPending = 1;
            return 1;
        }

        break;
    case ASI_EXACT_SEEK:
        if (pStream->header.uSeekCount != 0 && pStream->uSeekGranularity != 0 && puNew != NULL && puAfter != NULL)
        {
            uSample = (unsigned int)(((unsigned long long)*puNew * pStream->header.usSampleRate) / BCF_MILLISECONDS);
            iStartOffset = 0;
            iStartRemainder = 0;
            SeekTableLookup(pStream, uSample, &iStartOffset, &iStartRemainder, NULL);
            *puAfter = iStartOffset;
            pStream->uSkipSamples = iStartRemainder;
            return 1;
        }

        break;
    case ASI_EXACT_GET_POSITION:
        if (puNew == NULL)
        {
            return 0;
        }

        uHigh = pStream->header.uSeekCount;
        uTarget = *puNew;
        if (uHigh == 0)
        {
            return 0;
        }

        if (pStream->uSeekGranularity == 0)
        {
            return 0;
        }

        if (puAfter == NULL)
        {
            return 0;
        }

        uSampleRate = pStream->header.usSampleRate;
        uUnit = BcfSeekUnit(uSampleRate) * pStream->uSeekGranularity;
        uChannels = pStream->header.ucChannels;
        uBuffered = uTarget;
        if ((uTarget & BCF_PENDING_FLAG) != 0)
        {
            uBuffered = pStream->uInputFill - pStream->uInputPosition;
            if ((uTarget & BCF_PENDING_MASK) <= uBuffered)
            {
                *puAfter = 0;
                return 1;
            }

            uBuffered = (uTarget & BCF_PENDING_MASK) - uBuffered;
        }

        if (pStream->uDataOffset < uBuffered)
        {
            uBuffered = uBuffered - pStream->uDataOffset;
            if (pStream->pSeekOffsets[uHigh] <= uBuffered)
            {
                *puAfter = pStream->uDurationMs;
                return 1;
            }

            uLow = 0;
            uCursor = 0;
            uStep = ((uHigh + 1) >> 1) | 1;
            for (;;)
            {
                if (uCursor <= uHigh)
                {
                    while (pStream->pSeekOffsets[uCursor] <= uBuffered)
                    {
                        uLow = uCursor;
                        uCursor = uCursor + uStep;
                        if (uCursor > uHigh)
                        {
                            break;
                        }
                    }
                }

                if (uCursor < uHigh)
                {
                    uHigh = uCursor;
                }

                if (uStep == 1)
                {
                    break;
                }

                uCursor = uLow;
                uStep = (uHigh - uLow) >> 1;
                if (uStep <= 16)
                {
                    uStep = 1;
                    uCursor = uLow + 1;
                }
            }

            fIndex = (float)uLow
                     + (float)(uBuffered - pStream->pSeekOffsets[uLow])
                           / (float)(pStream->pSeekOffsets[uLow + 1] - pStream->pSeekOffsets[uLow]);
            uMilliseconds = (unsigned int)(((double)fIndex * 1000.0 * (double)(uUnit * uChannels * 2))
                                           / (double)(int)(uChannels * uSampleRate * 2));
            if ((uTarget & BCF_PENDING_FLAG) != 0)
            {
                uPendingMilliseconds =
                    (unsigned int)(((unsigned long long)(pStream->uOutputBytes - pStream->uOutputPosition)
                                    * BCF_MILLISECONDS)
                                   / (uChannels * uSampleRate * 2));
                if (uMilliseconds > uPendingMilliseconds)
                {
                    *puAfter = uMilliseconds - uPendingMilliseconds;
                    return 1;
                }

                uMilliseconds = 0;
            }

            *puAfter = uMilliseconds;
            return 1;
        }

        *puAfter = 0;
        return 1;
    case ASI_SEEK_TABLE_LOOKUP:
        if (pStream->header.uSeekCount != 0 && pStream->uSeekGranularity != 0 && puNew != NULL && puAfter != NULL)
        {
            uSample = (unsigned int)(((unsigned long long)*puNew * pStream->header.usSampleRate) / BCF_MILLISECONDS);
            SeekTableLookup(pStream, uSample, reinterpret_cast<int*>(puAfter), NULL,
                            reinterpret_cast<int*>(puAfter) + 1);
            return 1;
        }

        break;
    case ASI_SET_LOOPING_SAMPLES:
        if (pStream->header.uSeekCount != 0 && pStream->uSeekGranularity != 0 && puNew != NULL && puAfter != NULL)
        {
            uStart = puNew[0];
            uEnd = puNew[1];
            SeekTableLookup(pStream, uStart, &iStartOffset, &iStartRemainder, NULL);
            SeekTableLookup(pStream, uEnd, &iEndOffset, &iEndRemainder, NULL);
            uUnit = BcfSeekUnit(pStream->header.usSampleRate);
            puAfter[0] = ASI_LOOP_UNSET;
            if (uStart != ASI_LOOP_UNSET)
            {
                pStream->uLoopStartRemainder = iStartRemainder;
                puAfter[0] = iStartOffset;
            }

            if (uEnd == ASI_LOOP_CLEARED)
            {
                pStream->uLoopEndOffset = 0;
                pStream->uLoopEndRemainder = 0;
                puAfter[1] = ASI_LOOP_CLEARED;
                return 1;
            }

            if ((int)uEnd < 0)
            {
                puAfter[1] = ASI_LOOP_UNSET;
                return 1;
            }

            pStream->uLoopEndOffset = iEndOffset;
            pStream->uLoopEndRemainder = iEndRemainder;
            SeekTableLookup(pStream, uEnd + uUnit * pStream->uSeekGranularity, &iEndOffset, NULL, NULL);
            puAfter[1] = iEndOffset;
            return 1;
        }

        break;
    case ASI_CLEAR_LOOP_META:
        if (puNew != NULL)
        {
            if ((*puNew & ASI_CLEAR_LOOP_START) != 0)
            {
                pStream->uLoopStartRemainder = 0;
            }

            if ((*puNew & ASI_CLEAR_LOOP_END) != 0)
            {
                pStream->uLoopEndOffset = 0;
                pStream->uLoopEndRemainder = 0;
            }

            return 1;
        }

        break;
    default:
        break;
    }

    return 0;
}

int BinkADec_RIB_Main(unsigned int uProvider, int iUpDown, RIB_ALLOC_PROVIDER_HANDLE_CALLBACK pAllocProviderHandle,
                      RIB_REGISTER_INTERFACE_CALLBACK pRegisterInterface,
                      RIB_UNREGISTER_INTERFACE_CALLBACK pUnregisterInterface)
{
    const RIB_INTERFACE_ENTRY codecProviderEntries[4] = {
        {RIB_FUNCTION, "PROVIDER_property", reinterpret_cast<void*>(PROVIDER_property), 0},
        {RIB_PROPERTY, "Name", reinterpret_cast<void*>(RIB_PROVIDER_NAME), RIB_READ_ONLY | RIB_ENTRY_STRING},
        {RIB_PROPERTY, "Version", reinterpret_cast<void*>(RIB_PROVIDER_VERSION), RIB_READ_ONLY | RIB_ENTRY_HEXADECIMAL},
        {RIB_PROPERTY, "Input file types", reinterpret_cast<void*>(ASI_INPUT_FILE_TYPES),
         RIB_READ_ONLY | RIB_ENTRY_STRING}};
    const RIB_INTERFACE_ENTRY codecTypeEntries[4] = {
        {RIB_PROPERTY, "Editor-supported input file types", reinterpret_cast<void*>(ASI_EDITOR_INPUT_FILE_TYPES),
         RIB_READ_ONLY | RIB_ENTRY_STRING},
        {RIB_PROPERTY, "Output file types", reinterpret_cast<void*>(ASI_OUTPUT_FILE_TYPES),
         RIB_READ_ONLY | RIB_ENTRY_STRING},
        {RIB_PROPERTY, "Profile index", reinterpret_cast<void*>(ASI_PROFILE_PROPERTY),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "SPU flags", reinterpret_cast<void*>(ASI_SPU_FLAGS_PROPERTY),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL}};
    const RIB_INTERFACE_ENTRY codecEntries[3] = {
        {RIB_FUNCTION, "ASI_startup", reinterpret_cast<void*>(ASI_startup), 0},
        {RIB_FUNCTION, "ASI_error", reinterpret_cast<void*>(ASI_error), 0},
        {RIB_FUNCTION, "ASI_shutdown", reinterpret_cast<void*>(ASI_shutdown), 0}};
    const RIB_INTERFACE_ENTRY streamEntries[3] = {
        {RIB_FUNCTION, "ASI_stream_open", reinterpret_cast<void*>(ASI_stream_open), 0},
        {RIB_FUNCTION, "ASI_stream_process", reinterpret_cast<void*>(ASI_stream_process), 0},
        {RIB_FUNCTION, "ASI_stream_property", reinterpret_cast<void*>(ASI_stream_property), 0}};
    const RIB_INTERFACE_ENTRY streamControlEntries[4] = {
        {RIB_FUNCTION, "ASI_stream_seek", reinterpret_cast<void*>(ASI_stream_seek), 0},
        {RIB_FUNCTION, "ASI_stream_close", reinterpret_cast<void*>(ASI_stream_close), 0},
        {RIB_PROPERTY, "Input bit rate", reinterpret_cast<void*>(ASI_INPUT_BIT_RATE),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Input sample rate", reinterpret_cast<void*>(ASI_INPUT_SAMPLE_RATE),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL}};
    const RIB_INTERFACE_ENTRY streamFormatEntries[4] = {
        {RIB_PROPERTY, "Input sample width", reinterpret_cast<void*>(ASI_INPUT_SAMPLE_WIDTH),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Input channels", reinterpret_cast<void*>(ASI_INPUT_CHANNELS),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Output bit rate", reinterpret_cast<void*>(ASI_OUTPUT_BIT_RATE),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Output sample rate", reinterpret_cast<void*>(ASI_OUTPUT_SAMPLE_RATE),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL}};
    const RIB_INTERFACE_ENTRY streamOutputEntries[5] = {
        {RIB_PROPERTY, "Output sample width", reinterpret_cast<void*>(ASI_OUTPUT_SAMPLE_WIDTH),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Output channels", reinterpret_cast<void*>(ASI_OUTPUT_CHANNELS),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Output reservoir", reinterpret_cast<void*>(ASI_OUTPUT_RESERVOIR),
         RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Position", reinterpret_cast<void*>(ASI_POSITION), RIB_READ_ONLY | RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Set Looping Samples", reinterpret_cast<void*>(ASI_SET_LOOPING_SAMPLES), RIB_ENTRY_DECIMAL}};
    const RIB_INTERFACE_ENTRY streamSeekEntries[6] = {
        {RIB_PROPERTY, "Percent done", reinterpret_cast<void*>(ASI_PERCENT_DONE), RIB_READ_ONLY | RIB_ENTRY_PERCENT},
        {RIB_PROPERTY, "Stream seek position", reinterpret_cast<void*>(ASI_STREAM_SEEK_POSITION), RIB_ENTRY_DECIMAL},
        {RIB_PROPERTY, "Exact seek", reinterpret_cast<void*>(ASI_EXACT_SEEK), RIB_READ_ONLY | RIB_ENTRY_CUSTOM},
        {RIB_PROPERTY, "Exact get position", reinterpret_cast<void*>(ASI_EXACT_GET_POSITION),
         RIB_READ_ONLY | RIB_ENTRY_CUSTOM},
        {RIB_PROPERTY, "Seek table lookup", reinterpret_cast<void*>(ASI_SEEK_TABLE_LOOKUP),
         RIB_READ_ONLY | RIB_ENTRY_CUSTOM},
        {RIB_PROPERTY, "Clear Loop Meta", reinterpret_cast<void*>(ASI_CLEAR_LOOP_META),
         RIB_READ_ONLY | RIB_ENTRY_CUSTOM}};

    if (iUpDown)
    {
        pRegisterInterface(uProvider, ASI_CODEC_INTERFACE, 4, codecProviderEntries);
        pRegisterInterface(uProvider, ASI_CODEC_INTERFACE, 4, codecTypeEntries);
        pRegisterInterface(uProvider, ASI_CODEC_INTERFACE, 3, codecEntries);
        pRegisterInterface(uProvider, ASI_STREAM_INTERFACE, 3, streamEntries);
        pRegisterInterface(uProvider, ASI_STREAM_INTERFACE, 4, streamControlEntries);
        pRegisterInterface(uProvider, ASI_STREAM_INTERFACE, 4, streamFormatEntries);
        pRegisterInterface(uProvider, ASI_STREAM_INTERFACE, 5, streamOutputEntries);
        pRegisterInterface(uProvider, ASI_STREAM_INTERFACE, 6, streamSeekEntries);
    }
    else
    {
        pUnregisterInterface(uProvider, NULL, 0, NULL);
    }

    return 1;
}

int ASI_shutdown()
{
    return 0;
}
