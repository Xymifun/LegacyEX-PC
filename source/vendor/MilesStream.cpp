#include "vendor/MilesStream.hpp"

#include <stddef.h>
#include <string.h>

#include "vendor/MilesIO.hpp"
#include "vendor/MilesMem.hpp"
#include "vendor/MilesRib.hpp"

extern "C" void InMilesMutex();
extern "C" void OutMilesMutex();
extern "C" void* AIL_mem_alloc_lock_info(int iSize, const char* pOwnerName, int iOwnerLine);
extern "C" void AIL_mem_free_lock(void* pBlock);
extern "C" void AIL_set_error(const char* pError);
extern "C" _SAMPLE* AIL_allocate_sample_handle(_DIG_DRIVER* pDriver);
extern "C" void AIL_release_sample_handle(_SAMPLE* pSample);
extern "C" void AIL_init_sample(_SAMPLE* pSample, int iFormat);
extern "C" void AIL_set_sample_playback_rate(_SAMPLE* pSample, int iRate);
extern "C" int SS_set_sample_type(_SAMPLE* pSample, unsigned int uFlags, int iChannelMask);
extern "C" void AIL_set_sample_adpcm_block_size(_SAMPLE* pSample, unsigned int uBlockSize);
extern "C" int AIL_minimum_sample_buffer_size(_DIG_DRIVER* pDriver, int iRate, unsigned int uFormat);
extern "C" void AIL_set_sample_address(_SAMPLE* pSample, const void* pStart, unsigned int uLength);
extern "C" int AIL_set_sample_processor(_SAMPLE* pSample, int iStage, int iProvider);
extern "C" int AIL_set_sample_buffer_count(_SAMPLE* pSample, int iCount);
extern "C" int AIL_sample_buffer_available(_SAMPLE* pSample);
extern "C" int AIL_load_sample_buffer(_SAMPLE* pSample, int iBuffer, const void* pData, unsigned int uLength);
extern "C" void AIL_set_sample_position(_SAMPLE* pSample, unsigned int uPosition);
extern "C" unsigned int AIL_sample_position(_SAMPLE* pSample);
extern "C" unsigned int AIL_sample_status(_SAMPLE* pSample);
extern "C" int AIL_API_sample_granularity(_SAMPLE* pSample);
extern "C" void AIL_start_sample(_SAMPLE* pSample);
extern "C" void AIL_stop_sample(_SAMPLE* pSample);
extern "C" void AIL_resume_sample(_SAMPLE* pSample);
extern "C" void AIL_set_sample_loop_count(_SAMPLE* pSample, int iLoopCount);
extern "C" void AIL_set_sample_loop_block(_SAMPLE* pSample, int iLoopStart, int iLoopEnd);
extern "C" int AIL_register_timer(void (*pCallback)(unsigned int uUser));
extern "C" void AIL_set_timer_frequency(int iTimer, unsigned int uFrequency);
extern "C" void AIL_start_timer(int iTimer);
extern "C" void AIL_release_timer_handle(int iTimer);
extern "C" int AIL_stream_position(_STREAM* pStream);
extern "C" int AIL_stream_status(_STREAM* pStream);
extern "C" void AIL_set_stream_position(_STREAM* pStream, int iPosition);
extern "C" void AIL_close_stream(_STREAM* pStream);

static const char MILES_STREAM_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\mssstrm.cpp";
static const char STREAM_PERCENT_FORMAT[] = "Miles/Streams/Stream %d %%";
static const char OUT_OF_MEMORY_ERROR[] = "Out of memory.";
static const char OPEN_FILE_ERROR[] = "Unable to open file.";
static const char SAMPLE_HANDLE_ERROR[] = "Unable to obtain a sample handle.";
static const char SOUND_FORMAT_ERROR[] = "Error getting sound format.";
static const char TIMER_HANDLE_ERROR[] = "Out of timer handles.";
static const char ASI_CODEC_INTERFACE[] = "ASI codec";
static const char ASI_STREAM_INTERFACE[] = "ASI stream";
static const char INPUT_FILE_TYPES[] = "Input file types";
static const char INPUT_WAVE_TAG[] = "Input wave tag";
static const char OUTPUT_FILE_TYPES[] = "Output file types";
static const char RAW_FILE_TYPE[] = ".raw";
static const char MINIMUM_INPUT_STREAM_SIZE[] = "Minimum input stream size";

static const int STREAM_STRUCT_LINE = 712;
static const int STREAM_PRELOAD_LINE = 804;
static const int STREAM_BUFFERS_LINE = 822;
static const int STREAM_READ_LINE = 1074;

static const unsigned int RIB_PROPERTY_ENTRY = 1;
static const unsigned int PROPERTY_NOT_FOUND = 0xFFFFFFFF;

static const int SEEK_FROM_START = 0;
static const int SEEK_FROM_CURRENT = 1;
static const int SEEK_FROM_END = 2;

static const unsigned int RIFF_TAG = 0x46464952;
static const unsigned int WAVE_TAG = 0x45564157;
static const unsigned int FORMAT_TAG = 0x20746d66;
static const unsigned int DATA_TAG = 0x61746164;
static const unsigned int WAVE_FORMAT_PCM = 1;
static const unsigned int WAVE_FORMAT_IMA_ADPCM = 0x11;
static const unsigned int WAVE_FORMAT_EXTENSIBLE = 0xFFFE;
static const unsigned int EXTENSIBLE_MINIMUM_SIZE = 0x15;

static const unsigned int FORMAT_16_BIT = 1;
static const unsigned int FORMAT_STEREO = 2;
static const unsigned int FORMAT_ADPCM = 4;
static const unsigned int FORMAT_ADPCM_MONO_16 = 5;
static const unsigned int FORMAT_MULTICHANNEL = 0x10;
static const int FORMAT_CHANNEL_SHIFT = 16;

static const int ASI_PROBE_SIZE = 0x2000;
static const int ID3_HEADER_SIZE = 16;
static const int ID3_TAG_OVERHEAD = 10;
static const unsigned char ID3_MAXIMUM_VERSION_BYTE = 0xFE;
static const unsigned char ID3_MAXIMUM_SIZE_BYTE = 0x7F;
static const int GUID_SIZE = 16;

static const int MINIMUM_STREAM_SLACK = 0x3000;
static const int BUFFER_ALIGNMENT = 0x1000;
static const int BUFFER_PADDING = 0x40;
static const int SILENCE_LENGTH = 0x20;
static const int SECTOR_MASK = 0x7FF;
static const unsigned int SECTOR_MINIMUM_READ = 0x800;
static const int UNLIMITED_LOOP_END = 0x7FFFFFFF;
static const float PERCENT_UNSET = -1.0f;
static const float MILLISECONDS_PER_SECOND = 1000.0f;

