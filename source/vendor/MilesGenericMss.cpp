#include "vendor/MilesGeneric.hpp"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "vendor/MilesCoreApi.hpp"
#include "vendor/MilesFilters.hpp"
#include "vendor/MilesMixer.hpp"
#include "vendor/MilesRR.hpp"
#include "vendor/MilesRadSS.hpp"
#include "vendor/MilesRib.hpp"
#include "vendor/MilesTime.hpp"

typedef int (*MSS_PROVIDER_STARTUP_CALLBACK)();

extern "C" void* platform_alloc(unsigned int uSize);
extern "C" void platform_free(void* pBlock);

struct MilesTimer
{
    AIL_TIMER_CALLBACK pCallback;
    unsigned int uReserved4;
    unsigned long long uNextTime;
    unsigned long long uPeriod;
    unsigned int uUser;
    int iStatus;
};

static const int DIG_MAX_CHAIN_ELEMENT_SIZE_DEFAULT = 8192;
static const int DIG_OUTPUT_BUFFER_SIZE_DEFAULT = 49152;
static const int MDI_SYSEX_BUFFER_SIZE_DEFAULT = 1536;
static const int DLS_ADPCM_TO_ASI_THRESHOLD_DEFAULT = 32768;

static const int MILES_PREFERENCE_TIMER_WAIT = 0;
static const int MILES_PREFERENCE_TIMER_COUNT = 1;
static const int MILES_PREFERENCE_SAMPLE_COUNT = 3;
static const int MILES_PREFERENCE_SERVICE_MILLISECONDS = 6;
static const int MILES_PREFERENCE_BUFFER_COUNT = 7;
static const int MILES_PREFERENCE_BUFFER_RESERVE = 8;
static const int MILES_PREFERENCE_DRIVER_FIELD = 11;

static const int MILES_TIMER_INVALID = -1;
static const int MILES_TIMER_FREE = 0;
static const int MILES_TIMER_ALLOCATED = 1;
static const int MILES_TIMER_RUNNING = 2;
static const unsigned int MILES_TIMER_DEFAULT_PERIOD_MILLISECONDS = 10;
static const unsigned int MILES_TIMER_THREAD_BLOCK_SIZE = 0x14100;
static const int MILES_TIMER_THREAD_STACK_SIZE = 0x14000;
static const unsigned int MILES_TIMER_THREAD_FLAGS = 8;
static const unsigned int MILES_TIMER_THREAD_PRIORITY = 0x10;
static const unsigned int MILES_TIMER_THREAD_CORE = 1;
static const unsigned int MILES_TIMER_LOCK_WAIT_MILLISECONDS = 10;
static const unsigned int MILES_TIMER_SEMAPHORE_MAXIMUM = 1;
static const unsigned int MILES_WAIT_INFINITE = 0xFFFFFFFF;
static const unsigned int MILES_MICROSECONDS_PER_SECOND = 1000000;
static const unsigned int MILES_ENUMERATE_FIRST = 0xFFFFFFFF;

static const int MSS_PROVIDER_ALREADY_STARTED = 2;
static const int MILES_MUTEX_KIND = 2;

static const unsigned int MILES_LOCK_STATE_RELEASED = 1;
static const unsigned int MILES_LOCK_STATE_ACQUIRED = 2;
static const unsigned int MILES_ZONE_FLAGS_NONE = 0;
static const unsigned int MILES_ZONE_FLAGS_BLOCKED = 2;

static const int MILES_MAXIMUM_CHANNELS = 8;
static const int MILES_DEFAULT_SPEAKER_LEVEL_COUNT = 6;
static const unsigned int MILES_SOFTWARE_BUFFER_SAMPLES = 0x4000;
static const unsigned int MILES_DEVICE_FLAGS_SOFTWARE = 0x20;
static const unsigned int MILES_DEVICE_FLAGS_MONO = 1;
static const unsigned int MILES_DEVICE_FLAGS_STEREO = 3;
static const unsigned int MILES_DEVICE_FLAGS_MULTICHANNEL = 0x11;
static const unsigned int MILES_OPEN_START_PLAYBACK = 0x80;
static const int MILES_SAMPLE_STATUS_FREE = 1;
static const unsigned int MILES_SAMPLE_BUFFER_SAMPLE_SCALE = 0x3E800;
#ifdef MINECRAFT_PC
static const unsigned int MILES_RADSS_ALLOCATION_SIZE = RADSS_GENERIC_SIZE;
#else
static const unsigned int MILES_RADSS_ALLOCATION_SIZE = 0xB8;
#endif
static const unsigned int MILES_SAMPLE_ALIGNMENT_MASK = 0xFFFFFFF0;
static const unsigned int MILES_SAMPLE_ALIGNMENT_ROUND = 0xF;
static const unsigned int MILES_MILLISECONDS_PER_SECOND = 1000;
static const unsigned int MILES_SCRATCH_PADDING = 0x10;
static const int MILES_BUILD_BUFFER_SAMPLE_BYTES = 4;
static const int MILES_REVERB_BUILD_CHANNELS = 2;
static const int MILES_REVERB_BUILD_SAMPLE_BYTES = 8;

static const float MILES_ROOT_HALF = 0.70710678f;
static const float MILES_NEGATIVE_ROOT_HALF = -0.70710678f;
static const float MILES_NEGATIVE_UNITY = -1.0f;
static const float MILES_SPEAKER_FALLOFF_POWER = 3.0f;
static const float MILES_UNITY = 1.0f;
static const float MILES_ZERO = 0.0f;

static const char MILES_GENERIC_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\genericmss.cpp";
static const char MILES_DRIVER_FILE[] = "C:\\devel\\projects\\mss\\src\\sdk\\shared\\genericdig.cpp";
static const char MILES_TIMER_THREAD_NAME[] = "Miles Timer";
static const char MILES_TIMER_WAIT_NAME[] = "Timer waiting";
static const char MILES_MUTEX_BLOCK_NAME[] = "Miles mutex block";
static const char MILES_MUTEX_NAME[] = "Miles mutex";
static const char MILES_MUTEX_LABEL[] = "Miles API mutex";
static const char MILES_BACKGROUND_NAME[] = "Miles background processing";
static const char MILES_TIMER_THREAD_LABEL[] = "MSSTimer";

static const char MILES_FILTER_INTERFACE[] = "MSS pipeline filter";
static const char MILES_FILTER_SHUTDOWN[] = "FLT_shutdown";
static const char MILES_CODEC_INTERFACE[] = "ASI codec";
static const char MILES_CODEC_SHUTDOWN[] = "ASI_shutdown";
static const char MILES_MIXER_INTERFACE[] = "MSS mixer services";
static const char MILES_MIXER_SHUTDOWN[] = "MIXER_shutdown";
static const char MILES_MIXER_LIBRARY[] = "Miles Mixer";
static const char MILES_MIXER_STARTUP[] = "MIXER_startup";
static const char MILES_CODEC_STARTUP[] = "ASI_startup";
static const char MILES_FILTER_STARTUP[] = "FLT_startup";

int MSS_api_started = 0;

_DIG_DRIVER* MSS_primary_digital_driver = NULL;

