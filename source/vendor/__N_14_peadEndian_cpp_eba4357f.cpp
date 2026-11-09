#include "vendor/__N_14_peadEndian_cpp_eba4357f.hpp"

unsigned int __N_14_peadEndian_cpp_eba4357f::Swap16(unsigned short value)
{
    return ((value << 8) | (value >> 8)) & 0xffff;
}

unsigned int __N_14_peadEndian_cpp_eba4357f::Swap32(unsigned int value)
{
    return (value << 24) | ((value & 0xff00u) << 8) | (value >> 24) | ((value >> 8) & 0xff00u);
}

unsigned long long __N_14_peadEndian_cpp_eba4357f::Swap64(unsigned long long value)
{
    return ((value & 0x00000000000000ffULL) << 56) | ((value & 0x000000000000ff00ULL) << 40)
           | ((value & 0x0000000000ff0000ULL) << 24) | ((value & 0x00000000ff000000ULL) << 8)
           | ((value & 0xff00000000000000ULL) >> 56) | ((value & 0x00ff000000000000ULL) >> 40)
           | ((value & 0x0000ff0000000000ULL) >> 24) | ((value & 0x000000ff00000000ULL) >> 8);
}

void __N_14_peadEndian_cpp_eba4357f::Swap8(unsigned char value)
{
}

void __N_14_peadEndian_cpp_eba4357f::Null8(unsigned char value)
{
}

void __N_14_peadEndian_cpp_eba4357f::Null16(unsigned short value)
{
}

void __N_14_peadEndian_cpp_eba4357f::Null32(unsigned int value)
{
}

void __N_14_peadEndian_cpp_eba4357f::Null64(unsigned long long value)
{
}
