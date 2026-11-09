#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_THREADVARIABLELIST_HPP

#include <stdint.h>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qChain.hpp"

namespace nn
{
namespace nex
{

class ThreadVariableRoot;

class ThreadVariableList : public RootObject
{
public:
    ThreadVariableList();
    virtual ~ThreadVariableList();

    void AddVariable(ThreadVariableRoot* variable);
    static ThreadVariableList* GetInstanceRef();
    void ClearValue(uint32_t value);
    void RemoveVariable(ThreadVariableRoot* variable);
    void ResetValues();
    void ClearValue();

private:
    qChain<ThreadVariableRoot*, DefaultChainPolicy<ThreadVariableRoot*> > m_variables;
    ForcedCriticalSection m_criticalSection;
};

}
}

#endif
