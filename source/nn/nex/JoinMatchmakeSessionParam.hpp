#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_JOINMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_JOINMATCHMAKESESSIONPARAM_HPP

#include "nn/nex/_DDL_JoinMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

class JoinMatchmakeSessionParam : public _DDL_JoinMatchmakeSessionParam
{
public:
    enum
    {
        DEFAULT_PARTICIPATION_COUNT = 1
    };

    JoinMatchmakeSessionParam() { Reset(); }

    virtual ~JoinMatchmakeSessionParam() {}

    void Reset();
};

}
}

#endif
