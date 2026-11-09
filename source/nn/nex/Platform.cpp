#include "nn/nex/Platform.hpp"

#include <string.h>
#include <coreinit/systeminfo.h>
#include <coreinit/thread.h>
#include <coreinit/time.h>
#include <nn/act/client_cpp.h>

#include "nn/nex/BadEvents.hpp"
#include "nn/nex/MD5.hpp"
#include "nn/nex/RandomNumberGenerator.hpp"

namespace nn
{
namespace nex
{

Platform* Platform::s_instance = 0;

static RandomNumberGenerator s_platformRandomNumberGenerator;
static bool s_bPlatformRandomNumberSeeded = false;

Platform::Platform()
{
    s_instance = this;
}

Platform::~Platform()
{
}

void Platform::CreateInstance()
{
    s_instance = new Platform();
}

void Platform::DeleteInstance()
{
    if (s_instance == 0)
        return;

    delete s_instance;
}

void Platform::Sleep(unsigned int milliseconds)
{
    OSSleepTicks(OSMillisecondsToTicks(static_cast<OSTime>(milliseconds)));
}

void Platform::YieldThread()
{
    OSYieldThread();
}

int Platform::Breakpoint()
{
    __builtin_trap();
    return 0;
}

int64_t Platform::GetTick()
{
    return OSGetTime();
}

uint64_t Platform::GetPlatformID()
{
    static uint64_t s_uiPlatformID = 0;
    static bool s_bPlatformIDInitialized = false;

    if (!s_bPlatformIDInitialized)
    {
        nn::act::Initialize();
        char uuid[nn::act::UuidSize];
        nn::act::GetUuid(uuid);
        memcpy(&s_uiPlatformID, uuid, sizeof(s_uiPlatformID));
        nn::act::Finalize();
        s_bPlatformIDInitialized = true;
    }

    return s_uiPlatformID;
}

void Platform::SetRandomNumberSeed(unsigned int seed)
{
    s_platformRandomNumberGenerator.SetRandomNumberSeed(seed);
    s_bPlatformRandomNumberSeeded = true;
}

unsigned int Platform::GetRandomSeed()
{
    unsigned long long aSeed[3] = {0x024F8D827839D088ULL, 0x0F831237889EF409ULL, 0xAB37798789797343ULL};
    aSeed[0] ^= static_cast<unsigned int>(OSGetTick());
    aSeed[1] ^= GetTick();
    aSeed[2] ^= GetPlatformID();

    MD5 md5;
    md5.update(reinterpret_cast<const unsigned char*>(aSeed), sizeof(aSeed));
    md5.finalize();

    unsigned int aDigest[MD5::DigestSize / sizeof(unsigned int)];
    md5.raw_digest(reinterpret_cast<unsigned char*>(aDigest));
    return aDigest[0] ^ aDigest[1] ^ aDigest[2] ^ aDigest[3];
}

unsigned int Platform::GetRandomNumber(unsigned int limit)
{
    if (!s_bPlatformRandomNumberSeeded)
        SetRandomNumberSeed(GetRandomSeed());

    return s_platformRandomNumberGenerator.GetRandomNumber(limit);
}

float Platform::GetRealRandomNumber(float limit)
{
    if (!s_bPlatformRandomNumberSeeded)
        SetRandomNumberSeed(GetRandomSeed());

    return s_platformRandomNumberGenerator.GetRealRandomNumber(limit);
}

void Platform::WarnObsoleteMethod(const wchar_t*, const wchar_t*)
{
    BadEvents::Signal(BadEvents::BAD_EVENT_3);
}

}
}