static const unsigned int STREAM_TIMER_FREQUENCY = 16;
static const int STREAM_PLAY_FLAG_PLAYING = 1;
static const unsigned int STREAM_PLAY_MASK = 7;
static const unsigned int STREAM_PLAY_PAUSED = 8;
static const unsigned int STREAM_PLAY_SAMPLE_STOPPED = 0x10;
static const unsigned int STREAM_PLAY_PAUSE_BITS = 0x18;
static const unsigned int SAMPLE_STATUS_DONE = 2;
static const unsigned int SAMPLE_STATUS_PLAYING = 4;
static const unsigned int SAMPLE_STATUS_STOPPED = 8;
static const int STREAM_READ_PENDING = 1;
static const int STREAM_READ_COMPLETE = 2;
static const int STREAM_IO_WAITING = 1;
static const int STREAM_IO_ACTIVE = 2;
static const int STREAM_END_PENDING = 1;
static const int STREAM_END_QUEUED = 2;
static const int ASYNC_STATUS_CANCELLED = 0x1006;
static const int ASYNC_STATUS_FINISHED_MASK = 0x1000;
static const unsigned int ASYNC_FLAG_KEEP_OPEN = 1;
static const unsigned int ASYNC_FLAG_NO_SEEK = 4;
static const unsigned int ASYNC_WAIT_FOREVER = 0xFFFFFFFF;
static const int DECODER_STAGE = SAMPLE_STAGE_DECODER;

static _STREAM* StreamList = NULL;
static int StreamTimer = -1;
static int StreamCounter = 0;
static MilesProfileSite StreamPercentSite;

#define LITTLE_ENDIAN_32(uValue) \
    (((uValue) << 24) | (((uValue) >> 8 & 0xff) << 16) | (((uValue) >> 16 & 0xff) << 8) | ((uValue) >> 24))
#define LITTLE_ENDIAN_HIGH_16(uValue) ((((uValue) >> 16 & 0xff) << 8) | ((uValue) >> 24))
#define LITTLE_ENDIAN_LOW_16(uValue) ((((uValue) & 0xff) << 8) | ((uValue) >> 8 & 0xff))

#define QUEUE_ASI_RESET(pStream, iBuffer, pLength, iPosition) \
    do \
    { \
        int iLength = *(pLength); \
        if (iLength == 0) \
        { \
            *(pLength) = SILENCE_LENGTH; \
            memset((pStream)->buffers[iBuffer], 0, SILENCE_LENGTH); \
            iLength = *(pLength); \
        } \
        (pStream)->resetASI[iBuffer] = iLength + 1; \
        (pStream)->resetPosition[iBuffer] = (iPosition); \
    } while (0)

void handle_asyncs(_STREAM* pStream, int iWait)
{
    for (int iIndex = 0; iIndex < STREAM_BUFFER_COUNT; ++iIndex)
    {
        if (pStream->readStatus[iIndex] != STREAM_READ_PENDING)
        {
            continue;
        }

        MilesAsyncRead* pRead = &pStream->reads[iIndex];
        int iDone = MilesAsyncFileStatus(pRead, iWait == 0 ? 0 : ASYNC_WAIT_FOREVER);
        int iBytesRead = pRead->iBytesRead;
        if (iDone != 0 && pRead->iStatus != ASYNC_STATUS_CANCELLED)
        {
            if ((pRead->iStatus & ASYNC_STATUS_FINISHED_MASK) != 0)
            {
                iBytesRead = 0;
            }

            pStream->readStatus[iIndex] = STREAM_READ_COMPLETE;
            if (iBytesRead == 0)
            {
                pStream->error = 1;
                AIL_stop_sample(pStream->sample);
            }
        }
    }
}

void cancel_asyncs(_STREAM* pStream)
{
    for (int iIndex = 0; iIndex < STREAM_BUFFER_COUNT; ++iIndex)
    {
        if (pStream->readStatus[iIndex] == STREAM_READ_PENDING)
        {
            MilesAsyncFileCancel(&pStream->reads[iIndex]);
        }
    }
}

extern "C" float AIL_API_stream_filled_percent(_STREAM* pStream)
{
    if (pStream != NULL && pStream->error == 0)
    {
        if (pStream->preload != 0 || pStream->startRemaining == 0)
        {
            return 1.0f;
        }

        return pStream->fillPercent;
    }

    return 0.0f;
}

float MilesStreamCB(void* pUserData)
{
    return AIL_API_stream_filled_percent(static_cast<_STREAM*>(pUserData));
}

