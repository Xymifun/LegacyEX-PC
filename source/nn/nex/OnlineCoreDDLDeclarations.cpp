#include "nn/nex/OnlineCoreDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

OnlineCoreDDLDeclarations::OnlineCoreDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<OnlineCoreDDLDeclarations> OnlineCoreDDLDeclarations::s_declarations;

void OnlineCoreDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
