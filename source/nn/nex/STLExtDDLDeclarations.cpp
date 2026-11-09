#include "nn/nex/STLExtDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

STLExtDDLDeclarations::STLExtDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<STLExtDDLDeclarations> STLExtDDLDeclarations::s_declarations;

void STLExtDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
