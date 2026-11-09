#include "game/world/block/AbstractProperty.hpp"

#include <string>

int javaStringHashCode(const std::wstring& value)
{
    int hash = 0;
    for (unsigned int i = 0; i < value.length(); ++i)
    {
        if (i < value.length() - 1)
        {
            hash += static_cast<unsigned short>(value[i]) * 31 ^ (value.length() - 1 - i);
        }
        else
        {
            hash += static_cast<unsigned short>(value[i]);
        }
    }
    return hash;
}
