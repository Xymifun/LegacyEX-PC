#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_UPDATEMATCHMAKESESSIONPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_UPDATEMATCHMAKESESSIONPARAM_HPP

#include "nn/nex/_DDL_UpdateMatchmakeSessionParam.hpp"

namespace nn
{
namespace nex
{

class UpdateMatchmakeSessionParam : public _DDL_UpdateMatchmakeSessionParam
{
public:
    enum
    {
        ATTRIBUTE_COUNT = 6,
        MAX_PROGRESS_SCORE = 100
    };

    UpdateMatchmakeSessionParam() { Reset(); }

    virtual ~UpdateMatchmakeSessionParam() {}

    void Reset();
};

}
}

#endif