int start_IOs_if_we_can(_STREAM* pStream)
{
    int iStarted = 0;
    unsigned int uLoaded;
    int iBuffer;
    int iPasses;

    if (pStream->preloadLoaded != 0 || pStream->endOfData != 0)
    {
        return 0;
    }

    uLoaded = AIL_API_sample_loaded_len(pStream->sample);
    if (pStream->preload == 0)
    {
        if (uLoaded != 0 && pStream->playStatus == STREAM_PLAY_FLAG_PLAYING && pStream->error == 0
            && ((int)uLoaded > pStream->startRemaining
                || (unsigned int)pStream->readPosition >= (unsigned int)pStream->dataLength))
        {
            pStream->startRemaining = 0;
            AIL_start_sample(pStream->sample);
        }

        float fPercent = pStream->percentScale * (float)(int)uLoaded;
        pStream->fillPercent = fPercent;
        if (pStream->reportedPercent != fPercent)
        {
            if (MSS_profile != NULL)
            {
                MSS_profile->pPlot(MSS_profile, 0, 5, 0, fPercent, &StreamPercentSite, STREAM_PERCENT_FORMAT,
                                   pStream->id);
                fPercent = pStream->fillPercent;
            }

            pStream->reportedPercent = fPercent;
        }
    }

    iBuffer = pStream->readBuffer;
    iPasses = STREAM_BUFFER_COUNT;
    do
    {
        _SAMPLE* pSample = pStream->sample;
        if (pStream->readStatus[iBuffer] != 0 || pSample->buffers[iBuffer].position != pSample->buffers[iBuffer].size)
        {
            break;
        }

        int iPosition = pStream->readPosition;
        int* pLength = &pStream->bufferLength[iBuffer];
        unsigned int uToRead = pStream->dataLength - iPosition;
        if (uToRead > (unsigned int)pStream->bufferSize)
        {
            uToRead = pStream->bufferSize;
        }

        if (uToRead > (unsigned int)(pStream->loopEnd - iPosition))
        {
            uToRead = pStream->loopEnd - iPosition;
        }

        unsigned int uReadLength = 0;
        if (uToRead != 0 && pStream->buffers[iBuffer] != NULL)
        {
            unsigned int uSectorOffset = (iPosition + pStream->dataStart) & SECTOR_MASK;
            if (uSectorOffset != 0 && pStream->preload == 0 && uSectorOffset < uToRead)
            {
                if (uToRead - uSectorOffset > SECTOR_MINIMUM_READ
                    && uSectorOffset % AIL_API_sample_granularity(pStream->sample) == 0)
                {
                    uToRead = uToRead - uSectorOffset;
                }
            }

            if (uToRead != 0)
            {
                MilesAsyncRead* pRead = &pStream->reads[iBuffer];
                memset(pRead, 0, sizeof(MilesAsyncRead));
                pRead->pBuffer = pStream->buffers[iBuffer] + *pLength;
                pRead->uFileHandle = pStream->fileHandle;
                pRead->iLengthHigh = 0;
                pRead->iLength = uToRead;
                pRead->pPriorityCallback = MilesStreamCB;
                pRead->pOwnerName = MILES_STREAM_FILE;
                pRead->iOwnerLine = STREAM_READ_LINE;
                if (pStream->seekOffset != -1)
                {
                    pRead->uFlags = ASYNC_FLAG_KEEP_OPEN;
                    pRead->lOffset = pStream->seekOffset;
                    pStream->seekOffset = -1;
                    pStream->readStatus[iBuffer] = STREAM_READ_PENDING;
                    MilesAsyncFileRead(pRead);
                }
                else
                {
                    pRead->uFlags = ASYNC_FLAG_KEEP_OPEN | ASYNC_FLAG_NO_SEEK;
                    pStream->readStatus[iBuffer] = STREAM_READ_PENDING;
                    MilesAsyncFileRead(pRead);
                }
            }

            pStream->bufferStart[iBuffer] = pStream->readPosition;
            uReadLength = uToRead;
            pStream->readPosition = pStream->readPosition + uReadLength;
            *pLength = *pLength + uReadLength;
        }
        else
        {
            pStream->bufferStart[iBuffer] = iPosition;
        }

        if (uReadLength != uToRead)
        {
            pStream->error = 1;
        }

        if (pStream->loopEnd == pStream->readPosition)
        {
            if (pStream->loopCount != 0)
            {
                pStream->loopCount = pStream->loopCount - 1;
                if (pStream->loopCount == 1)
                {
                    pStream->loopEnd = UNLIMITED_LOOP_END;
                }
            }

            pStream->readPosition = pStream->loopStart;
            pStream->seekOffset = pStream->loopStart + pStream->dataStart;
            if (pStream->usingASI != 0 || pStream->blockOriented != 0)
            {
                QUEUE_ASI_RESET(pStream, iBuffer, pLength, pStream->loopStart);
            }
        }

        if (pStream->dataLength == pStream->readPosition)
        {
            if (pStream->loopCount == 1)
            {
                pStream->endOfData = STREAM_END_PENDING;
            }
            else
            {
                if (pStream->loopCount != 0)
                {
                    pStream->loopCount = pStream->loopCount - 1;
                }

                pStream->readPosition = 0;
                pStream->seekOffset = pStream->dataStart;
                if (pStream->usingASI != 0 || pStream->blockOriented != 0)
                {
                    QUEUE_ASI_RESET(pStream, iBuffer, pLength, 0);
                }
            }
        }

        if (pStream->error != 0 || uReadLength == 0)
        {
            return -1;
        }

        iBuffer = pStream->readBuffer + 1;
        if (iBuffer >= STREAM_BUFFER_COUNT)
        {
            iBuffer = 0;
        }

        iPasses = iPasses - 1;
        iStarted = iStarted + uReadLength;
        pStream->readBuffer = iBuffer;
    } while (iPasses != 0);

    if (pStream->preload != 0 && pStream->fileHandle != STREAM_NO_FILE)
    {
        pStream->ioState = 0;
    }
    else if (pStream->ioState == STREAM_IO_WAITING)
    {
        pStream->ioState = STREAM_IO_ACTIVE;
    }

    return iStarted;
}

void background(unsigned int uTimer)
{
    _STREAM* pStream = StreamList;
    InMilesMutex();
    while (pStream != NULL)
    {
        _STREAM* pNext;
        if (pStream->preloadLoaded == 0)
        {
            if (pStream->ioState == STREAM_IO_ACTIVE)
            {
                start_IOs_if_we_can(pStream);
            }

            for (;;)
            {
                int iLoad = pStream->loadBuffer;
                handle_asyncs(pStream, 0);
                if (pStream->endOfData == STREAM_END_PENDING && pStream->playStatus == STREAM_PLAY_FLAG_PLAYING
                    && pStream->bufferLength[iLoad] == 0)
                {
                    if (AIL_sample_buffer_available(pStream->sample) != 0)
                    {
                        AIL_load_sample_buffer(pStream->sample, pStream->loadBuffer, pStream->buffers[0], 0);
                        pStream->loadBuffer = pStream->loadBuffer + 1;
                        if (pStream->loadBuffer >= STREAM_BUFFER_COUNT)
                        {
                            pStream->loadBuffer = 0;
                        }

                        if (pStream->startRemaining != 0 && pStream->playStatus == STREAM_PLAY_FLAG_PLAYING
                            && pStream->error == 0)
                        {
                            pStream->startRemaining = 0;
                            AIL_start_sample(pStream->sample);
                        }

                        pStream->endOfData = STREAM_END_QUEUED;
                        if (pStream->callback != NULL)
                        {
                            pStream->callbackPending = 1;
                        }
                    }
                }

                if ((pStream->bufferLength[iLoad] == 0 && pStream->endOfData != STREAM_END_PENDING)
                    || pStream->playStatus != STREAM_PLAY_FLAG_PLAYING)
                {
                    break;
                }

                if (pStream->readStatus[iLoad] != STREAM_READ_COMPLETE)
                {
                    break;
                }

                if (pStream->preload != 0)
                {
                    AIL_set_sample_address(pStream->sample, pStream->buffers[iLoad], pStream->bufferLength[iLoad]);
                    if (pStream->preloadSeek != 0)
                    {
                        AIL_set_sample_position(pStream->sample, pStream->preloadSeek);
                        pStream->preloadSeek = 0;
                    }

                    pStream->readStatus[iLoad] = 0;
                    AIL_resume_sample(pStream->sample);
                    pStream->preloadLoaded = 1;
                    break;
                }

                InMilesMutex();
                if (AIL_sample_buffer_available(pStream->sample) != 0)
                {
                    pStream->loadBuffer = pStream->loadBuffer + 1;
                    if (pStream->loadBuffer >= STREAM_BUFFER_COUNT)
                    {
                        pStream->loadBuffer = 0;
                    }

                    if (pStream->resetASI[iLoad] != 0)
                    {
                        pStream->bufferLength[iLoad] = pStream->resetASI[iLoad] - 1;
                        pStream->resetASI[iLoad] = 0;
                        AIL_request_EOB_ASI_reset(pStream->sample, iLoad, pStream->resetPosition[iLoad]);
                    }

                    AIL_load_sample_buffer(pStream->sample, iLoad, pStream->buffers[iLoad],
                                           pStream->bufferLength[iLoad]);
                    pStream->sample->lastBuffer = pStream->loadBuffer;
                    pStream->readStatus[iLoad] = 0;
                    OutMilesMutex();
                    if (pStream->startRemaining <= pStream->bufferLength[iLoad]
                        && pStream->playStatus == STREAM_PLAY_FLAG_PLAYING && pStream->error == 0)
                    {
                        pStream->startRemaining = 0;
                        AIL_start_sample(pStream->sample);
                    }
                    else
                    {
                        pStream->startRemaining = pStream->startRemaining - pStream->bufferLength[iLoad];
                    }

                    pStream->bufferLength[iLoad] = 0;
                }
                else
                {
                    OutMilesMutex();
                    break;
                }
            }

            pNext = pStream->next;
            if (pStream->callbackPending != 0 && pStream->callback != NULL
                && AIL_sample_status(pStream->sample) != SAMPLE_STATUS_PLAYING)
            {
                pStream->callbackPending = 0;
                pStream->callback(pStream);
                pNext = StreamList;
            }
        }
        else
        {
            pNext = pStream->next;
        }

        pStream = pNext;
    }

    OutMilesMutex();
}