AIL_MEM_ALLOC_CALLBACK AIL_mem_alloc_handler = platform_alloc;
AIL_MEM_FREE_CALLBACK AIL_mem_free_handler = platform_free;

rrMutex MSS_api_mutex;
MilesProfileSite MSS_api_lock_wait_site;
MilesProfileSite MSS_api_lock_site;
MilesProfileSite MSS_api_unlock_site;
MilesProfileSite MSS_api_mutex_name_site;

char AIL_error[256];
int AIL_preference[37];

static int TimerBusy = 0;
static unsigned long long TimerLastTime = 0;
static unsigned long long TimerMaxInterval = 0;
static unsigned long long TimerTotalTime = 0;
static MilesTimer* TimerTable = NULL;
static int TimerCount = 0;
static int TimerThreadStarted = 0;
static rrThread* TimerThread;
static rrSemaphore TimerSemaphore;
static MilesProfileSite TimerThreadNameSite;
static MilesProfileSite TimerWaitSite;
static MilesProfileSite TimerMutexBlockSite;
static MilesProfileSite TimerLockSite;
static MilesProfileSite TimerBackgroundSite;
static MilesProfileSite TimerUnlockSite;

static int SS_serve_nesting = 0;

_DIG_DRIVER* MSS_first_digital_driver = NULL;

extern "C" void InMilesMutex()
{
    if (MSS_api_started == 0)
    {
        return;
    }

    if (rrMutexLockTimeout(&MSS_api_mutex, 0) == 0)
    {
        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, 1, MILES_GENERIC_FILE, 88, &MSS_api_lock_wait_site,
                                    MILES_MUTEX_BLOCK_NAME);
        }

        rrMutexLock(&MSS_api_mutex);

        if (MSS_profile == NULL)
        {
            return;
        }

        MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_GENERIC_FILE, 90);
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pLockState(MSS_profile, &MSS_api_mutex, MILES_LOCK_STATE_ACQUIRED, MILES_GENERIC_FILE, 92,
                                &MSS_api_lock_site, MILES_MUTEX_NAME);
    }
}

extern "C" void OutMilesMutex()
{
    if (MSS_api_started == 0)
    {
        return;
    }

    if (MSS_profile != NULL)
    {
        MSS_profile->pLockState(MSS_profile, &MSS_api_mutex, MILES_LOCK_STATE_RELEASED, MILES_GENERIC_FILE, 100,
                                &MSS_api_unlock_site, MILES_MUTEX_NAME);
    }

    rrMutexUnlock(&MSS_api_mutex);
}

extern "C" void AIL_API_startup()
{
    unsigned int uIndex;
    unsigned int uProvider;
    MSS_PROVIDER_STARTUP_CALLBACK pMixerStartup;
    MSS_PROVIDER_STARTUP_CALLBACK pCodecStartup;
    MSS_PROVIDER_STARTUP_CALLBACK pFilterStartup;
    int iResult;

    AIL_preference[2] = 1;
    AIL_preference[3] = 0x40;
    AIL_preference[5] = 0x83;
    AIL_preference[11] = 1;
    AIL_preference[14] = 0;
    AIL_preference[6] = 4;
    AIL_preference[10] = 500;
    AIL_preference[24] = 1;
    AIL_preference[15] = DIG_MAX_CHAIN_ELEMENT_SIZE_DEFAULT;
    AIL_preference[17] = 0;
    AIL_preference[20] = 0;
    AIL_preference[25] = 0;
    AIL_preference[22] = 0x78;
    AIL_preference[12] = 0;
    AIL_preference[28] = MDI_SYSEX_BUFFER_SIZE_DEFAULT;
    AIL_preference[21] = 8;
    AIL_preference[26] = 2;
    AIL_preference[1] = 0x10;
    AIL_preference[4] = 1;
    AIL_preference[18] = DIG_OUTPUT_BUFFER_SIZE_DEFAULT;
    AIL_preference[23] = 0x7F;
    AIL_preference[7] = 0x40;
    AIL_preference[19] = -1;
    AIL_preference[8] = 0xC;
    AIL_preference[13] = 0;
    AIL_preference[0] = 1;
    AIL_preference[16] = 100;
    AIL_preference[9] = 0x20;
    AIL_preference[27] = 0;
    AIL_preference[29] = 0x40;
    AIL_preference[30] = 0x78;
    AIL_preference[31] = 0;
    AIL_preference[32] = 1;
    AIL_preference[33] = 0;
    AIL_preference[34] = 1;
    AIL_preference[35] = 1;
    AIL_preference[36] = DLS_ADPCM_TO_ASI_THRESHOLD_DEFAULT;

    AIL_error[0] = '\0';

    rrGetTime();

    MSS_api_started = 1;
    rrMutexCreate(&MSS_api_mutex, MILES_MUTEX_KIND);

    if (MSS_profile != NULL)
    {
        MSS_profile->pLockName(MSS_profile, &MSS_api_mutex, &MSS_api_mutex_name_site, MILES_MUTEX_LABEL);
    }

    RIB_load_static_provider_library(MIX_RIB_MAIN, MILES_MIXER_LIBRARY);

    uIndex = MILES_ENUMERATE_FIRST;
    while (RIB_enumerate_providers(MILES_MIXER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_MIXER_INTERFACE, 0, MILES_MIXER_STARTUP, &pMixerStartup) == 0)
        {
            pMixerStartup();
        }
    }

    uIndex = MILES_ENUMERATE_FIRST;
    while (RIB_enumerate_providers(MILES_CODEC_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_CODEC_INTERFACE, 0, MILES_CODEC_STARTUP, &pCodecStartup) == 0)
        {
            iResult = pCodecStartup();
            if (iResult != 0 && iResult != MSS_PROVIDER_ALREADY_STARTED)
            {
                RIB_free_provider_library(uProvider);
            }
        }
    }

    FLT_init_list();

    uIndex = MILES_ENUMERATE_FIRST;
    while (RIB_enumerate_providers(MILES_FILTER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_FILTER_INTERFACE, 0, MILES_FILTER_STARTUP, &pFilterStartup)
            == 0)
        {
            iResult = pFilterStartup();
            if (iResult != 0 && iResult != MSS_PROVIDER_ALREADY_STARTED)
            {
                RIB_free_provider_library(uProvider);
            }
        }
    }

    MilesAsyncStartup();
}

