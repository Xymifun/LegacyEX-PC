#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SECURECONNECTIONSERVICEDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SECURECONNECTIONSERVICEDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class SecureConnectionServiceDDLDeclarations : public DDLDeclarations
{
public:
    SecureConnectionServiceDDLDeclarations();

    virtual ~SecureConnectionServiceDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<SecureConnectionServiceDDLDeclarations> s_declarations;
};

}
}

#endif
