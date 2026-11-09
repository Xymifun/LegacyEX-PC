#ifndef SOURCE_VENDOR_N_16_CRYPTO_ISEED_CPP_03964D54_HPP
#define SOURCE_VENDOR_N_16_CRYPTO_ISEED_CPP_03964D54_HPP

#include <stdint.h>

namespace __N_16_crypto_iSeed_cpp_03964d54
{
uint64_t rdtsc();
uint64_t GetTicks();
uint64_t hash64shift(unsigned long long value);
void Stir();
uint32_t AddToSeed(unsigned char* data, unsigned int length);
}

#endif