static int ThreadProc(void*)
{
    unsigned long long uNow;
    unsigned long long uElapsed;
    int iTimer;

    if (MSS_profile != NULL)
    {
        MSS_profile->pThreadName(MSS_profile, 0, &TimerThreadNameSite, MILES_TIMER_THREAD_NAME);
    }

    for (;;)
    {
        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, MILES_ZONE_FLAGS_BLOCKED, MILES_GENERIC_FILE, 466,
                                    &TimerWaitSite, MILES_TIMER_WAIT_NAME);
        }

        if (rrSemaphoreDecrementOrWait(&TimerSemaphore, AIL_preference[MILES_PREFERENCE_TIMER_WAIT]) != 0)
        {
            if (MSS_profile != NULL)
            {
                MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_GENERIC_FILE, 469);
            }

            break;
        }

        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_GENERIC_FILE, 472);
        }

        if (rrMutexLockTimeout(&MSS_api_mutex, 0) == 0)
        {
            if (MSS_profile != NULL)
            {
                MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, MILES_ZONE_FLAGS_BLOCKED, MILES_GENERIC_FILE, 476,
                                        &TimerMutexBlockSite, MILES_MUTEX_BLOCK_NAME);
            }

            if (rrMutexLockTimeout(&MSS_api_mutex, MILES_TIMER_LOCK_WAIT_MILLISECONDS) != 0)
            {
                if (MSS_profile != NULL)
                {
                    MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_GENERIC_FILE, 479);
                }
            }
            else
            {
                if (MSS_profile != NULL)
                {
                    MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_GENERIC_FILE, 482);
                }

                continue;
            }
        }

        if (MSS_profile != NULL)
        {
            MSS_profile->pLockState(MSS_profile, &MSS_api_mutex, MILES_LOCK_STATE_ACQUIRED, MILES_GENERIC_FILE, 487,
                                    &TimerLockSite, MILES_MUTEX_NAME);

            if (MSS_profile != NULL)
            {
                MSS_profile->pZoneEnter(MSS_profile, 0, 0, 0, MILES_ZONE_FLAGS_NONE, MILES_GENERIC_FILE, 488,
                                        &TimerBackgroundSite, MILES_BACKGROUND_NAME);
            }
        }

        rrAtomicAddExchange32(&TimerBusy, 1);

        if (TimerTable != NULL && TimerBusy == 1)
        {
            uNow = rrGetTime();

            if (TimerLastTime != 0)
            {
                uElapsed = uNow - TimerLastTime;

                if (uElapsed > TimerMaxInterval)
                {
                    TimerMaxInterval = uElapsed;
                }
            }

            TimerLastTime = uNow;

            for (iTimer = 0; iTimer < TimerCount; iTimer++)
            {
                if (TimerTable[iTimer].iStatus == MILES_TIMER_RUNNING && uNow >= TimerTable[iTimer].uNextTime)
                {
                    do
                    {
                        TimerTable[iTimer].uNextTime += TimerTable[iTimer].uPeriod;
                        TimerTable[iTimer].pCallback(TimerTable[iTimer].uUser);

                        if (TimerTable[iTimer].iStatus != MILES_TIMER_RUNNING)
                        {
                            break;
                        }
                    } while (uNow >= TimerTable[iTimer].uNextTime);
                }
            }

            uElapsed = rrGetTime() - uNow;
            TimerTotalTime += uElapsed;
        }

        rrAtomicAddExchange32(&TimerBusy, -1);

        if (MSS_profile != NULL)
        {
            MSS_profile->pZoneLeave(MSS_profile, 0, 0, MILES_GENERIC_FILE, 490);

            if (MSS_profile != NULL)
            {
                MSS_profile->pLockState(MSS_profile, &MSS_api_mutex, MILES_LOCK_STATE_RELEASED, MILES_GENERIC_FILE, 491,
                                        &TimerUnlockSite, MILES_MUTEX_NAME);
            }
        }

        rrMutexUnlock(&MSS_api_mutex);
    }

    return 0;
}

extern "C" void AIL_API_shutdown()
{
    _DIG_DRIVER* pDriver;
    _DIG_DRIVER* pNext;
    int (*pFilterShutdown)();
    int (*pCodecShutdown)();
    int (*pMixerShutdown)();
    unsigned int uIndex;
    unsigned int uProvider;
    int iTimer;
    int bActive;

    MilesAsyncShutdown();
    InMilesMutex();

    pDriver = MSS_first_digital_driver;

    while (pDriver != NULL)
    {
        pNext = pDriver->next;
        AIL_close_digital_driver(pDriver);
        pDriver = pNext;
    }

    uIndex = MILES_ENUMERATE_FIRST;
    uProvider = 0;

    while (RIB_enumerate_providers(MILES_FILTER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_FILTER_INTERFACE, 0, MILES_FILTER_SHUTDOWN, &pFilterShutdown)
            == 0)
        {
            pFilterShutdown();
        }
    }

    uIndex = MILES_ENUMERATE_FIRST;

    while (RIB_enumerate_providers(MILES_CODEC_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_CODEC_INTERFACE, 0, MILES_CODEC_SHUTDOWN, &pCodecShutdown)
            == 0)
        {
            pCodecShutdown();
        }
    }

    uIndex = MILES_ENUMERATE_FIRST;

    while (RIB_enumerate_providers(MILES_MIXER_INTERFACE, &uIndex, &uProvider) != 0)
    {
        if (RIB_request_interface_entry(uProvider, MILES_MIXER_INTERFACE, 0, MILES_MIXER_SHUTDOWN, &pMixerShutdown)
            == 0)
        {
            pMixerShutdown();
        }
    }

    RIB_free_provider_library(0);

    if (TimerTable != NULL)
    {
        AIL_release_all_timers();

        bActive = 0;

        for (iTimer = 0; iTimer < TimerCount; iTimer++)
        {
            if (TimerTable[iTimer].iStatus != MILES_TIMER_FREE)
            {
                bActive = 1;
                break;
            }
        }

        if (bActive == 0)
        {
            rrSemaphoreIncrement(&TimerSemaphore, 1);
            rrThreadWaitDone(TimerThread, MILES_WAIT_INFINITE, NULL);
            rrThreadCleanUp(TimerThread);
            AIL_mem_free_lock(TimerThread);
            TimerThread = NULL;
            TimerThreadStarted = 1;
            AIL_mem_free_lock(TimerTable);
            TimerTable = NULL;
        }
    }

    OutMilesMutex();

    if (MSS_api_started != 0)
    {
        MSS_api_started = 0;
        rrMutexDestroy(&MSS_api_mutex);
    }
}

extern "C" void AIL_lock_mutex()
{
    InMilesMutex();
}

extern "C" void AIL_unlock_mutex()
{
    OutMilesMutex();
}

extern "C" void AIL_API_sleep(int iMilliseconds)
{
    rrThreadSleep(iMilliseconds);
}

