#include "nn/nex/SecureConnectionServiceDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

SecureConnectionServiceDDLDeclarations::SecureConnectionServiceDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<SecureConnectionServiceDDLDeclarations> SecureConnectionServiceDDLDeclarations::s_declarations;

void SecureConnectionServiceDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
