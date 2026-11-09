#ifndef SOURCE_VENDOR_BINKADEC_HPP
#define SOURCE_VENDOR_BINKADEC_HPP

#include <stdint.h>

#include "vendor/MilesRib.hpp"

struct BINKAUDIODECOMP;

#if __SIZEOF_POINTER__ == 8
typedef intptr_t BcfStreamHandle;
#else
typedef int BcfStreamHandle;
#endif

typedef void* (*BCF_ALLOC_CALLBACK)(unsigned int uSize, unsigned int uUser, const char* pFile, unsigned int uLine);
typedef void (*BCF_FREE_CALLBACK)(void* pBlock, unsigned int uUser, const char* pFile, unsigned int uLine);
typedef int (*BCF_FETCH_CALLBACK)(unsigned int uUser, void* pDestination, int iBytes, int iOffset);

static const unsigned int BCF_MAX_STREAMS = 4;

struct BcfHeader
{
    unsigned int uMagic;
    unsigned char ucVersion;
    unsigned char ucChannels;
    unsigned short usSampleRate;
    unsigned int uTotalSamples;
    unsigned int uMaxFrameBytes;
    unsigned int uFileBytes;
    unsigned int uSeekCount;
};

struct BcfStream
{
    unsigned int uReserved00;
    unsigned int uReserved04;
    unsigned char* pOutput;
    unsigned int uReserved0C;
    unsigned int uOutputBytes;
    unsigned int uOutputPosition;
    unsigned int uUser;
    BCF_FETCH_CALLBACK pFetch;
    BCF_FREE_CALLBACK pFree;
    unsigned int uFileBytes;
    unsigned int uStreamPosition;
    int iSeekPending;
    int iNeedSync;
    BcfHeader header;
    unsigned int* pSeekOffsets;
    unsigned int uDurationMs;
    unsigned int uSeekGranularity;
    unsigned char* pInput;
    unsigned int uInputFill;
    unsigned int uInputPosition;
    unsigned int uInputCapacity;
    unsigned int uReserved68;
    unsigned int uOutputCapacity;
    unsigned int uFrameBytes;
    unsigned int uSkipSamples;
    unsigned int uLoopStartRemainder;
    unsigned int uLoopEndOffset;
    unsigned int uLoopEndRemainder;
    unsigned int uSamplesRemaining;
    unsigned int uDataOffset;
    unsigned int uPendingSkip;
    unsigned int uPendingSeek;
    int iResyncPending;
    unsigned int uStreamCount;
    unsigned int auStreamChannels[BCF_MAX_STREAMS];
    unsigned int auStreamBytes[BCF_MAX_STREAMS];
    BINKAUDIODECOMP* apDecoders[BCF_MAX_STREAMS];
    unsigned int uReservedCC;
};

int PROVIDER_property(unsigned int uProperty, void* pBefore, const void* pNewValue, void* pAfter);
const char* ASI_error();
int ASI_startup();
BcfStreamHandle ASI_stream_open(BCF_ALLOC_CALLBACK pAlloc, BCF_FREE_CALLBACK pFree, unsigned int uUser, BCF_FETCH_CALLBACK pFetch,
                    unsigned int uTotalSize);
int ASI_stream_close(BcfStreamHandle iStream);
void decode_bink_buffer(BcfStream* pStream);
void FindFrameBoundary(BcfStreamHandle iStream);
void SeekStreamForward(BcfStream* pStream, int iBytes);
int ASI_stream_process(BcfStreamHandle iStream, void* pBuffer, int iBytes);
int ASI_stream_seek(BcfStreamHandle iStream, int iPosition);
void SeekTableLookup(BcfStream* pStream, unsigned int uSample, int* piOffset, int* piRemainder, int* piBase);
int ASI_stream_property(BcfStreamHandle iStream, unsigned int uProperty, void* pBefore, const void* pNewValue, void* pAfter);
int ASI_shutdown();

extern "C" int BinkADec_RIB_Main(unsigned int uProvider, int iUpDown,
                                 RIB_ALLOC_PROVIDER_HANDLE_CALLBACK pAllocProviderHandle,
                                 RIB_REGISTER_INTERFACE_CALLBACK pRegisterInterface,
                                 RIB_UNREGISTER_INTERFACE_CALLBACK pUnregisterInterface);

#endif
