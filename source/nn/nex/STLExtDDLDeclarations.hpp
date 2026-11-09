#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_STLEXTDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_STLEXTDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class STLExtDDLDeclarations : public DDLDeclarations
{
public:
    STLExtDDLDeclarations();

    virtual ~STLExtDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<STLExtDDLDeclarations> s_declarations;
};

}
}

#endif