extern "C" int AIL_API_register_timer(AIL_TIMER_CALLBACK pCallback)
{
    int iTimer;

    InMilesMutex();

    if (TimerTable == NULL)
    {
        if (MSS_api_started == 0)
        {
            return MILES_TIMER_INVALID;
        }

        TimerCount = AIL_preference[MILES_PREFERENCE_TIMER_COUNT];
        TimerTable = static_cast<MilesTimer*>(
            AIL_mem_alloc_lock_info(TimerCount * sizeof(MilesTimer), MILES_GENERIC_FILE, 721));

        if (TimerTable == NULL)
        {
            return MILES_TIMER_INVALID;
        }

        for (iTimer = 0; iTimer < TimerCount; iTimer++)
        {
            TimerTable[iTimer].iStatus = MILES_TIMER_FREE;
        }

        OutMilesMutex();

        rrSemaphoreCreate(&TimerSemaphore, 0, MILES_TIMER_SEMAPHORE_MAXIMUM);
        TimerThread = static_cast<rrThread*>(
            AIL_mem_alloc_lock_info(MILES_TIMER_THREAD_BLOCK_SIZE, MILES_GENERIC_FILE, 764));
        rrThreadCreate(TimerThread, ThreadProc, MILES_TIMER_THREAD_STACK_SIZE, NULL, MILES_TIMER_THREAD_FLAGS,
                       MILES_TIMER_THREAD_LABEL);
        rrThreadSetPriority(TimerThread, MILES_TIMER_THREAD_PRIORITY);
        rrThreadSetPriority(TimerThread, MILES_TIMER_THREAD_PRIORITY);
        rrThreadSetCPUCore(TimerThread, MILES_TIMER_THREAD_CORE);
        TimerThreadStarted = 1;
    }
    else
    {
        OutMilesMutex();
    }

    for (iTimer = 0; iTimer < TimerCount; iTimer++)
    {
        if (TimerTable[iTimer].iStatus == MILES_TIMER_FREE)
        {
            break;
        }
    }

    if (iTimer == TimerCount)
    {
        return MILES_TIMER_INVALID;
    }

    TimerTable[iTimer].iStatus = MILES_TIMER_ALLOCATED;
    TimerTable[iTimer].pCallback = pCallback;
    TimerTable[iTimer].uPeriod = rrMillisToTime(MILES_TIMER_DEFAULT_PERIOD_MILLISECONDS);
    TimerTable[iTimer].uNextTime = 0;

    return iTimer;
}

extern "C" unsigned int AIL_API_set_timer_user(int iTimer, unsigned int uUser)
{
    unsigned int uPrevious;

    if (iTimer == MILES_TIMER_INVALID)
    {
        return 0;
    }

    uPrevious = TimerTable[iTimer].uUser;
    TimerTable[iTimer].uUser = uUser;
    return uPrevious;
}

extern "C" void AIL_API_set_timer_period(int iTimer, unsigned int uMicroseconds)
{
    if (iTimer != MILES_TIMER_INVALID)
    {
        InMilesMutex();
        TimerTable[iTimer].uPeriod = rrMicrosToTime(uMicroseconds);
        TimerTable[iTimer].uNextTime = rrGetTime() + TimerTable[iTimer].uPeriod;
        OutMilesMutex();
    }
}

extern "C" void AIL_API_set_timer_frequency(int iTimer, unsigned int uHertz)
{
    if (iTimer != MILES_TIMER_INVALID)
    {
        AIL_set_timer_period(iTimer, MILES_MICROSECONDS_PER_SECOND / uHertz);
    }
}

extern "C" void AIL_API_start_timer(int iTimer)
{
    if (iTimer != MILES_TIMER_INVALID)
    {
        if (TimerTable[iTimer].iStatus == MILES_TIMER_ALLOCATED)
        {
            TimerTable[iTimer].uNextTime = rrGetTime() + TimerTable[iTimer].uPeriod;
            TimerTable[iTimer].iStatus = MILES_TIMER_RUNNING;
        }
    }
}

extern "C" void AIL_API_release_timer_handle(int iTimer)
{
    if (iTimer != MILES_TIMER_INVALID)
    {
        TimerTable[iTimer].iStatus = MILES_TIMER_FREE;
    }
}

extern "C" void AIL_API_release_all_timers()
{
    int iTimer;

    for (iTimer = 0; iTimer < TimerCount; iTimer++)
    {
        AIL_release_timer_handle(iTimer);
    }
}

extern "C" void AIL_API_set_error(const char* pError)
{
    if (pError == NULL)
    {
        AIL_error[0] = '\0';
        return;
    }

    strcpy(AIL_error, pError);
}

extern "C" int AIL_get_preference(int iPreference)
{
    return AIL_preference[iPreference];
}

extern "C" int AIL_ms_count()
{
    return (int)rrTimeToMillis(rrGetTime());
}

extern "C" unsigned int AIL_us_count()
{
    return (unsigned int)rrTimeToMicros(rrGetTime());
}

extern "C" void* AIL_API_mem_alloc_lock(unsigned int uSize)
{
    return AIL_mem_alloc_handler(uSize);
}

extern "C" void AIL_API_mem_free_lock(void* pBlock)
{
    if (pBlock != NULL)
    {
        AIL_mem_free_handler(pBlock);
    }
}

extern "C" void* MSS_alloc_info(unsigned int uSize, unsigned int, const char* pOwnerName, int iOwnerLine)
{
    return AIL_mem_alloc_lock_info(uSize, pOwnerName, iOwnerLine);
}

extern "C" void MSS_free_info(void* pBlock)
{
    AIL_mem_free_lock(pBlock);
}

extern "C" int AIL_API_background()
{
    return 0;
}

extern "C" const char* AIL_API_last_error()
{
    return AIL_error;
}

extern "C" void SS_start_DIG_driver_playback(_SAMPLE* pSample)
{
    _DIG_DRIVER* pDriver;

    pDriver = pSample->driver;

    if (pDriver->started == 0 && pDriver->hardwareChannels != 0)
    {
        pDriver->started = 1;
        InMilesMutex();

        if (pDriver->output != NULL)
        {
            pDriver->output->pSetPaused(pDriver->output, 0);
        }

        OutMilesMutex();
    }
}

void SS_serve(unsigned int uDriver)
{
    _DIG_DRIVER* pDriver;
    int iBufferCount;
    int iThreshold;
    _RadSoundSystem* pOutput;
    void* pBuffer;
    int iReady;
    int iPass;

    pDriver = reinterpret_cast<_DIG_DRIVER*>(static_cast<uintptr_t>(uDriver));

    if (pDriver->started != 0)
    {
        rrAtomicAddExchange32(&SS_serve_nesting, 1);

        if (SS_serve_nesting == 1)
        {
            iBufferCount = AIL_preference[MILES_PREFERENCE_BUFFER_COUNT];
            iThreshold = ((iBufferCount * AIL_preference[MILES_PREFERENCE_SERVICE_MILLISECONDS]
                           - AIL_preference[MILES_PREFERENCE_BUFFER_RESERVE]
                                 * AIL_preference[MILES_PREFERENCE_SERVICE_MILLISECONDS])
                          * (pDriver->hardwareChannels * pDriver->outputRate) * 2)
                         / 1000;
            pOutput = pDriver->output;

            iReady = pOutput->pGetReadyBytes(pOutput);
            iPass = 0;
            while (iReady >= iThreshold)
            {
                if (iPass >= iBufferCount)
                {
                    break;
                }

                iPass++;
                pOutput->pLock(pOutput, &pBuffer, pDriver->fillBytes);
                SS_fill(pDriver, static_cast<short*>(pBuffer));
                pOutput->pUnlock(pOutput);
                iReady = pOutput->pGetReadyBytes(pOutput);
            }
        }

        rrAtomicAddExchange32(&SS_serve_nesting, -1);
    }
}

