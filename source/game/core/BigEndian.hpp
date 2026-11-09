#ifndef GAME_CORE_BIGENDIAN_HPP
#define GAME_CORE_BIGENDIAN_HPP

namespace BigEndian
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
inline unsigned short ToHost(unsigned short value)
{
    return value;
}

inline short ToHost(short value)
{
    return value;
}

inline unsigned int ToHost(unsigned int value)
{
    return value;
}

inline int ToHost(int value)
{
    return value;
}
#else
inline unsigned short ToHost(unsigned short value)
{
    return static_cast<unsigned short>((value << 8) | (value >> 8));
}

inline short ToHost(short value)
{
    return static_cast<short>(ToHost(static_cast<unsigned short>(value)));
}

inline unsigned int ToHost(unsigned int value)
{
    return (value << 24) | ((value & 0x0000ff00) << 8) | ((value >> 8) & 0x0000ff00) | (value >> 24);
}

inline int ToHost(int value)
{
    return static_cast<int>(ToHost(static_cast<unsigned int>(value)));
}
#endif
}

#endif
