#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKEEXTENSIONDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_MATCHMAKEEXTENSIONDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class MatchmakeExtensionDDLDeclarations : public DDLDeclarations
{
public:
    MatchmakeExtensionDDLDeclarations();

    virtual ~MatchmakeExtensionDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<MatchmakeExtensionDDLDeclarations> s_declarations;
};

}
}

#endif
