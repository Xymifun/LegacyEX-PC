#include "nn/nex/MatchmakeExtensionDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

MatchmakeExtensionDDLDeclarations::MatchmakeExtensionDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<MatchmakeExtensionDDLDeclarations> MatchmakeExtensionDDLDeclarations::s_declarations;

void MatchmakeExtensionDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
