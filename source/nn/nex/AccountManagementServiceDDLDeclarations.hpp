#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTSERVICEDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ACCOUNTMANAGEMENTSERVICEDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class AccountManagementServiceDDLDeclarations : public DDLDeclarations
{
public:
    AccountManagementServiceDDLDeclarations();

    virtual ~AccountManagementServiceDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<AccountManagementServiceDDLDeclarations> s_declarations;
};

}
}

#endif