extern "C" _STREAM* AIL_API_open_stream(_DIG_DRIVER* pDriver, const char* pFileName, int iStreamMemory)
{
    _STREAM* pStream;
    unsigned int uProvider;
    bool bId3Checked;
    bool bId3First;
    bool bScan;
    bool bParsed;
    unsigned int uWord;
    unsigned int uChunk;
    unsigned int uChunkEnd;
    unsigned int uFormatTag;
    unsigned int uChannels;
    unsigned int uBits;
    unsigned int uSampleRate;
    unsigned int uChannelMask;
    unsigned int uFormat;
    unsigned int uExtensionSize;
    unsigned int uBlockUnit;
    int iBytes;
    int iPosition;
    int iMinimum;
    int iBufferSize;
    int iAsiMinimum;
    int iAsiBits;
    int iAsiMask;
    int iSeekExtra;
    unsigned int uAsiChannels;
    unsigned int uMinimumToken;
    DecoderStage* pDecoder;
    unsigned char formatGuid[GUID_SIZE];
    unsigned char id3Header[ID3_HEADER_SIZE];
    unsigned char probe[ASI_PROBE_SIZE];
    RIB_INTERFACE_ENTRY minimumEntry;
    const unsigned char pcmGuid[GUID_SIZE] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
                                              0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71};

    if (pDriver == NULL || pFileName == NULL)
    {
        return NULL;
    }

    pStream = static_cast<_STREAM*>(AIL_mem_alloc_lock_info(sizeof(_STREAM), MILES_STREAM_FILE, STREAM_STRUCT_LINE));
    if (pStream == NULL)
    {
        AIL_set_error(OUT_OF_MEMORY_ERROR);
        return NULL;
    }

    memset(pStream, 0, sizeof(_STREAM));
    pStream->seekOffset = -1;
    OutMilesMutex();
    if (AIL_file_open(pFileName, &pStream->fileHandle) == 0)
    {
        pStream->fileHandle = STREAM_NO_FILE;
    }

    InMilesMutex();
    if (pStream->fileHandle == STREAM_NO_FILE)
    {
        AIL_set_error(OPEN_FILE_ERROR);
        AIL_mem_free_lock(pStream);
        return NULL;
    }

    pStream->sample = AIL_allocate_sample_handle(pDriver);
    if (pStream->sample == NULL)
    {
        AIL_set_error(SAMPLE_HANDLE_ERROR);
        if (pStream->fileHandle != STREAM_NO_FILE)
        {
            AIL_file_close(pStream->fileHandle);
        }

        AIL_mem_free_lock(pStream);
        return NULL;
    }

    AIL_init_sample(pStream->sample, 1);
    OutMilesMutex();
    pStream->blockOriented = 0;
    pStream->usingASI = 0;
    pStream->channelMask = -1;
    bId3Checked = false;
    bId3First = false;
    bScan = false;
    bParsed = false;
    uProvider = RIB_find_file_provider(ASI_CODEC_INTERFACE, INPUT_FILE_TYPES, pFileName);
    if (uProvider == 0)
    {
        do
        {
            AIL_file_seek(pStream->fileHandle, pStream->dataStart, SEEK_FROM_START);
            AIL_file_read(pStream->fileHandle, &uWord, 4);
            if (LITTLE_ENDIAN_32(uWord) != RIFF_TAG)
            {
                bScan = true;
                bId3First = true;
                break;
            }

            AIL_file_read(pStream->fileHandle, &uWord, 4);
            AIL_file_read(pStream->fileHandle, &uWord, 4);
            if (LITTLE_ENDIAN_32(uWord) != WAVE_TAG)
            {
                break;
            }

            AIL_file_read(pStream->fileHandle, &uWord, 4);
            uChunk = LITTLE_ENDIAN_32(uWord);
            while (uChunk != FORMAT_TAG)
            {
                if (AIL_file_read(pStream->fileHandle, &uWord, 4) != 4)
                {
                    break;
                }

                uWord = (LITTLE_ENDIAN_32(uWord) + 1) & 0xFFFFFFFE;
                AIL_file_seek(pStream->fileHandle, uWord, SEEK_FROM_CURRENT);
                AIL_file_read(pStream->fileHandle, &uWord, 4);
                uChunk = LITTLE_ENDIAN_32(uWord);
            }

            AIL_file_read(pStream->fileHandle, &uChunkEnd, 4);
            iPosition = AIL_file_seek(pStream->fileHandle, 0, SEEK_FROM_CURRENT);
            uChunkEnd = LITTLE_ENDIAN_32(uChunkEnd) + iPosition;
            AIL_file_read(pStream->fileHandle, &uWord, 4);
            uFormatTag = LITTLE_ENDIAN_HIGH_16(uWord);
            uChannels = LITTLE_ENDIAN_LOW_16(uWord);
            AIL_file_read(pStream->fileHandle, &uSampleRate, 4);
            uSampleRate = LITTLE_ENDIAN_32(uSampleRate);
            AIL_file_read(pStream->fileHandle, &uWord, 4);
            AIL_file_read(pStream->fileHandle, &uWord, 4);
            pStream->blockSize = LITTLE_ENDIAN_HIGH_16(uWord);
            uBits = LITTLE_ENDIAN_LOW_16(uWord);
            if (uFormatTag == WAVE_FORMAT_EXTENSIBLE)
            {
                if (AIL_file_read(pStream->fileHandle, &uWord, 4) != 4)
                {
                    break;
                }

                uExtensionSize = LITTLE_ENDIAN_HIGH_16(uWord);
                if (uExtensionSize <= EXTENSIBLE_MINIMUM_SIZE)
                {
                    break;
                }

                if (AIL_file_read(pStream->fileHandle, &uChannelMask, 4) != 4)
                {
                    break;
                }

                uChannelMask = LITTLE_ENDIAN_32(uChannelMask);
                if (AIL_file_read(pStream->fileHandle, formatGuid, GUID_SIZE) != GUID_SIZE)
                {
                    break;
                }
            }
            else
            {
                uChannelMask = 0xFFFFFFFF;
            }

            AIL_file_seek(pStream->fileHandle, uChunkEnd, SEEK_FROM_START);
            AIL_file_read(pStream->fileHandle, &uWord, 4);
            uChunk = LITTLE_ENDIAN_32(uWord);
            while (uChunk != DATA_TAG)
            {
                if (AIL_file_read(pStream->fileHandle, &uWord, 4) != 4)
                {
                    break;
                }

                uWord = (LITTLE_ENDIAN_32(uWord) + 1) & 0xFFFFFFFE;
                AIL_file_seek(pStream->fileHandle, uWord, SEEK_FROM_CURRENT);
                AIL_file_read(pStream->fileHandle, &uWord, 4);
                uChunk = LITTLE_ENDIAN_32(uWord);
            }

            AIL_file_read(pStream->fileHandle, &pStream->dataLength, 4);
            pStream->dataLength = LITTLE_ENDIAN_32(pStream->dataLength);
            pStream->dataStart = AIL_file_seek(pStream->fileHandle, 0, SEEK_FROM_CURRENT);
            pStream->sampleRate = uSampleRate;
            pStream->dataRate = uSampleRate * uBits * uChannels >> 3;
            if (uBits != 8 && uBits != 16)
            {
                pStream->blockOriented = 1;
            }

            if (uFormatTag != WAVE_FORMAT_PCM && uFormatTag != WAVE_FORMAT_EXTENSIBLE
                && (uFormatTag != WAVE_FORMAT_IMA_ADPCM || uBits != 4))
            {
                uProvider = RIB_find_file_dec_provider(ASI_CODEC_INTERFACE, INPUT_WAVE_TAG, uFormatTag,
                                                       OUTPUT_FILE_TYPES, RAW_FILE_TYPE);
                if (uProvider != 0)
                {
                    bScan = true;
                }

                break;
            }

            pStream->channelMask = uChannelMask;
            if (uBits == 4)
            {
                uFormat = FORMAT_ADPCM_MONO_16;
                pStream->dataLength = pStream->dataLength & ~(uChannels * 4 - 1);
            }
            else
            {
                uBlockUnit = uBits * uChannels >> 3;
                pStream->dataLength = pStream->dataLength & ~(uBlockUnit - 1);
                if (uBits == 16)
                {
                    uFormat = FORMAT_16_BIT;
                }
                else if (uBits == 8)
                {
                    uFormat = 0;
                }
                else
                {
                    break;
                }
            }

            if (uChannels > 2)
            {
                pStream->format = uFormat | uChannels << FORMAT_CHANNEL_SHIFT | FORMAT_MULTICHANNEL;
                int iByte = 0;
                while (iByte < GUID_SIZE && formatGuid[iByte] == pcmGuid[iByte])
                {
                    ++iByte;
                }

                if (iByte != GUID_SIZE)
                {
                    break;
                }
            }
            else if (uChannels > 1)
            {
                pStream->format = uFormat | FORMAT_STEREO;
            }
            else
            {
                pStream->format = uFormat;
            }

            AIL_set_sample_playback_rate(pStream->sample, pStream->sampleRate);
            SS_set_sample_type(pStream->sample, pStream->format, pStream->channelMask);
            AIL_set_sample_adpcm_block_size(pStream->sample, pStream->blockSize);
            InMilesMutex();
            bParsed = true;
        } while (false);
    }
    else
    {
        pStream->dataStart = AIL_file_seek(pStream->fileHandle, 0, SEEK_FROM_CURRENT);
        iPosition = AIL_file_seek(pStream->fileHandle, 0, SEEK_FROM_END);
        pStream->dataLength = iPosition - pStream->dataStart;
        AIL_file_seek(pStream->fileHandle, pStream->dataStart, SEEK_FROM_START);
        bScan = true;
    }

    if (bScan)
    {
        while (bId3First || (iBytes = AIL_file_read(pStream->fileHandle, probe, ASI_PROBE_SIZE)) != 0)
        {
            if (!bId3First)
            {
                AIL_file_seek(pStream->fileHandle, pStream->dataStart, SEEK_FROM_START);
                AIL_set_sample_address(pStream->sample, probe, iBytes);
                if (AIL_set_sample_processor(pStream->sample, DECODER_STAGE, uProvider) == -1)
                {
                    break;
                }

                pDecoder = &pStream->sample->stages[DECODER_STAGE].decoder;
                pStream->asi = pDecoder;
                if (pDecoder->stream != NULL)
                {
                    pStream->blockSize = 1;
                    pDecoder->property(pDecoder->stream, pDecoder->minimumInputBlockSize, &pStream->blockSize, NULL,
                                       NULL);
                    AIL_file_seek(pStream->fileHandle, pStream->dataStart, SEEK_FROM_START);
                    if (pStream->asi->dataOffset != PROPERTY_NOT_FOUND)
                    {
                        pStream->dataStart = -1;
                        pStream->asi->property(pStream->asi->stream, pStream->asi->dataOffset, &pStream->dataStart,
                                               NULL, NULL);
                        AIL_file_seek(pStream->fileHandle, pStream->dataStart, SEEK_FROM_START);
                    }

                    if (pStream->asi->dataLength != PROPERTY_NOT_FOUND)
                    {
                        pStream->dataLength = -1;
                        pStream->asi->property(pStream->asi->stream, pStream->asi->dataLength, &pStream->dataLength,
                                               NULL, NULL);
                    }

                    pStream->asi->seek(pStream->asi->stream, 0);
                    if (pStream->asi->streamSeekPosition != PROPERTY_NOT_FOUND)
                    {
                        iSeekExtra = 0;
                        pStream->asi->property(pStream->asi->stream, pStream->asi->streamSeekPosition, NULL,
                                               &iSeekExtra, &iSeekExtra);
                        if (iSeekExtra != 0)
                        {
                            AIL_file_seek(pStream->fileHandle, pStream->dataStart + iSeekExtra, SEEK_FROM_START);
                        }
                    }

                    pStream->sampleRate = -1;
                    pStream->asi->property(pStream->asi->stream, pStream->asi->outputSampleRate, &pStream->sampleRate,
                                           NULL, NULL);
                    pStream->dataRate = -1;
                    pStream->asi->property(pStream->asi->stream, pStream->asi->inputBitRate, &pStream->dataRate, NULL,
                                           NULL);
                    pStream->dataRate = (unsigned int)pStream->dataRate >> 3;
                    uAsiChannels = 0xFFFFFFFF;
                    pStream->asi->property(pStream->asi->stream, pStream->asi->outputChannels, &uAsiChannels, NULL,
                                           NULL);
                    iAsiBits = -1;
                    pStream->asi->property(pStream->asi->stream, pStream->asi->outputSampleWidth, &iAsiBits, NULL,
                                           NULL);
                    iAsiMask = -1;
                    if (pStream->asi->outputChannelMask != PROPERTY_NOT_FOUND)
                    {
                        pStream->asi->property(pStream->asi->stream, pStream->asi->outputChannelMask, &iAsiMask, NULL,
                                               NULL);
                    }

                    pStream->sample->buffers[0].position = 0;
                    pStream->sample->buffers[0].size = 0;
                    pStream->sample->buffers[1].position = 0;
                    pStream->sample->buffers[1].size = 0;
                    if (pStream->dataRate != 0 && uAsiChannels != 0)
                    {
                        if (iAsiBits == 16)
                        {
                            pStream->format = FORMAT_16_BIT;
                        }
                        else if (iAsiBits == 8)
                        {
                            pStream->format = 0;
                        }

                        if (uAsiChannels == 2)
                        {
                            pStream->usingASI = 1;
                            pStream->blockOriented = 1;
                            pStream->format = pStream->format | FORMAT_STEREO;
                        }
                        else
                        {
                            if (uAsiChannels > 2)
                            {
                                pStream->format = pStream->format | uAsiChannels << FORMAT_CHANNEL_SHIFT
                                                  | FORMAT_MULTICHANNEL;
                                pStream->channelMask = iAsiMask;
                            }

                            pStream->usingASI = 1;
                            pStream->blockOriented = 1;
                        }

                        InMilesMutex();
                        bParsed = true;
                        break;
                    }
                }

                if (bId3Checked)
                {
                    break;
                }
            }

            bId3First = false;
            bId3Checked = true;
            AIL_file_seek(pStream->fileHandle, pStream->dataStart, SEEK_FROM_START);
            iBytes = AIL_file_read(pStream->fileHandle, id3Header, ID3_HEADER_SIZE);
            if (iBytes == 0 || id3Header[0] != 'I' || id3Header[1] != 'D' || id3Header[2] != '3'
                || id3Header[3] > ID3_MAXIMUM_VERSION_BYTE || id3Header[4] > ID3_MAXIMUM_VERSION_BYTE
                || id3Header[6] > ID3_MAXIMUM_SIZE_BYTE || id3Header[7] > ID3_MAXIMUM_SIZE_BYTE
                || id3Header[8] > ID3_MAXIMUM_SIZE_BYTE || id3Header[9] > ID3_MAXIMUM_SIZE_BYTE)
            {
                break;
            }

            iPosition = (id3Header[9] | id3Header[8] << 7 | id3Header[7] << 14 | id3Header[6] << 21) + ID3_TAG_OVERHEAD;
            pStream->dataStart = iPosition;
            pStream->dataLength = pStream->dataLength - iPosition;
            AIL_file_seek(pStream->fileHandle, iPosition, SEEK_FROM_START);
        }
    }

    if (!bParsed)
    {
        InMilesMutex();
        AIL_set_error(SOUND_FORMAT_ERROR);
        AIL_release_sample_handle(pStream->sample);
        if (pStream->fileHandle != STREAM_NO_FILE)
        {
            AIL_file_close(pStream->fileHandle);
        }

        AIL_mem_free_lock(pStream);
        return NULL;
    }

    if (iStreamMemory == 0)
    {
        pStream->bufferSize = (unsigned int)pStream->dataRate >> 3;
    }
    else
    {
        pStream->bufferSize = iStreamMemory / 8;
    }

    iMinimum = AIL_minimum_sample_buffer_size(pDriver, pStream->sampleRate, pStream->format);
    iBufferSize = pStream->bufferSize;
    if (pStream->bufferSize < iMinimum)
    {
        pStream->bufferSize = iMinimum;
        iBufferSize = iMinimum;
    }

    if (pStream->usingASI != 0)
    {
        uMinimumToken = 0;
        minimumEntry.uType = RIB_PROPERTY_ENTRY;
        minimumEntry.pEntryName = MINIMUM_INPUT_STREAM_SIZE;
        minimumEntry.pToken = &uMinimumToken;
        minimumEntry.uSubType = 0;
        RIB_request_interface(pStream->sample->stages[DECODER_STAGE].decoder.provider, ASI_STREAM_INTERFACE, 1,
                              &minimumEntry);
        iAsiMinimum = 0;
        pStream->asi->property(pStream->asi->stream, uMinimumToken, &iAsiMinimum, NULL, NULL);
        iBufferSize = pStream->bufferSize;
        if (pStream->bufferSize < iAsiMinimum)
        {
            iBufferSize = iAsiMinimum;
        }
    }

    pStream->bufferSize = (iBufferSize + BUFFER_ALIGNMENT - 1) & ~(BUFFER_ALIGNMENT - 1);
    if (pStream->blockOriented == 0)
    {
        int iSampleBytes = 1;
        if ((pStream->format & FORMAT_16_BIT) != 0)
        {
            iSampleBytes = 2;
        }

        unsigned int uUnit = iSampleBytes * pStream->sample->channelCount;
        pStream->bufferSize = (unsigned int)pStream->bufferSize / uUnit * uUnit;
    }

    iBufferSize = pStream->dataLength;
    if (pStream->bufferSize == 0)
    {
        pStream->bufferSize = BUFFER_ALIGNMENT;
    }

    if ((int)(iBufferSize - pStream->bufferSize * 3) < MINIMUM_STREAM_SLACK)
    {
        pStream->bufferSize = iBufferSize;
        pStream->startThreshold = iBufferSize;
        pStream->buffers[0] = static_cast<unsigned char*>(
            AIL_mem_alloc_lock_info(iBufferSize, MILES_STREAM_FILE, STREAM_PRELOAD_LINE));
        if (pStream->buffers[0] == NULL)
        {
            AIL_set_error(OUT_OF_MEMORY_ERROR);
            AIL_release_sample_handle(pStream->sample);
            if (pStream->fileHandle != STREAM_NO_FILE)
            {
                AIL_file_close(pStream->fileHandle);
            }

            AIL_mem_free_lock(pStream);
            return NULL;
        }

        pStream->preload = 1;
    }
    else
    {
        pStream->startThreshold = iMinimum << 1;
        if (pStream->startThreshold < pStream->bufferSize * 7)
        {
            pStream->startThreshold = pStream->bufferSize * 7;
        }

        pStream->buffers[0] = static_cast<unsigned char*>(
            AIL_mem_alloc_lock_info(pStream->bufferSize * STREAM_BUFFER_COUNT + BUFFER_PADDING * STREAM_BUFFER_COUNT,
                                    MILES_STREAM_FILE, STREAM_BUFFERS_LINE));
        if (pStream->buffers[0] == NULL)
        {
            AIL_set_error(OUT_OF_MEMORY_ERROR);
            AIL_release_sample_handle(pStream->sample);
            if (pStream->fileHandle != STREAM_NO_FILE)
            {
                AIL_file_close(pStream->fileHandle);
            }

            AIL_mem_free_lock(pStream);
            return NULL;
        }

        for (int iIndex = 1; iIndex < STREAM_BUFFER_COUNT; ++iIndex)
        {
            pStream->buffers[iIndex] = pStream->buffers[iIndex - 1] + pStream->bufferSize + BUFFER_PADDING;
        }
    }

    pStream->sample->stream = pStream;
    AIL_set_sample_buffer_count(pStream->sample, STREAM_BUFFER_COUNT);
    pStream->loopEnd = UNLIMITED_LOOP_END;
    pStream->loopStart = 0;
    pStream->loopCount = 1;
    pStream->startRemaining = pStream->startThreshold;
    pStream->ioState = STREAM_IO_WAITING;
    pStream->reportedPercent = PERCENT_UNSET;
    pStream->percentScale = 1.0f / (float)(pStream->bufferSize << 3);
    pStream->id = StreamCounter;
    StreamCounter = StreamCounter + 1;
    if (StreamList != NULL)
    {
        pStream->next = StreamList;
        StreamList = pStream;
        return pStream;
    }

    StreamTimer = AIL_register_timer(background);
    if (StreamTimer != -1)
    {
        AIL_set_timer_frequency(StreamTimer, STREAM_TIMER_FREQUENCY);
        AIL_start_timer(StreamTimer);
        StreamList = pStream;
        return pStream;
    }

    AIL_set_error(TIMER_HANDLE_ERROR);
    AIL_mem_free_lock(pStream->buffers[0]);
    AIL_release_sample_handle(pStream->sample);
    if (pStream->fileHandle != STREAM_NO_FILE)
    {
        AIL_file_close(pStream->fileHandle);
    }

    AIL_mem_free_lock(pStream);
    return NULL;
}

