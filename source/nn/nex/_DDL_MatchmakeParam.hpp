#ifndef LEGACY_CONSOLE_WIIU_NN_NEX__DDL_MATCHMAKEPARAM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX__DDL_MATCHMAKEPARAM_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/Variant.hpp"
#include "nn/nex/qMap.hpp"

namespace nn
{
namespace nex
{

class Message;

class _DDL_MatchmakeParam : public RootObject
{
public:
    _DDL_MatchmakeParam() : m_uiStructureVersion(0) {}

    virtual ~_DDL_MatchmakeParam() {}

    _DDL_MatchmakeParam& operator=(const _DDL_MatchmakeParam& rSource);

    static void Add(Message* pMessage, const _DDL_MatchmakeParam& rSource);
    static void Extract(Message* pMessage, _DDL_MatchmakeParam* pDestination);

    uint8_t m_uiStructureVersion;
    qMap<String, Variant> m_Parameters;
};

}
}

#endif
