#include "vendor/__N_14_crypto_Md5_cpp_c46079df.hpp"

static inline uint32_t SwapBytes(uint32_t value)
{
    return (value << 24) | ((value & 0x0000ff00U) << 8) | (value >> 24) | ((value >> 8) & 0x0000ff00U);
}

static inline uint32_t RotateLeft(uint32_t value, uint32_t shift)
{
    return (value << shift) | (value >> (32U - shift));
}

int32_t __N_14_crypto_Md5_cpp_c46079df::CalcRound1(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord,
                                                   uint32_t shift, uint32_t constant)
{
    messageWord = SwapBytes(messageWord);
    return (int32_t)(b + RotateLeft(a + ((b & c) | (d & ~b)) + messageWord + constant, shift));
}

int32_t __N_14_crypto_Md5_cpp_c46079df::CalcRound2(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord,
                                                   uint32_t shift, uint32_t constant)
{
    messageWord = SwapBytes(messageWord);
    return (int32_t)(b + RotateLeft(a + ((b & d) | (c & ~d)) + messageWord + constant, shift));
}

int32_t __N_14_crypto_Md5_cpp_c46079df::CalcRound4(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t messageWord,
                                                   uint32_t shift, uint32_t constant)
{
    messageWord = SwapBytes(messageWord);
    return (int32_t)(b + RotateLeft(a + (c ^ (b | ~d)) + messageWord + constant, shift));
}
