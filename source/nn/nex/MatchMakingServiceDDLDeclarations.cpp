#include "nn/nex/MatchMakingServiceDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

MatchMakingServiceDDLDeclarations::MatchMakingServiceDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<MatchMakingServiceDDLDeclarations> MatchMakingServiceDDLDeclarations::s_declarations;

void MatchMakingServiceDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
