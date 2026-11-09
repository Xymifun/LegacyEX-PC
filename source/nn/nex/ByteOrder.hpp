#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BYTEORDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BYTEORDER_HPP

namespace nn
{
namespace nex
{

template <typename T>
T HostOrderToNexNetworkOrder(T value)
{
    if (sizeof(T) == sizeof(unsigned short))
    {
        return static_cast<T>(static_cast<T>((value & 0xFF) << 8) | static_cast<T>((value >> 8) & 0xFF));
    }
    return static_cast<T>(static_cast<T>((value & 0xFF) << 24) | static_cast<T>(((value >> 8) & 0xFF) << 16)
                          | static_cast<T>(((value >> 16) & 0xFF) << 8) | static_cast<T>((value >> 24) & 0xFF));
}

}
}

#endif
