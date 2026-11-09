#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_AUTHENTICATIONFOUNDATIONDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_AUTHENTICATIONFOUNDATIONDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class AuthenticationFoundationDDLDeclarations : public DDLDeclarations
{
public:
    AuthenticationFoundationDDLDeclarations();

    virtual ~AuthenticationFoundationDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<AuthenticationFoundationDDLDeclarations> s_declarations;
};

}
}

#endif
