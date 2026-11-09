#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKEPARAM_HPP

#include "nn/nex/_DDL_MatchmakeParam.hpp"

namespace nn
{
namespace nex
{

class MatchmakeParam : public _DDL_MatchmakeParam
{
public:
    MatchmakeParam() { m_Parameters.clear(); }

    virtual ~MatchmakeParam() {}
};

}
}

#endif