extern "C" void AIL_API_close_stream(_STREAM* pStream)
{
    if (pStream == NULL)
    {
        return;
    }

    InMilesMutex();
    if (pStream->sample != NULL)
    {
        _SAMPLE* pSample = pStream->sample;
        pStream->sample = NULL;
        AIL_release_sample_handle(pSample);
    }

    if (StreamList == pStream)
    {
        StreamList = StreamList->next;
    }
    else
    {
        for (_STREAM* pCurrent = StreamList; pCurrent != NULL; pCurrent = pCurrent->next)
        {
            if (pCurrent->next == pStream)
            {
                pCurrent->next = pStream->next;
                break;
            }
        }
    }

    if (StreamList == NULL)
    {
        AIL_release_timer_handle(StreamTimer);
        StreamTimer = 0;
    }

    OutMilesMutex();
    cancel_asyncs(pStream);
    handle_asyncs(pStream, -1);
    if (pStream->buffers[0] != NULL)
    {
        AIL_mem_free_lock(pStream->buffers[0]);
    }

    if (pStream->fileHandle != STREAM_NO_FILE)
    {
        AIL_file_close(pStream->fileHandle);
    }

    memset(pStream, 0, sizeof(_STREAM));
    AIL_mem_free_lock(pStream);
}

extern "C" int AIL_API_stream_status(_STREAM* pStream)
{
    if (pStream != NULL && pStream->error == 0)
    {
        if (pStream->paused == 0)
        {
            if ((pStream->playStatus & SAMPLE_STATUS_STOPPED) != 0 || pStream->playStatus == 0)
            {
                return SAMPLE_STATUS_STOPPED;
            }

            if (pStream->endOfData == 0 || AIL_API_sample_loaded_len(pStream->sample) != 0
                || ((pStream->bufferLength[pStream->loadBuffer] != 0 || pStream->endOfData == STREAM_END_PENDING)
                    && pStream->playStatus == STREAM_PLAY_FLAG_PLAYING
                    && pStream->readStatus[pStream->loadBuffer] != 0))
            {
                return SAMPLE_STATUS_PLAYING;
            }
        }

        return SAMPLE_STATUS_DONE;
    }

    return -1;
}

