#ifndef GAME_CORE_IDSSTRINGBUILDER_HPP
#define GAME_CORE_IDSSTRINGBUILDER_HPP

#include <string>

#include "game/core/StringBuilderCore.hpp"

class IdsStringBuilder : public StringBuilderCore
{
public:
    explicit IdsStringBuilder(int id);
    virtual ~IdsStringBuilder();
    virtual std::wstring getString();

private:
    int id;
};

#endif
