#ifndef SOURCE_VENDOR_MILES_STREAM_HPP
#define SOURCE_VENDOR_MILES_STREAM_HPP

#include "game/client/audio/FalloffGraph.hpp"
#include "vendor/MilesAsync.hpp"

struct _STREAM;

typedef void (*AIL_STREAM_CALLBACK)(_STREAM* pStream);

const int STREAM_BUFFER_COUNT = 8;
const unsigned int STREAM_NO_FILE = 0xFFFFFFFF;

struct _STREAM
{
    int blockOriented;
    int usingASI;
    ASISTAGE* asi;
    _SAMPLE* sample;
    unsigned int fileHandle;
    unsigned char* buffers[STREAM_BUFFER_COUNT];
    int resetASI[STREAM_BUFFER_COUNT];
    int resetPosition[STREAM_BUFFER_COUNT];
    int bufferStart[STREAM_BUFFER_COUNT];
    int bufferLength[STREAM_BUFFER_COUNT];
    MilesAsyncRead reads[STREAM_BUFFER_COUNT];
    int readStatus[STREAM_BUFFER_COUNT];
    int seekOffset;
    int loadBuffer;
    int readBuffer;
    int bufferSize;
    int dataRate;
    int sampleRate;
    unsigned int format;
    int channelMask;
    int dataLength;
    int loopStart;
    int loopEnd;
    int blockSize;
    unsigned char reserved1048[4];
    int dataStart;
    int readPosition;
    int loopCount;
    int error;
    int preload;
    int preloadSeek;
    int preloadLoaded;
    int endOfData;
    int startThreshold;
    int startRemaining;
    unsigned int playStatus;
    AIL_STREAM_CALLBACK callback;
    int userData[8];
    _STREAM* next;
    int ioState;
    float fillPercent;
    float reportedPercent;
    float percentScale;
    int id;
    int callbackPending;
    int paused;
    unsigned char reserved10bc[4];
};

void handle_asyncs(_STREAM* pStream, int iWait);
void cancel_asyncs(_STREAM* pStream);
float MilesStreamCB(void* pUserData);
int start_IOs_if_we_can(_STREAM* pStream);
void background(unsigned int uTimer);
int stream_align(_STREAM* pStream, int iPosition);

extern "C" float AIL_API_stream_filled_percent(_STREAM* pStream);
extern "C" _STREAM* AIL_API_open_stream(_DIG_DRIVER* pDriver, const char* pFileName, int iStreamMemory);
extern "C" void AIL_API_close_stream(_STREAM* pStream);
extern "C" int AIL_API_stream_status(_STREAM* pStream);
extern "C" void AIL_API_set_stream_position(_STREAM* pStream, int iPosition);
extern "C" int AIL_API_stream_position(_STREAM* pStream);
extern "C" void AIL_API_start_stream(_STREAM* pStream);
extern "C" void AIL_API_pause_stream(_STREAM* pStream, int iPause);
extern "C" void AIL_API_set_stream_loop_count(_STREAM* pStream, int iLoopCount);
extern "C" void AIL_API_set_stream_loop_block(_STREAM* pStream, int iLoopStart, int iLoopEnd);
extern "C" void AIL_API_set_stream_ms_position(_STREAM* pStream, unsigned int uMilliseconds);
extern "C" void AILSTRM_shutdown(_DIG_DRIVER* pDriver);

#endif
