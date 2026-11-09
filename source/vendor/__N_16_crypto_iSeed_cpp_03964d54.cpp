#include "vendor/__N_16_crypto_iSeed_cpp_03964d54.hpp"

#include "nn/crypto/iSeed.hpp"

#include <string.h>

#ifdef __ghs__
#include <ppc_ghs.h>
#elif defined(__powerpc__) || defined(__PPC__)
#include <coreinit/time.h>
#else
#include <time.h>
#endif

static bool g_seedInitialized = false;
static uint32_t g_seed[4];
static int g_getSeedCount = 0;

#ifdef __ghs__
uint64_t __N_16_crypto_iSeed_cpp_03964d54::rdtsc()
{
    uint32_t upperCheck;
    int lower;
    uint32_t upper;

    do
    {
        upper = __MFTBU();
        lower = __MFTB();
        upperCheck = __MFTBU();
    } while (upperCheck != upper);

    return (static_cast<uint64_t>(upperCheck) << 32) | lower;
}

uint64_t __N_16_crypto_iSeed_cpp_03964d54::GetTicks()
{
    uint32_t upper = __MFTBU();
    int lower = __MFTB();
    return lower | (static_cast<uint64_t>(upper) << 32);
}
#elif defined(__powerpc__) || defined(__PPC__)
uint64_t __N_16_crypto_iSeed_cpp_03964d54::rdtsc()
{
    const uint64_t timeBase = static_cast<uint64_t>(OSGetSystemTime());
    const uint32_t upper = static_cast<uint32_t>(timeBase >> 32);
    const int lower = static_cast<int>(static_cast<uint32_t>(timeBase));
    return (static_cast<uint64_t>(upper) << 32) | lower;
}

uint64_t __N_16_crypto_iSeed_cpp_03964d54::GetTicks()
{
    const uint64_t timeBase = static_cast<uint64_t>(OSGetSystemTime());
    const uint32_t upper = static_cast<uint32_t>(timeBase >> 32);
    const int lower = static_cast<int>(static_cast<uint32_t>(timeBase));
    return lower | (static_cast<uint64_t>(upper) << 32);
}
#else
uint64_t __N_16_crypto_iSeed_cpp_03964d54::rdtsc()
{
    return static_cast<uint64_t>(clock());
}

uint64_t __N_16_crypto_iSeed_cpp_03964d54::GetTicks()
{
    return static_cast<uint64_t>(clock());
}
#endif

uint64_t __N_16_crypto_iSeed_cpp_03964d54::hash64shift(unsigned long long value)
{
    value = (~value) + (value << 21);
    value ^= value >> 24;
    value = value + (value << 3) + (value << 8);
    value ^= value >> 14;
    value = value + (value << 2) + (value << 4);
    value ^= value >> 28;
    value = value + (value << 31);
    return static_cast<uint64_t>(value);
}

void __N_16_crypto_iSeed_cpp_03964d54::Stir()
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint64_t mixed;

    if (!g_seedInitialized)
    {
        g_seedInitialized = true;
        g_seed[3] = 0x9e3779b1U;
        g_seed[1] = 0x9e3779b9U;
        g_seed[2] = 0x9e3779b9U;
        g_seed[0] = 0x9e3779b1U;
    }

    a = g_seed[0];
    b = g_seed[1];
    c = g_seed[2];
    a -= b;
    a -= c;
    a ^= c >> 13;
    b -= c;
    b -= a;
    b ^= a << 8;
    c -= a;
    c -= b;
    c ^= b >> 13;
    a -= b;
    a -= c;
    a ^= c >> 12;
    b -= c;
    b -= a;
    b ^= a << 16;
    c -= a;
    c -= b;
    c ^= b >> 5;
    a -= b;
    a -= c;
    a ^= c >> 3;
    b -= c;
    b -= a;
    b ^= a << 10;
    c -= a;
    c -= b;
    c ^= b >> 15;
    g_seed[0] = a;
    g_seed[1] = b;
    g_seed[2] = c;

    mixed = hash64shift((static_cast<uint64_t>(g_seed[2]) << 32) | g_seed[0]);
    g_seed[2] = static_cast<uint32_t>(mixed >> 32);
    g_seed[3] = static_cast<uint32_t>(mixed);
    mixed = hash64shift((static_cast<uint64_t>(g_seed[3]) << 32) | g_seed[1]);
    g_seed[0] = static_cast<uint32_t>(mixed >> 32);
    g_seed[1] = static_cast<uint32_t>(mixed);
}

uint32_t __N_16_crypto_iSeed_cpp_03964d54::AddToSeed(unsigned char* data, unsigned int length)
{
    unsigned int index;

    if (data == 0 || length == 0)
        return 0;
    for (index = 0; index < length; ++index)
    {
        g_seed[0] ^= data[index];
        Stir();
    }
    return 1;
}

uint32_t nn::crypto::detail::GetSeed(unsigned char* data, unsigned int length)
{
    unsigned int offset;
    unsigned int copySize;
    uint64_t ticks;

    if (data != 0 && length != 0)
    {
        if (g_getSeedCount % 3 == 0)
        {
            __N_16_crypto_iSeed_cpp_03964d54::Stir();
            ticks = __N_16_crypto_iSeed_cpp_03964d54::GetTicks();
            __N_16_crypto_iSeed_cpp_03964d54::AddToSeed(reinterpret_cast<unsigned char*>(&ticks), sizeof(ticks));
            ticks = __N_16_crypto_iSeed_cpp_03964d54::rdtsc();
            __N_16_crypto_iSeed_cpp_03964d54::AddToSeed(reinterpret_cast<unsigned char*>(&ticks), sizeof(ticks));
            ++g_getSeedCount;
        }
        copySize = length;
        if (copySize > sizeof(g_seed))
            copySize = sizeof(g_seed);
        for (offset = 0; offset < length;)
        {
            __N_16_crypto_iSeed_cpp_03964d54::Stir();
            memcpy(data + offset, g_seed, copySize);
            offset += copySize;
            copySize = length - offset;
            if (copySize > sizeof(g_seed))
                copySize = sizeof(g_seed);
        }
        return 1;
    }
    return 0;
}
