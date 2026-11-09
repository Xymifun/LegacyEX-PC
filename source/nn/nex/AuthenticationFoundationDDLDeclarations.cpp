#include "nn/nex/AuthenticationFoundationDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

AuthenticationFoundationDDLDeclarations::AuthenticationFoundationDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<AuthenticationFoundationDDLDeclarations> AuthenticationFoundationDDLDeclarations::s_declarations;

void AuthenticationFoundationDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