extern "C" void AIL_API_close_digital_driver(_DIG_DRIVER* pDriver)
{
    _DIG_DRIVER* pPrevious;
    _DIG_DRIVER* pCurrent;
    int iSample;
    int iBuffer;

    if (pDriver == NULL || MSS_first_digital_driver == NULL)
    {
        return;
    }

    AILSTRM_shutdown(pDriver);
    rrAtomicAddExchange32(&SS_serve_nesting, 1);
    InMilesMutex();

    if (pDriver->started != 0)
    {
        pDriver->started = 0;

        if (pDriver->output != NULL)
        {
            pDriver->output->pSetPaused(pDriver->output, 1);
        }
    }

    for (iSample = 0; iSample < pDriver->sampleCount; iSample++)
    {
        if (pDriver->sampleStatus[iSample] != MILES_SAMPLE_STATUS_FREE)
        {
            AIL_release_sample_handle(&pDriver->samples[iSample]);
        }
    }

    if (pDriver->busBuffers[0] != NULL)
    {
        AIL_mem_free_lock(pDriver->busBuffers[0]);
    }

    if (pDriver->buses[0].buffer != NULL)
    {
        AIL_mem_free_lock(pDriver->buses[0].buffer);
    }

    if (pDriver->busBuffers[1] != NULL)
    {
        AIL_mem_free_lock(pDriver->busBuffers[1]);
    }

    if (pDriver->buses[1].buffer != NULL)
    {
        AIL_mem_free_lock(pDriver->buses[1].buffer);
    }

    if (pDriver->busBuffers[2] != NULL)
    {
        AIL_mem_free_lock(pDriver->busBuffers[2]);
    }

    if (pDriver->buses[2].buffer != NULL)
    {
        AIL_mem_free_lock(pDriver->buses[2].buffer);
    }

    if (pDriver->busBuffers[3] != NULL)
    {
        AIL_mem_free_lock(pDriver->busBuffers[3]);
    }

    if (pDriver->buses[3].buffer != NULL)
    {
        AIL_mem_free_lock(pDriver->buses[3].buffer);
    }

    FLT_disconnect_driver(pDriver);
    AIL_release_timer_handle(pDriver->timer);

    if (pDriver == MSS_first_digital_driver)
    {
        MSS_first_digital_driver = pDriver->next;
    }
    else
    {
        pPrevious = MSS_first_digital_driver;
        pCurrent = pPrevious->next;

        while (pCurrent != pDriver)
        {
            if (pCurrent == NULL)
            {
                break;
            }

            pPrevious = pCurrent;
            pCurrent = pCurrent->next;
        }

        if (pCurrent == pDriver)
        {
            pPrevious->next = pCurrent->next;
        }
    }

    OutMilesMutex();
    InMilesMutex();

    if (pDriver->output != NULL)
    {
        pDriver->output->pClose(pDriver->output);
    }

    AIL_mem_free_lock(pDriver->outputMemory);
    AIL_mem_free_lock(pDriver->output);
    AIL_mem_free_lock(pDriver->samples);
    AIL_mem_free_lock(pDriver->sampleStatus);

    for (iBuffer = 0; iBuffer < DRIVER_OUTPUT_COUNT; iBuffer++)
    {
        if (pDriver->outputs[iBuffer].buffer != NULL)
        {
            AIL_mem_free_lock(pDriver->outputs[iBuffer].buffer);
            pDriver->outputs[iBuffer].buffer = NULL;
        }
    }

    if (pDriver->mix.buffer != NULL)
    {
        AIL_mem_free_lock(pDriver->mix.buffer);
        pDriver->mix.buffer = NULL;
    }

    if (pDriver->interleaveBuffer != NULL)
    {
        AIL_mem_free_lock(pDriver->interleaveBuffer);
    }

    rrAtomicAddExchange32(&SS_serve_nesting, -1);
    OutMilesMutex();
    AIL_mem_free_lock(pDriver);
}

