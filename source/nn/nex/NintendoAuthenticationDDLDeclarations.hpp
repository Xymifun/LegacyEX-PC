#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDOAUTHENTICATIONDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NINTENDOAUTHENTICATIONDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class NintendoAuthenticationDDLDeclarations : public DDLDeclarations
{
public:
    NintendoAuthenticationDDLDeclarations();

    virtual ~NintendoAuthenticationDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<NintendoAuthenticationDDLDeclarations> s_declarations;
};

}
}

#endif
