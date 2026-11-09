#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOGLOBALVARIABLELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOGLOBALVARIABLELIST_HPP

#include "nn/nex/PseudoGlobalVariableRoot.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class PseudoGlobalVariableList : public RootObject
{
public:
    PseudoGlobalVariableList();
    virtual ~PseudoGlobalVariableList();

    void AddVariable(PseudoGlobalVariableRoot* pVariable);
    void RemoveVariable(PseudoGlobalVariableRoot* pVariable);
    void ResetContextForAllVariables(unsigned int uiContextId);
    void AllocateExtraContextsForAllVariables();
    void FreeExtraContextsForAllVariables();

    static unsigned int FindVariableIndex(PseudoGlobalVariableRoot* pVariable);
    static PseudoGlobalVariableRoot* GetVariable(unsigned int uiIndex);

    static unsigned int GetNbOfVariables() { return s_uiNbOfVariables; }

private:
    static PseudoGlobalVariableRoot* s_pFirst;
    static unsigned int s_uiNbOfVariables;
};

extern PseudoGlobalVariableList g_pseudoGlobalVariableList;

}
}

#endif
