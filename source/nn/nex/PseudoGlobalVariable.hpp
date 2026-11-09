#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOGLOBALVARIABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOGLOBALVARIABLE_HPP

#include <new>

#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/PseudoGlobalVariableList.hpp"
#include "nn/nex/PseudoGlobalVariableRoot.hpp"
#include "nn/nex/PseudoSingleton.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class PseudoGlobalVariable : public PseudoGlobalVariableRoot
{
public:
    PseudoGlobalVariable() : PseudoGlobalVariableRoot(), m_value(), m_initialValue()
    {
        m_value = m_initialValue;
        m_pExtraContexts = 0;
        g_pseudoGlobalVariableList.AddVariable(this);
        if (GetNbOfExtraContexts() > 1)
            AllocateExtraContexts();
    }

    explicit PseudoGlobalVariable(const T& rInitialValue)
        : PseudoGlobalVariableRoot(),
          m_value(rInitialValue),
          m_initialValue(rInitialValue)
    {
        m_pExtraContexts = 0;
        g_pseudoGlobalVariableList.AddVariable(this);
        if (GetNbOfExtraContexts() > 1)
            AllocateExtraContexts();
    }

    virtual ~PseudoGlobalVariable()
    {
        g_pseudoGlobalVariableList.RemoveVariable(this);
        FreeExtraContexts();
    }

    virtual void AllocateExtraContexts();

    virtual void FreeExtraContexts()
    {
        if (m_pExtraContexts != 0)
        {
            for (unsigned int i = 0; i < GetNbOfExtraContexts(); ++i)
                m_pExtraContexts[i].~T();

            void* pContexts = m_pExtraContexts;
            MemoryManager::Free(pContexts);
            m_pExtraContexts = 0;
        }
    }

    virtual void ResetContext(unsigned int uiContextId) { m_pExtraContexts[uiContextId] = m_initialValue; }

    virtual PseudoGlobalVariableRoot* GetNext() { return m_pNext; }

    virtual void SetNext(PseudoGlobalVariableRoot* pNext) { m_pNext = pNext; }

    T& Get()
    {
        unsigned int uiContextId = g_usingInstantiationContext ? g_instantiationContextVariable->GetRef() : 0;

        if (uiContextId == 0)
            return m_value;

        return m_pExtraContexts[uiContextId];
    }

private:
    T* m_pExtraContexts;
    T m_value;
    T m_initialValue;
};

template <typename T>
void PseudoGlobalVariable<T>::AllocateExtraContexts()
{
    if (GetNbOfExtraContexts() == static_cast<unsigned int>(-1))
        new T[GetNbOfExtraContexts()];

    m_pExtraContexts = static_cast<T*>(MemoryManager::Allocate(GetNbOfExtraContexts() * sizeof(T)));
    for (unsigned int i = 0; i < GetNbOfExtraContexts(); ++i)
        ::new (static_cast<void*>(&m_pExtraContexts[i])) T(m_initialValue);
}

}
}

#endif
