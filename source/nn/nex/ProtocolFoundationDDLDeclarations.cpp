#include "nn/nex/ProtocolFoundationDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

ProtocolFoundationDDLDeclarations::ProtocolFoundationDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<ProtocolFoundationDDLDeclarations> ProtocolFoundationDDLDeclarations::s_declarations;

void ProtocolFoundationDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
