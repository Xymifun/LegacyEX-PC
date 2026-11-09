#ifndef GAME_CORE_NAMEABLE_HPP
#define GAME_CORE_NAMEABLE_HPP

#include <string>

class Nameable
{
public:
    virtual std::wstring getName() = 0;
    virtual std::wstring getCustomName() = 0;
    virtual bool hasCustomName() = 0;
    virtual std::wstring getDisplayName() = 0;
};

#endif
