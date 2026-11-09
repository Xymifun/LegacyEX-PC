#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_INSTANCECONTROL_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_INSTANCECONTROL_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class InstanceControl : public RootObject
{
public:
    InstanceControl(unsigned int classId, unsigned int contextId);
    virtual ~InstanceControl();

    void SetDelegatorInstance(void* delegatorInstance);

    void* GetDelegatorInstance() const { return m_delegatorInstance; }

    static int GetHighestID();
    static bool ContextIsValid(unsigned int contextId);
    static void AllocateExtraContexts(unsigned int contextCount);
    static void FreeExtraContexts();
    static unsigned int CreateContext();
    static bool DeleteContext(unsigned int contextId);
    static unsigned int FindInstanceContext(InstanceControl* instance, unsigned int classId);

protected:
    unsigned int m_contextId;
    unsigned int m_classId;
    void* m_delegatorInstance;
    bool m_registered;
};

extern CriticalSection g_instanceTableCriticalSection;

}
}

#endif
