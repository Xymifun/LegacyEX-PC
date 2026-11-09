#include "nn/nex/PseudoGlobalVariableList.hpp"

namespace nn
{
namespace nex
{

PseudoGlobalVariableRoot* PseudoGlobalVariableList::s_pFirst = 0;
unsigned int PseudoGlobalVariableList::s_uiNbOfVariables = 0;

PseudoGlobalVariableList g_pseudoGlobalVariableList;

PseudoGlobalVariableList::PseudoGlobalVariableList()
{
}

PseudoGlobalVariableList::~PseudoGlobalVariableList()
{
}

PseudoGlobalVariableRoot* PseudoGlobalVariableList::GetVariable(unsigned int uiIndex)
{
    PseudoGlobalVariableRoot* pVariable = s_pFirst;
    for (unsigned int i = 0; i < uiIndex; ++i)
        pVariable = pVariable->GetNext();

    return pVariable;
}

unsigned int PseudoGlobalVariableList::FindVariableIndex(PseudoGlobalVariableRoot* pVariable)
{
    PseudoGlobalVariableRoot* pCurrent = s_pFirst;
    unsigned int uiIndex = 0;
    while (pCurrent != 0)
    {
        if (pCurrent == pVariable)
            return uiIndex;

        pCurrent = pCurrent->GetNext();
        ++uiIndex;
    }

    return 0xFFFF;
}

void PseudoGlobalVariableList::AddVariable(PseudoGlobalVariableRoot* pVariable)
{
    if (s_pFirst == 0)
    {
        s_pFirst = pVariable;
        s_uiNbOfVariables = 1;
        return;
    }

    PseudoGlobalVariableRoot* pLast = s_pFirst;
    PseudoGlobalVariableRoot* pNext = pLast->GetNext();
    while (pNext != 0)
    {
        pLast = pLast->GetNext();
        pNext = pLast->GetNext();
    }

    pLast->SetNext(pVariable);
    ++s_uiNbOfVariables;
}

void PseudoGlobalVariableList::RemoveVariable(PseudoGlobalVariableRoot* pVariable)
{
    if (s_pFirst == pVariable)
    {
        s_pFirst = s_pFirst->GetNext();
        pVariable->SetNext(0);
        --s_uiNbOfVariables;
        return;
    }

    PseudoGlobalVariableRoot* pPrevious = 0;
    PseudoGlobalVariableRoot* pCurrent = s_pFirst;
    while (pCurrent != 0)
    {
        if (pCurrent == pVariable)
        {
            pPrevious->SetNext(pCurrent->GetNext());
            pVariable->SetNext(0);
            --s_uiNbOfVariables;
            return;
        }

        pPrevious = pCurrent;
        pCurrent = pCurrent->GetNext();
    }
}

void PseudoGlobalVariableList::ResetContextForAllVariables(unsigned int uiContextId)
{
    for (PseudoGlobalVariableRoot* pVariable = s_pFirst; pVariable != 0; pVariable = pVariable->GetNext())
        pVariable->ResetContext(uiContextId);
}

void PseudoGlobalVariableList::AllocateExtraContextsForAllVariables()
{
    for (PseudoGlobalVariableRoot* pVariable = s_pFirst; pVariable != 0; pVariable = pVariable->GetNext())
        pVariable->AllocateExtraContexts();
}

void PseudoGlobalVariableList::FreeExtraContextsForAllVariables()
{
    for (PseudoGlobalVariableRoot* pVariable = s_pFirst; pVariable != 0; pVariable = pVariable->GetNext())
        pVariable->FreeExtraContexts();
}

}
}
