#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_INSTANCETABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_INSTANCETABLE_HPP

#include "nn/nex/InstantiationContext.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/SystemError.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class InstanceControl;
class StepSequenceJob;

class InstanceTable : public RootObject
{
public:
    typedef qVector<InstantiationContext*> ContextVector;

    InstanceTable();
    virtual ~InstanceTable();

    int GetHighestID() const;

    bool AddInstance(InstanceControl* instance, unsigned int classId, unsigned int contextId);
    bool AddInstanceToVector(InstanceControl* instance, unsigned int classId, unsigned int contextId);
    void DelInstance(InstanceControl* instance, unsigned int classId, unsigned int contextId);
    void DelInstanceFromVector(InstanceControl* instance, unsigned int classId, unsigned int contextId);

    unsigned int FindInstanceContext(InstanceControl* instance, unsigned int classId);

    unsigned int CreateContext();
    bool DeleteContext(unsigned int contextId);

    void AllocateExtraContexts(unsigned int contextCount);
    void FreeExtraContexts();

    bool ContextIsValid(unsigned int contextId) const
    {
        if (contextId == 0)
            return true;

        if (contextId >= m_extraContexts->size())
            return false;

        if ((*m_extraContexts)[contextId] != 0)
            return true;

        return false;
    }

    InstanceControl* FindInstance(unsigned int classId, unsigned int contextId)
    {
        if (contextId == 0)
            return m_defaultContext.GetInstance(classId);

        if (contextId >= m_extraContexts->size())
        {
            SystemError::SignalError(
                L"D:\\home\\jenkins\\workspace\\Master\\Pack\\d4f027b8\\.\\OnlineCore/src/Core/InstanceTable.h", 0x48,
                0xE0000003, 0);
            return reinterpret_cast<InstanceControl*>(0xFFFFFFFF);
        }

        return (*m_extraContexts)[contextId]->GetInstance(classId);
    }

private:
    friend class StepSequenceJob;

    InstantiationContext m_defaultContext;
    ContextVector* m_extraContexts;
};

extern InstanceTable g_instanceTable;

}
}

#endif
