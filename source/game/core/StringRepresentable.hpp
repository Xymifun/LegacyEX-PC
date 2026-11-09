#ifndef GAME_CORE_STRINGREPRESENTABLE_HPP
#define GAME_CORE_STRINGREPRESENTABLE_HPP

#include <string>

class StringRepresentable
{
public:
    virtual std::wstring toString() const = 0;
    virtual std::wstring getSerializedName() const = 0;
};

#endif
