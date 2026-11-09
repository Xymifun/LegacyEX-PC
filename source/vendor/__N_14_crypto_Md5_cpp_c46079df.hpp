#ifndef SOURCE_VENDOR___N_14_CRYPTO_MD5_CPP_C46079DF_HPP
#define SOURCE_VENDOR___N_14_CRYPTO_MD5_CPP_C46079DF_HPP

#include <stdint.h>

namespace __N_14_crypto_Md5_cpp_c46079df
{
int32_t CalcRound1(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord, uint32_t shift,
                   uint32_t constant);
int32_t CalcRound2(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord, uint32_t shift,
                   uint32_t constant);
int32_t CalcRound4(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord, uint32_t shift,
                   uint32_t constant);
}

#endif
