#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRAVERSALDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class NATTraversalDDLDeclarations : public DDLDeclarations
{
public:
    NATTraversalDDLDeclarations();

    virtual ~NATTraversalDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<NATTraversalDDLDeclarations> s_declarations;
};

}
}

#endif
