#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOSINGLETON_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOSINGLETON_HPP

#include "nn/nex/InstanceControl.hpp"
#include "nn/nex/ThreadVariable.hpp"

namespace nn
{
namespace nex
{

class PseudoSingleton : public InstanceControl
{
public:
    explicit PseudoSingleton(unsigned int classId);
    virtual ~PseudoSingleton();

    static bool SetContext(unsigned int contextId);
    static void SetContextIfRequired(unsigned int contextId);
    static bool UseInstantiationContext(unsigned int contextCount);
    static bool UseNoInstantiationContext();
    static bool UsingInstantiationContext();
};

extern bool g_usingInstantiationContext;
extern ThreadVariable<unsigned int>* g_instantiationContextVariable;

}
}

#endif
