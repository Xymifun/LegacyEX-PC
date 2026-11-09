#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKINGSERVICEDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKINGSERVICEDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class MatchMakingServiceDDLDeclarations : public DDLDeclarations
{
public:
    MatchMakingServiceDDLDeclarations();

    virtual ~MatchMakingServiceDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<MatchMakingServiceDDLDeclarations> s_declarations;
};

}
}

#endif
