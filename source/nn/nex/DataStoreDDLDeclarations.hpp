#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREDDLDECLARATIONS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_DATASTOREDDLDECLARATIONS_HPP

#include "nn/nex/DDLDeclarations.hpp"

namespace nn
{
namespace nex
{

class DataStoreDDLDeclarations : public DDLDeclarations
{
public:
    DataStoreDDLDeclarations();

    virtual ~DataStoreDDLDeclarations() {}

    virtual void Init() {}

    static void Register();
    static void RegisterParseTree();

private:
    static PseudoGlobalVariable<DataStoreDDLDeclarations> s_declarations;
};

}
}

#endif