int stream_align(_STREAM* pStream, int iPosition)
{
    int iAligned = iPosition;
    if (pStream->usingASI == 0)
    {
        if (pStream->blockOriented != 0)
        {
            unsigned int uBlock = pStream->blockSize;
            iAligned = iPosition / uBlock * uBlock;
            if (iPosition - iAligned > (int)uBlock / 2)
            {
                iAligned = iAligned + uBlock;
            }

            if (iAligned > pStream->dataLength)
            {
                return pStream->dataLength;
            }

            return iAligned;
        }
        else
        {
            int iUnit = 1;
            if ((pStream->format & FORMAT_16_BIT) != 0)
            {
                iUnit = 2;
            }

            iUnit = iUnit * pStream->sample->channelCount;
            iAligned = iPosition - iPosition % iUnit;
        }
    }

    if (iAligned > pStream->dataLength)
    {
        iAligned = pStream->dataLength;
    }

    return iAligned;
}

extern "C" void AIL_API_set_stream_position(_STREAM* pStream, int iPosition)
{
    if (pStream == NULL)
    {
        return;
    }

    iPosition = stream_align(pStream, iPosition);
    if (iPosition == AIL_stream_position(pStream))
    {
        return;
    }

    if (pStream->usingASI != 0)
    {
        ASISTAGE* pAsi = pStream->asi;
        if (pAsi->streamSeekPosition != PROPERTY_NOT_FOUND)
        {
            pAsi->property(pAsi->stream, pAsi->streamSeekPosition, NULL, &iPosition, &iPosition);
        }
    }

    if (pStream->preload != 0)
    {
        if (pStream->startRemaining != pStream->startThreshold)
        {
            AIL_set_sample_position(pStream->sample, iPosition);
            return;
        }

        pStream->preloadSeek = iPosition;
        return;
    }

    pStream->sample->resetB = 0;
    pStream->sample->resetA = 0;
    int iBuffer = pStream->sample->currentBuffer;
    if (pStream->sample->currentBuffer != pStream->sample->lastBuffer || pStream->sample->starved == 0)
    {
        do
        {
            if (pStream->bufferStart[iBuffer] <= iPosition
                && iPosition - pStream->bufferStart[iBuffer] < pStream->sample->buffers[iBuffer].size)
            {
                while (pStream->sample->currentBuffer != iBuffer)
                {
                    pStream->sample->buffers[pStream->sample->currentBuffer].position = SILENCE_LENGTH;
                    pStream->sample->buffers[pStream->sample->currentBuffer].size = SILENCE_LENGTH;
                    pStream->readStatus[pStream->sample->currentBuffer] = 0;
                    pStream->bufferLength[pStream->sample->currentBuffer] = 0;
                    pStream->resetASI[pStream->sample->currentBuffer] = 0;
                    pStream->resetPosition[pStream->sample->currentBuffer] = 0;
                    pStream->bufferStart[pStream->sample->currentBuffer] = 0;
                    pStream->sample->currentBuffer = pStream->sample->currentBuffer + 1;
                    if (pStream->sample->currentBuffer >= pStream->sample->bufferCount)
                    {
                        pStream->sample->currentBuffer = 0;
                    }
                }

                pStream->sample->buffers[iBuffer].position = iPosition - pStream->bufferStart[iBuffer];
                if (pStream->usingASI != 0)
                {
                    pStream->asi->seek(pStream->asi->stream, -1);
                }

                return;
            }

            iBuffer = iBuffer + 1;
            if (iBuffer >= pStream->sample->bufferCount)
            {
                iBuffer = 0;
            }
        } while (iBuffer != pStream->sample->lastBuffer);
    }

    cancel_asyncs(pStream);
    handle_asyncs(pStream, -1);
    if (pStream->startRemaining != pStream->startThreshold)
    {
        AIL_set_sample_buffer_count(pStream->sample, pStream->sample->bufferCount);
        if (pStream->usingASI != 0)
        {
            pStream->asi->seek(pStream->asi->stream, -1);
        }
    }

    pStream->readBuffer = 0;
    pStream->fillPercent = 0.0f;
    pStream->startRemaining = pStream->startThreshold;
    pStream->seekOffset = pStream->dataStart + iPosition;
    pStream->readPosition = iPosition;
    pStream->reportedPercent = PERCENT_UNSET;
    pStream->loadBuffer = 0;
    for (int iIndex = 0; iIndex < STREAM_BUFFER_COUNT; ++iIndex)
    {
        pStream->readStatus[iIndex] = 0;
        pStream->bufferLength[iIndex] = 0;
        pStream->resetASI[iIndex] = 0;
        pStream->resetPosition[iIndex] = 0;
        pStream->bufferStart[iIndex] = iPosition;
    }

    if (pStream->playStatus != STREAM_PLAY_FLAG_PLAYING)
    {
        pStream->playStatus = 0;
    }

    pStream->endOfData = 0;
}

