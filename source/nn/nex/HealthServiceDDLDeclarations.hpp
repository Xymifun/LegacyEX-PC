#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_HEALTHSERVICEDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_HEALTHSERVICEDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class HealthServiceDDLDeclarations : public DDLDeclarations
{
public:
    HealthServiceDDLDeclarations();

    virtual ~HealthServiceDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<HealthServiceDDLDeclarations> s_declarations;
};

}
}

#endif
