#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ONLINECOREDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ONLINECOREDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class OnlineCoreDDLDeclarations : public DDLDeclarations
{
public:
    OnlineCoreDDLDeclarations();

    virtual ~OnlineCoreDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<OnlineCoreDDLDeclarations> s_declarations;
};

}
}

#endif
