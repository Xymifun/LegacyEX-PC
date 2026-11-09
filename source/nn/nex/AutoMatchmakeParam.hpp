#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_AUTOMATCHMAKEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_AUTOMATCHMAKEPARAM_HPP

#include "nn/nex/_DDL_AutoMatchmakeParam.hpp"

namespace nn
{
namespace nex
{

class AutoMatchmakeParam : public _DDL_AutoMatchmakeParam
{
public:
    enum
    {
        DEFAULT_PARTICIPATION_COUNT = 1
    };

    AutoMatchmakeParam() { Reset(); }

    virtual ~AutoMatchmakeParam() {}

    void Reset();
};

}
}

#endif
