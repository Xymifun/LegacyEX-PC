#include "nn/nex/PseudoSingleton.hpp"

namespace nn
{
namespace nex
{

bool g_usingInstantiationContext = false;
ThreadVariable<unsigned int>* g_instantiationContextVariable = 0;

PseudoSingleton::PseudoSingleton(unsigned int classId)
    : InstanceControl(classId, g_usingInstantiationContext ? g_instantiationContextVariable->GetRef() : 0)
{
}

PseudoSingleton::~PseudoSingleton()
{
}

bool PseudoSingleton::SetContext(unsigned int contextId)
{
    if (!g_usingInstantiationContext)
        return false;

    g_instantiationContextVariable->SetValue(contextId);
    return true;
}

void PseudoSingleton::SetContextIfRequired(unsigned int contextId)
{
    if (!g_usingInstantiationContext)
        return;

    SetContext(contextId);
}

}
}
