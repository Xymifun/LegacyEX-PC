#include "nn/nex/NintendoAuthenticationDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

NintendoAuthenticationDDLDeclarations::NintendoAuthenticationDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<NintendoAuthenticationDDLDeclarations> NintendoAuthenticationDDLDeclarations::s_declarations;

void NintendoAuthenticationDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