extern "C" int AIL_API_stream_position(_STREAM* pStream)
{
    if (pStream != NULL)
    {
        int iPosition = AIL_sample_position(pStream->sample);
        if (pStream->preload == 0)
        {
            iPosition = iPosition + pStream->bufferStart[pStream->sample->currentBuffer];
        }

        if (iPosition > pStream->dataLength)
        {
            iPosition = pStream->dataLength;
        }

        return iPosition;
    }

    return -1;
}

extern "C" void AIL_API_start_stream(_STREAM* pStream)
{
    if (pStream == NULL)
    {
        return;
    }

    if (pStream->ioState == STREAM_IO_WAITING)
    {
        pStream->ioState = STREAM_IO_ACTIVE;
    }

    AIL_request_EOB_ASI_reset(pStream->sample, 0xFFFFFFFF, 0);
    AIL_set_stream_position(pStream, 0);
    if (pStream->preload != 0 && pStream->preloadLoaded == 1)
    {
        AIL_start_sample(pStream->sample);
    }

    pStream->playStatus = STREAM_PLAY_FLAG_PLAYING;
}

extern "C" void AIL_API_pause_stream(_STREAM* pStream, int iPause)
{
    if (pStream == NULL)
    {
        return;
    }

    if (iPause != 0)
    {
        if (AIL_stream_status(pStream) == (int)SAMPLE_STATUS_PLAYING)
        {
            AIL_stop_sample(pStream->sample);
            pStream->playStatus = pStream->playStatus | STREAM_PLAY_PAUSE_BITS;
            return;
        }

        pStream->playStatus = pStream->playStatus | STREAM_PLAY_PAUSED;
        return;
    }

    if ((pStream->playStatus & STREAM_PLAY_MASK) == 0)
    {
        if (pStream->ioState == STREAM_IO_WAITING)
        {
            pStream->ioState = STREAM_IO_ACTIVE;
        }

        AIL_request_EOB_ASI_reset(pStream->sample, 0xFFFFFFFF, 0);
        if (pStream->preload != 0 && pStream->preloadLoaded == 1)
        {
            AIL_start_sample(pStream->sample);
        }

        pStream->playStatus = STREAM_PLAY_FLAG_PLAYING;
        return;
    }

    if ((pStream->playStatus & STREAM_PLAY_SAMPLE_STOPPED) != 0)
    {
        AIL_resume_sample(pStream->sample);
    }

    pStream->playStatus = pStream->playStatus & ~STREAM_PLAY_PAUSE_BITS;
    background(0);
}

