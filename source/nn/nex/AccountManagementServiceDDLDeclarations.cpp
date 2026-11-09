#include "nn/nex/AccountManagementServiceDDLDeclarations.hpp"

namespace nn
{
namespace nex
{

AccountManagementServiceDDLDeclarations::AccountManagementServiceDDLDeclarations() : DDLDeclarations(true)
{
}

PseudoGlobalVariable<AccountManagementServiceDDLDeclarations> AccountManagementServiceDDLDeclarations::s_declarations;

void AccountManagementServiceDDLDeclarations::Register()
{
    s_declarations.Get().RegisterIfRequired();
}

}
}
