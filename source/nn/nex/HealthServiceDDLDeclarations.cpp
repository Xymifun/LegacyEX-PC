#include "nn/nex/HealthServiceDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

HealthServiceDDLDeclarations::HealthServiceDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<HealthServiceDDLDeclarations> HealthServiceDDLDeclarations::s_declarations;

void HealthServiceDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
