#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLFOUNDATIONDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PROTOCOLFOUNDATIONDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class ProtocolFoundationDDLDeclarations : public DDLDeclarations
{
public:
    ProtocolFoundationDDLDeclarations();

    virtual ~ProtocolFoundationDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<ProtocolFoundationDDLDeclarations> s_declarations;
};

}
}

#endif
