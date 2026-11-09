#include "nn/nex/NATTraversalDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

NATTraversalDDLDeclarations::NATTraversalDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<NATTraversalDDLDeclarations> NATTraversalDDLDeclarations::s_declarations;

void NATTraversalDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
