#ifndef GAME_CORE_FJSTRINGBUILDER_HPP
#define GAME_CORE_FJSTRINGBUILDER_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/core/StringBuilderCore.hpp"

class FJStringBuilder
{
public:
    FJStringBuilder();
    FJStringBuilder(StringBuilderCore* core);

    std::wstring operator()();

private:
    boost::shared_ptr<StringBuilderCore> mCore;
};

#endif