extern "C" void AIL_API_set_stream_loop_count(_STREAM* pStream, int iLoopCount)
{
    if (pStream == NULL)
    {
        return;
    }

    if (pStream->preload != 0)
    {
        AIL_set_sample_loop_count(pStream->sample, iLoopCount);
        return;
    }

    pStream->loopCount = iLoopCount;
}

extern "C" void AIL_API_set_stream_loop_block(_STREAM* pStream, int iLoopStart, int iLoopEnd)
{
    if (pStream == NULL)
    {
        return;
    }

    if (pStream->preload != 0)
    {
        AIL_set_sample_loop_block(pStream->sample, iLoopStart, iLoopEnd);
        return;
    }

    if (iLoopStart == 0 && iLoopEnd == -1)
    {
        pStream->loopStart = 0;
        pStream->loopEnd = UNLIMITED_LOOP_END;
        return;
    }

    if (iLoopStart != -2)
    {
        pStream->loopStart = stream_align(pStream, iLoopStart);
    }

    if (iLoopEnd == -1)
    {
        iLoopEnd = pStream->dataLength;
    }

    if (iLoopEnd != -2)
    {
        pStream->loopEnd = stream_align(pStream, iLoopEnd);
    }
}

extern "C" void AIL_API_set_stream_ms_position(_STREAM* pStream, unsigned int uMilliseconds)
{
    int iPosition;
    int iLocation;

    if (pStream != NULL)
    {
        if ((pStream->format & FORMAT_ADPCM) != 0)
        {
            unsigned int uHeader = 4 << ((pStream->format >> 1) & 1);
            iPosition = (int)(((float)((unsigned int)(pStream->sampleRate * pStream->blockSize)
                                       / (((pStream->blockSize - uHeader) * 8) / uHeader + 1))
                               * (float)(int)uMilliseconds)
                              / MILLISECONDS_PER_SECOND);
        }
        else
        {
            DecoderStage& decoder = pStream->sample->stages[DECODER_STAGE].decoder;
            if (decoder.active != 0 && decoder.exactSeek != 0)
            {
                iLocation = 0;
                if (decoder.property(decoder.stream, decoder.exactSeek, NULL, &uMilliseconds, &iLocation) == 1)
                {
                    AIL_set_stream_position(pStream, iLocation);
                    return;
                }
            }

            if (pStream->blockOriented != 0)
            {
                iPosition = (int)(((float)(unsigned int)pStream->dataRate * (float)(int)uMilliseconds)
                                  / MILLISECONDS_PER_SECOND);
            }
            else
            {
                int iSampleBytes = 1;
                if ((pStream->format & FORMAT_16_BIT) != 0)
                {
                    iSampleBytes = 2;
                }

                iPosition =
                    (int)(((float)(unsigned int)(pStream->sampleRate * pStream->sample->channelCount * iSampleBytes)
                           * (float)(int)uMilliseconds)
                          / MILLISECONDS_PER_SECOND);
            }
        }

        AIL_set_stream_position(pStream, iPosition);
    }
}

extern "C" void AILSTRM_shutdown(_DIG_DRIVER* pDriver)
{
    _STREAM* pNext = StreamList;
    while (pNext != NULL)
    {
        _STREAM* pStream = pNext;
        _SAMPLE* pSample = pNext->sample;
        pNext = pNext->next;
        if (pSample != NULL && pSample->driver == pDriver)
        {
            pStream->sample = NULL;
            AIL_close_stream(pStream);
        }
    }
}