extern "C" _DIG_DRIVER* AIL_API_open_generic_digital_driver(unsigned int uFrequency, int, unsigned int uSpeakerConfig,
                                                            unsigned int uFlags, RADSS_OPEN_DRIVER_CALLBACK pOutput)
{
    _DIG_DRIVER* pDriver;
    _DIG_DRIVER* pNext;
    _DIG_DRIVER* pPrevious;
    _DIG_DRIVER* pCurrent;
    _RadSoundSystem* pSystem;
    MilesSpeakerPosition speakers[MILES_MAXIMUM_CHANNELS];
    _SAMPLE sample;
    unsigned int uSystemConfiguration;
    unsigned int uChannels;
    unsigned int uSpeakers;
    unsigned int uProvider;
    int iBufferTime;
    int iConfigCount;
    int iFirstChannel;
    int iRemaining;
    int iBuffer;
    int iSample;
    int iResult;
    int bFailed;

    bFailed = 0;
    pDriver = static_cast<_DIG_DRIVER*>(AIL_mem_alloc_lock_info(DIG_DRIVER_SIZE, MILES_DRIVER_FILE, 181));

    if (pDriver == NULL)
    {
        AIL_set_error("Could not allocate memory for driver descriptor.");
        return NULL;
    }

    memset(pDriver, 0, DIG_DRIVER_SIZE);

    pDriver->magic = DIG_DRIVER_MAGIC;
    pDriver->speakers.active = AIL_preference[MILES_PREFERENCE_DRIVER_FIELD];
    pDriver->speakers.listenerPosition[0] = MILES_ZERO;
    pDriver->speakers.listenerPosition[1] = MILES_ZERO;
    pDriver->speakers.listenerPosition[2] = MILES_ZERO;
    pDriver->speakers.listenerFace[0] = MILES_ZERO;
    pDriver->speakers.listenerFace[1] = MILES_ZERO;
    pDriver->speakers.listenerFace[2] = MILES_UNITY;
    pDriver->speakers.listenerUp[0] = MILES_ZERO;
    pDriver->speakers.listenerUp[1] = MILES_UNITY;
    pDriver->speakers.listenerUp[2] = MILES_ZERO;
    pDriver->speakers.listenerCross[0] = MILES_UNITY;
    pDriver->speakers.listenerCross[1] = MILES_ZERO;
    pDriver->speakers.listenerCross[2] = MILES_ZERO;
    pDriver->speakers.listenerVelocity[0] = MILES_ZERO;
    pDriver->speakers.listenerVelocity[1] = MILES_ZERO;
    pDriver->speakers.listenerVelocity[2] = MILES_ZERO;
    pDriver->speakers.rolloffFactor = MILES_UNITY;
    pDriver->speakers.dopplerFactor = MILES_UNITY;
    pDriver->speakers.distanceFactor = MILES_UNITY;

    if (uSpeakerConfig == MSS_MC_USE_SYSTEM_CONFIG)
    {
        uSystemConfiguration = MSS_MC_USE_SYSTEM_CONFIG;

        if (get_system_speaker_configuration(&uSystemConfiguration) == 0
            || uSystemConfiguration == MSS_MC_USE_SYSTEM_CONFIG)
        {
            AIL_set_error("Could not retrieve system speaker config.");
            AIL_mem_free_lock(pDriver);
            return NULL;
        }

        uSpeakerConfig = uSystemConfiguration;
    }

    uChannels = 0;
    uSpeakers = 0;

    switch (uSpeakerConfig)
    {
    case MSS_MC_MONO:
        uChannels = 1;
        uSpeakers = 1;
        break;
    case MSS_MC_STEREO:
    case MSS_MC_HEADPHONES:
        uChannels = 2;
        uSpeakers = 2;
        break;
    case MSS_MC_DOLBY_SURROUND:
        uChannels = 2;
        uSpeakers = 3;
        break;
    case MSS_MC_SRS_CIRCLE_SURROUND:
        uChannels = 2;
        uSpeakers = 7;
        break;
    case MSS_MC_40_DTS:
        uChannels = 2;
        uSpeakers = 4;
        break;
    case MSS_MC_40_DISCRETE:
        uChannels = 4;
        uSpeakers = 4;
        break;
    case MSS_MC_51_DTS:
        uChannels = 2;
        uSpeakers = 6;
        break;
    case MSS_MC_51_DISCRETE:
        uChannels = 6;
        uSpeakers = 6;
        break;
    case MSS_MC_61_DISCRETE:
        uChannels = 7;
        uSpeakers = 7;
        break;
    case MSS_MC_71_DISCRETE:
        uChannels = 8;
        uSpeakers = 8;
        break;
    case MSS_MC_81_DISCRETE:
        uChannels = 9;
        uSpeakers = 9;
        break;
    default:
        break;
    }

    pDriver->outputRate = uFrequency;
    pDriver->requestedSpeakerConfig = uSpeakerConfig;
    pDriver->started = 0;
    pDriver->bufferCount = 2;
    pDriver->hardwareChannels = uChannels;
    pDriver->speakerConfig = uSpeakers;

    if (uChannels == 0)
    {
        pDriver->bufferCount = 0;
        pDriver->outputCount = 0;
        pDriver->flags = MILES_DEVICE_FLAGS_SOFTWARE;
        pDriver->framesPerBuffer = MILES_SOFTWARE_BUFFER_SAMPLES;
        pDriver->fillBytes = 0;
    }
    else if (uChannels == 1)
    {
        pDriver->flags = MILES_DEVICE_FLAGS_MONO;
    }
    else
    {
        pDriver->flags = MILES_DEVICE_FLAGS_STEREO;

        if (uChannels > 2)
        {
            pDriver->flags = MILES_DEVICE_FLAGS_MULTICHANNEL;
        }
    }

    if (pDriver->hardwareChannels > MILES_MAXIMUM_CHANNELS)
    {
        AIL_set_error("Requested channel count is outside the maximum for this platform.");
        AIL_mem_free_lock(pDriver);
        return NULL;
    }

    pSystem = static_cast<_RadSoundSystem*>(
        AIL_mem_alloc_lock_info(MILES_RADSS_ALLOCATION_SIZE, MILES_DRIVER_FILE, 329));

    if (pSystem == NULL)
    {
        AIL_set_error("Out of memory.");
        AIL_mem_free_lock(pDriver);
        return NULL;
    }

    pDriver->output = pSystem;
    iBufferTime = 0;

    if (pDriver->hardwareChannels != 0)
    {
        iBufferTime = AIL_preference[MILES_PREFERENCE_SERVICE_MILLISECONDS]
                      * AIL_preference[MILES_PREFERENCE_BUFFER_COUNT];
        pDriver->fillBytes =
            (((AIL_preference[MILES_PREFERENCE_SERVICE_MILLISECONDS] * uFrequency) / MILES_MILLISECONDS_PER_SECOND
              + MILES_SAMPLE_ALIGNMENT_ROUND)
             & MILES_SAMPLE_ALIGNMENT_MASK)
            * uChannels * 2;
    }

    if (pOutput == NULL)
    {
        pOutput = AIL_get_default_output();
    }

    iResult = pOutput(pSystem, iBufferTime, pDriver->outputRate, pDriver->hardwareChannels, pDriver->fillBytes, 0);

    if (iResult == 0)
    {
        AIL_set_error(pSystem->pError);
        AIL_mem_free_lock(pSystem);
        AIL_mem_free_lock(pDriver);
        return NULL;
    }

    if (pDriver->hardwareChannels == 0)
    {
        pDriver->fillPeriod = (unsigned long long)pDriver->framesPerBuffer * MILES_SAMPLE_BUFFER_SAMPLE_SCALE
                              / pDriver->outputRate;
        uSpeakers = pDriver->speakerConfig;
    }
    else
    {
        pDriver->outputMemory = AIL_mem_alloc_lock_info(pSystem->uMemoryBytes, MILES_DRIVER_FILE, 384);

        if (pDriver->outputMemory == NULL)
        {
            pSystem->pClose(pSystem);
            AIL_mem_free_lock(pSystem);
            AIL_mem_free_lock(pDriver);
            AIL_set_error("Out of memory.");
            return NULL;
        }

        pSystem->pSetMemory(pSystem, pDriver->outputMemory);

        pDriver->framesPerBuffer = (static_cast<int>(pDriver->fillBytes) / pDriver->bufferCount)
                                   / pDriver->hardwareChannels;
        pDriver->fillPeriod = (unsigned long long)pDriver->framesPerBuffer * MILES_SAMPLE_BUFFER_SAMPLE_SCALE
                              / pDriver->outputRate;
        uSpeakers = pDriver->speakerConfig;
    }

    if (uSpeakers != 0)
    {
        pDriver->outputCount = 1;

        if (static_cast<int>(uSpeakers) > 2)
        {
            pDriver->outputCount = static_cast<int>(uSpeakers) / 2 + (uSpeakers & 1);
        }
    }

    memset(pDriver->outputs, 0, sizeof(pDriver->outputs));

    iFirstChannel = 0;
    iRemaining = pDriver->speakerConfig;

    for (iBuffer = 0; iBuffer < pDriver->outputCount; iBuffer++)
    {
        pDriver->outputs[iBuffer].chans = 1;

        if (iRemaining > 1)
        {
            pDriver->outputs[iBuffer].chans = 2;
        }

        pDriver->outputs[iBuffer].bytes = pDriver->outputs[iBuffer].chans * pDriver->framesPerBuffer
                                          * MILES_BUILD_BUFFER_SAMPLE_BYTES;
        pDriver->outputs[iBuffer].buffer = static_cast<unsigned char*>(
            AIL_mem_alloc_lock_info(pDriver->outputs[iBuffer].bytes, MILES_DRIVER_FILE, 445));
        pDriver->outputs[iBuffer].speaker = iFirstChannel;
        iFirstChannel = iFirstChannel + pDriver->outputs[iBuffer].chans;

        if (pDriver->outputs[iBuffer].buffer == NULL)
        {
            AIL_set_error("Could not allocate build buffer.");
            bFailed = 1;
            break;
        }

        iRemaining = iRemaining - pDriver->outputs[iBuffer].chans;
    }


    if (bFailed == 0)
    {
        pDriver->interleaveBuffer = static_cast<unsigned char*>(
            AIL_mem_alloc_lock_info(pDriver->framesPerBuffer * 2 + MILES_SCRATCH_PADDING, MILES_DRIVER_FILE, 507));
        pDriver->mix.chans = MILES_REVERB_BUILD_CHANNELS;
        pDriver->mix.bytes = pDriver->framesPerBuffer * MILES_REVERB_BUILD_SAMPLE_BYTES;
        pDriver->mix.buffer = static_cast<unsigned char*>(AIL_mem_alloc_lock_info(
            pDriver->framesPerBuffer * MILES_REVERB_BUILD_SAMPLE_BYTES, MILES_DRIVER_FILE, 520));

        if (pDriver->mix.buffer == NULL)
        {
            AIL_set_error("Could not allocate build buffer.");
            bFailed = 1;
        }
    }

    if (bFailed == 0)
    {
        if (AIL_allocate_reverb_buffers(pDriver, 0) == 0)
        {
            AIL_set_error("Could not allocate reverb build buffer.");
            bFailed = 1;
        }
    }

    if (bFailed == 0)
    {
        memset(speakers, 0, sizeof(speakers));
        iConfigCount = pDriver->speakerConfig;

        switch (pDriver->requestedSpeakerConfig)
        {
        case MSS_MC_MONO:
            speakers[0].fX = MILES_ZERO;
            speakers[0].fY = MILES_ZERO;
            speakers[0].fZ = MILES_UNITY;
            break;
        case MSS_MC_STEREO:
            speakers[0].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[0].fY = MILES_ZERO;
            speakers[0].fZ = MILES_ROOT_HALF;
            speakers[1].fX = MILES_ROOT_HALF;
            speakers[1].fY = MILES_ZERO;
            speakers[1].fZ = MILES_ROOT_HALF;
            break;
        case MSS_MC_HEADPHONES:
            speakers[0].fX = MILES_NEGATIVE_UNITY;
            speakers[0].fY = MILES_ZERO;
            speakers[0].fZ = MILES_ZERO;
            speakers[1].fX = MILES_UNITY;
            speakers[1].fY = MILES_ZERO;
            speakers[1].fZ = MILES_ZERO;
            break;
        case MSS_MC_DOLBY_SURROUND:
            speakers[0].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[0].fY = MILES_ZERO;
            speakers[0].fZ = MILES_ROOT_HALF;
            speakers[1].fX = MILES_ROOT_HALF;
            speakers[1].fY = MILES_ZERO;
            speakers[1].fZ = MILES_ROOT_HALF;
            speakers[2].fX = MILES_ZERO;
            speakers[2].fY = MILES_ZERO;
            speakers[2].fZ = MILES_NEGATIVE_UNITY;
            break;
        case MSS_MC_SRS_CIRCLE_SURROUND:
        case MSS_MC_61_DISCRETE:
            speakers[0].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[0].fY = MILES_ZERO;
            speakers[0].fZ = MILES_ROOT_HALF;
            speakers[1].fX = MILES_ROOT_HALF;
            speakers[1].fY = MILES_ZERO;
            speakers[1].fZ = MILES_ROOT_HALF;
            speakers[2].fX = MILES_ZERO;
            speakers[2].fY = MILES_ZERO;
            speakers[2].fZ = MILES_UNITY;
            speakers[3].fX = MILES_ZERO;
            speakers[3].fY = MILES_ZERO;
            speakers[3].fZ = MILES_ZERO;
            speakers[4].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[4].fY = MILES_ZERO;
            speakers[4].fZ = MILES_NEGATIVE_ROOT_HALF;
            speakers[5].fX = MILES_ROOT_HALF;
            speakers[5].fY = MILES_ZERO;
            speakers[5].fZ = MILES_NEGATIVE_ROOT_HALF;
            speakers[6].fX = MILES_ZERO;
            speakers[6].fY = MILES_ZERO;
            speakers[6].fZ = MILES_NEGATIVE_UNITY;
            break;
        case MSS_MC_40_DTS:
        case MSS_MC_40_DISCRETE:
            speakers[0].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[0].fY = MILES_ZERO;
            speakers[0].fZ = MILES_ROOT_HALF;
            speakers[1].fX = MILES_ROOT_HALF;
            speakers[1].fY = MILES_ZERO;
            speakers[1].fZ = MILES_ROOT_HALF;
            speakers[2].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[2].fY = MILES_ZERO;
            speakers[2].fZ = MILES_NEGATIVE_ROOT_HALF;
            speakers[3].fX = MILES_ROOT_HALF;
            speakers[3].fY = MILES_ZERO;
            speakers[3].fZ = MILES_NEGATIVE_ROOT_HALF;
            break;
        case MSS_MC_51_DTS:
        case MSS_MC_51_DISCRETE:
            speakers[0].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[0].fY = MILES_ZERO;
            speakers[0].fZ = MILES_ROOT_HALF;
            speakers[1].fX = MILES_ROOT_HALF;
            speakers[1].fY = MILES_ZERO;
            speakers[1].fZ = MILES_ROOT_HALF;
            speakers[2].fX = MILES_ZERO;
            speakers[2].fY = MILES_ZERO;
            speakers[2].fZ = MILES_UNITY;
            speakers[3].fX = MILES_ZERO;
            speakers[3].fY = MILES_ZERO;
            speakers[3].fZ = MILES_ZERO;
            speakers[4].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[4].fY = MILES_ZERO;
            speakers[4].fZ = MILES_NEGATIVE_ROOT_HALF;
            speakers[5].fX = MILES_ROOT_HALF;
            speakers[5].fY = MILES_ZERO;
            speakers[5].fZ = MILES_NEGATIVE_ROOT_HALF;
            break;
        case MSS_MC_71_DISCRETE:
            speakers[0].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[0].fY = MILES_ZERO;
            speakers[0].fZ = MILES_ROOT_HALF;
            speakers[1].fX = MILES_ROOT_HALF;
            speakers[1].fY = MILES_ZERO;
            speakers[1].fZ = MILES_ROOT_HALF;
            speakers[2].fX = MILES_ZERO;
            speakers[2].fY = MILES_ZERO;
            speakers[2].fZ = MILES_UNITY;
            speakers[3].fX = MILES_ZERO;
            speakers[3].fY = MILES_ZERO;
            speakers[3].fZ = MILES_ZERO;
            speakers[4].fX = MILES_NEGATIVE_ROOT_HALF;
            speakers[4].fY = MILES_ZERO;
            speakers[4].fZ = MILES_NEGATIVE_ROOT_HALF;
            speakers[5].fX = MILES_ROOT_HALF;
            speakers[5].fY = MILES_ZERO;
            speakers[5].fZ = MILES_NEGATIVE_ROOT_HALF;
            iConfigCount = MILES_DEFAULT_SPEAKER_LEVEL_COUNT;
            break;
        case MSS_MC_81_DISCRETE:
            AIL_set_error("Invalid channel spec for platform.");
            bFailed = 1;
        default:
            break;
        }
    }

    if (bFailed == 0)
    {
        AIL_set_speaker_configuration(pDriver, &speakers[0].fX, iConfigCount, MILES_SPEAKER_FALLOFF_POWER);
        AIL_set_speaker_reverb_levels(pDriver, NULL, NULL, NULL, 0);

        pDriver->sampleCount = AIL_get_preference(MILES_PREFERENCE_SAMPLE_COUNT);
        pDriver->activeSamples = 0;
        pDriver->masterVolume = MILES_UNITY;
        pDriver->buses[0].sendLevel = MILES_UNITY;
        pDriver->buses[0].returnGain = MILES_UNITY;

        pDriver->samples = static_cast<_SAMPLE*>(
            AIL_mem_alloc_lock_info(pDriver->sampleCount * sizeof(_SAMPLE), MILES_DRIVER_FILE, 727));

        if (pDriver->samples == NULL)
        {
            AIL_set_error("Could not allocate SAMPLE structures.");
            AIL_mem_free_lock(pDriver->buses[0].buffer);
            bFailed = 1;
        }
    }

    if (bFailed == 0)
    {
        pDriver->sampleStatus = static_cast<int*>(
            AIL_mem_alloc_lock_info(pDriver->sampleCount * sizeof(int), MILES_DRIVER_FILE, 738));

        if (pDriver->sampleStatus == NULL)
        {
            AIL_mem_free_lock(pDriver->samples);
            AIL_set_error("Could not allocate SAMPLE structures.");
            AIL_mem_free_lock(pDriver->buses[0].buffer);
            bFailed = 1;
        }
    }

    if (bFailed == 0)
    {
        memset(pDriver->samples, 0, pDriver->sampleCount * sizeof(_SAMPLE));

        for (iSample = 0; iSample < pDriver->sampleCount; iSample++)
        {
            pDriver->samples[iSample].magic = SAMPLE_MAGIC;
            pDriver->sampleStatus[iSample] = MILES_SAMPLE_STATUS_FREE;
            pDriver->samples[iSample].index = iSample;
            pDriver->samples[iSample].driver = pDriver;
            pDriver->samples[iSample].is3D = 0;
        }
    }

    if (bFailed == 0)
    {
        if (MSS_first_digital_driver == NULL)
        {
            pDriver->next = NULL;
            AIL_primary_digital_driver(pDriver);
            MSS_first_digital_driver = pDriver;
            pDriver->timer = AIL_register_timer(SS_serve);
        }
        else
        {
            pDriver->next = MSS_first_digital_driver;
            MSS_first_digital_driver = pDriver;
            pDriver->timer = AIL_register_timer(SS_serve);
        }

        if (pDriver->timer == MILES_TIMER_INVALID)
        {
            AIL_set_error("Out of timer handles.");
            AIL_mem_free_lock(pDriver->buses[0].buffer);
            AIL_mem_free_lock(pDriver->samples);
            AIL_mem_free_lock(pDriver->sampleStatus);
            bFailed = 1;
        }
    }

    if (bFailed == 0)
    {
        pDriver->processors[0].provider = 0;
        pDriver->processors[1].provider = 0;
        pDriver->processors[2].provider = 0;
        pDriver->processors[3].provider = 0;
        pDriver->processors[4].provider = 0;
        pDriver->processors[5].provider = 0;

        RIB_enumerate_providers(MILES_MIXER_INTERFACE, NULL, &uProvider);

        if (uProvider == 0)
        {
            AIL_set_error("No mixer services exist.");
            AIL_mem_free_lock(pDriver->buses[0].buffer);
            AIL_mem_free_lock(pDriver->samples);
            AIL_mem_free_lock(pDriver->sampleStatus);
            bFailed = 1;
        }
    }

    if (bFailed == 0)
    {
        AIL_set_digital_driver_processor(pDriver, 1, 0);
        AIL_set_digital_driver_processor(pDriver, 2, uProvider);
        AIL_set_digital_driver_processor(pDriver, 0, uProvider);
        AIL_set_digital_driver_processor(pDriver, 3, uProvider);
        AIL_set_digital_driver_processor(pDriver, 4, uProvider);
        AIL_set_digital_driver_processor(pDriver, 5, uProvider);

        AIL_set_timer_frequency(pDriver->timer,
                                MILES_MILLISECONDS_PER_SECOND / AIL_preference[MILES_PREFERENCE_SERVICE_MILLISECONDS]);
        AIL_set_timer_user(pDriver->timer, static_cast<unsigned int>(reinterpret_cast<uintptr_t>(pDriver)));
        AIL_start_timer(pDriver->timer);

        iResult = 1;

        switch (uSpeakerConfig)
        {
        case MSS_MC_DOLBY_SURROUND:
            iResult = MC_open_output_filter("Dolby Surround Matrix Filter", pDriver, 1);
            break;
        case MSS_MC_SRS_CIRCLE_SURROUND:
            iResult = MC_open_output_filter("SRS Circle Surround 6.1 Matrix Filter", pDriver, 1);
            break;
        case MSS_MC_40_DTS:
        case MSS_MC_51_DTS:
            iResult = MC_open_output_filter("DTS 4.0/5.1 Matrix Filter", pDriver, 1);
            break;
        case MSS_MC_DIRECTSOUND3D:
            iResult = MC_open_output_filter("DirectSound3D Hardware Support", pDriver, 0);
            break;
        case MSS_MC_EAX2:
            iResult = MC_open_output_filter("Creative Labs EAX 2 (TM)", pDriver, 0);
            break;
        case MSS_MC_EAX3:
            iResult = MC_open_output_filter("Creative Labs EAX 3 (TM)", pDriver, 0);
            break;
        case MSS_MC_EAX4:
            iResult = MC_open_output_filter("Creative Labs EAX 4 (TM)", pDriver, 0);
            break;
        default:
            break;
        }

        if (iResult == 0)
        {
            AIL_API_close_digital_driver(pDriver);
            AIL_set_error("Couldn't create output filter.");
            return NULL;
        }

        if ((uFlags & MILES_OPEN_START_PLAYBACK) != 0)
        {
            sample.driver = pDriver;
            SS_start_DIG_driver_playback(&sample);
        }
    }

    if (bFailed == 0)
    {
        return pDriver;
    }

    for (iBuffer = 0; iBuffer < DRIVER_OUTPUT_COUNT; iBuffer++)
    {
        if (pDriver->outputs[iBuffer].buffer != NULL)
        {
            AIL_mem_free_lock(pDriver->outputs[iBuffer].buffer);
            pDriver->outputs[iBuffer].buffer = NULL;
        }
    }

    if (pDriver->mix.buffer != NULL)
    {
        AIL_mem_free_lock(pDriver->mix.buffer);
        pDriver->mix.buffer = NULL;
    }

    if (pDriver->output != NULL)
    {
        pDriver->output->pClose(pDriver->output);
    }

    AIL_mem_free_lock(pDriver->outputMemory);
    AIL_mem_free_lock(pDriver->output);

    pNext = pDriver->next;
    AIL_mem_free_lock(pDriver);

    if (MSS_primary_digital_driver == pDriver)
    {
        MSS_primary_digital_driver = pNext;
    }

    if (MSS_first_digital_driver == pDriver)
    {
        MSS_first_digital_driver = pNext;
        return NULL;
    }

    if (MSS_first_digital_driver != NULL)
    {
        pPrevious = MSS_first_digital_driver;

        for (pCurrent = pPrevious->next; pCurrent != NULL; pCurrent = pCurrent->next)
        {
            if (pCurrent == pDriver)
            {
                pPrevious->next = pCurrent->next;
                break;
            }

            pPrevious = pCurrent;
        }
    }

    return NULL;
}
