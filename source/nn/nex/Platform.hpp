#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PLATFORM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PLATFORM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class Platform : public RootObject
{
public:
    Platform();
    virtual ~Platform();

    static void CreateInstance();
    static void DeleteInstance();
    static Platform* Instance();

    static void Sleep(unsigned int milliseconds);
    static void YieldThread();
    static int Breakpoint();
    static int64_t GetTick();
    static uint64_t GetPlatformID();
    static uint32_t GetProcessID();
    static int64_t GetCurrentTimeMilli(int64_t time);

    static void SetRandomNumberSeed(unsigned int seed);
    static unsigned int GetRandomSeed();
    static unsigned int GetRandomNumber(unsigned int limit);
    static float GetRealRandomNumber(float limit);

    static void WarnObsoleteMethod(const wchar_t* method, const wchar_t* replacement);

    static void NetworkToHost(const unsigned char* source, unsigned short* destination);
    static void NetworkToHost(const unsigned char* source, unsigned int* destination);
    static void NetworkToHost(const unsigned char* source, unsigned long long* destination);
    static void HostToNetwork(const unsigned short* source, unsigned char* destination);
    static void HostToNetwork(const unsigned int* source, unsigned char* destination);
    static void HostToNetwork(const unsigned long long* source, unsigned char* destination);

private:
    static Platform* s_instance;
};

}
}

#endif
