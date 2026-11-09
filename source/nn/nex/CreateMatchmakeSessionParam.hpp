#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CREATEMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CREATEMATCHMAKESESSIONPARAM_HPP

#include "nn/nex/_DDL_CreateMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

class CreateMatchmakeSessionParam : public _DDL_CreateMatchmakeSessionParam
{
public:
    enum
    {
        DEFAULT_PARTICIPATION_COUNT = 1
    };

    CreateMatchmakeSessionParam() { Reset(); }

    virtual ~CreateMatchmakeSessionParam() {}

    void Reset();
};

}
}

#endif
