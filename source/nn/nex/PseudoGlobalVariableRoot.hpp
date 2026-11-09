#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOGLOBALVARIABLEROOT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOGLOBALVARIABLEROOT_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class PseudoGlobalVariableRoot : public RootObject
{
public:
    PseudoGlobalVariableRoot();
    virtual ~PseudoGlobalVariableRoot();

    virtual void AllocateExtraContexts() = 0;
    virtual void FreeExtraContexts() = 0;
    virtual void ResetContext(unsigned int uiContextId) = 0;
    virtual PseudoGlobalVariableRoot* GetNext() = 0;
    virtual void SetNext(PseudoGlobalVariableRoot* pNext) = 0;

    static void ResetContextForAllVariables(unsigned int uiContextId);
    static void FreeExtraContextsForAllVariables();

    static unsigned int GetNbOfExtraContexts() { return s_uiNbOfExtraContexts; }

protected:
    PseudoGlobalVariableRoot* m_pNext;

    static unsigned int s_uiNbOfExtraContexts;
};

}
}

#endif
