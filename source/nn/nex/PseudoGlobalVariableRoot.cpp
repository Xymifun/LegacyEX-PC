#include "nn/nex/PseudoGlobalVariableRoot.hpp"

#include "nn/nex/PseudoGlobalVariableList.hpp"

namespace nn
{
namespace nex
{

unsigned int PseudoGlobalVariableRoot::s_uiNbOfExtraContexts = 1;

PseudoGlobalVariableRoot::PseudoGlobalVariableRoot() : m_pNext(0)
{
}

PseudoGlobalVariableRoot::~PseudoGlobalVariableRoot()
{
}

void PseudoGlobalVariableRoot::ResetContextForAllVariables(unsigned int uiContextId)
{
    g_pseudoGlobalVariableList.ResetContextForAllVariables(uiContextId);
}

void PseudoGlobalVariableRoot::FreeExtraContextsForAllVariables()
{
    g_pseudoGlobalVariableList.FreeExtraContextsForAllVariables();
    s_uiNbOfExtraContexts = 1;
}

}
}
