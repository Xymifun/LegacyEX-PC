#ifndef LEGACY_CONSOLE_WIIU_NN_CRYPTO_ISEED_HPP
#define LEGACY_CONSOLE_WIIU_NN_CRYPTO_ISEED_HPP

#include <stdint.h>

namespace nn
{
namespace crypto
{
namespace detail
{

uint32_t GetSeed(unsigned char* pSeed, unsigned int uiSize);

}
}
}

#endif
