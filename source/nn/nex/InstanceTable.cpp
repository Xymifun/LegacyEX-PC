#include "nn/nex/InstanceTable.hpp"

#include "nn/nex/InstanceControl.hpp"

namespace nn
{
namespace nex
{

InstanceTable g_instanceTable;

InstanceTable::InstanceTable() : m_extraContexts(0)
{
}

InstanceTable::~InstanceTable()
{
}

int InstanceTable::GetHighestID() const
{
    if (m_extraContexts == 0)
        return 0;

    return static_cast<int>(m_extraContexts->size()) - 1;
}

bool InstanceTable::AddInstance(InstanceControl* instance, unsigned int classId, unsigned int contextId)
{
    if (contextId == 0)
    {
        if (!m_defaultContext.AddInstance(instance, classId))
            return false;
    }
    else
    {
        if (!AddInstanceToVector(instance, classId, contextId))
            return false;
    }

    return true;
}

bool InstanceTable::AddInstanceToVector(InstanceControl* instance, unsigned int classId, unsigned int contextId)
{
    if (contextId < m_extraContexts->size())
    {
        if ((*m_extraContexts)[contextId] != 0)
        {
            if ((*m_extraContexts)[contextId]->AddInstance(instance, classId))
                return true;
        }
        else
        {
            SystemError::SignalError(
                L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/InstanceTable.cpp", 0x9E,
                0xE0000008, 0);
            return false;
        }
    }

    SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/InstanceTable.cpp",
                             0xA5, 0xE0000003, 0);
    return false;
}

void InstanceTable::DelInstance(InstanceControl* instance, unsigned int classId, unsigned int contextId)
{
    if (contextId == 0)
    {
        m_defaultContext.DelInstance(instance, classId);
        return;
    }

    DelInstanceFromVector(instance, classId, contextId);
}

void InstanceTable::DelInstanceFromVector(InstanceControl* instance, unsigned int classId, unsigned int contextId)
{
    (*m_extraContexts)[contextId]->DelInstance(instance, classId);
}

unsigned int InstanceTable::FindInstanceContext(InstanceControl* instance, unsigned int classId)
{
    if (m_defaultContext.GetInstance(classId) == instance)
        return 0;

    unsigned int contextCount = m_extraContexts->size();
    for (unsigned int contextId = 0; contextId < contextCount; ++contextId)
    {
        if ((*m_extraContexts)[contextId] != 0 && (*m_extraContexts)[contextId]->GetInstance(classId) == instance)
            return contextId;
    }

    SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/InstanceTable.cpp",
                             0xE0, 0xA0000006, 0);
    return 0xFFFF;
}

unsigned int InstanceTable::CreateContext()
{
    unsigned int contextCount = m_extraContexts->size();
    for (unsigned int contextId = 1; contextId < contextCount; ++contextId)
    {
        if ((*m_extraContexts)[contextId] == 0)
        {
            (*m_extraContexts)[contextId] = new InstantiationContext();
            if (!(*m_extraContexts)[contextId]->m_initialized)
                return 0xFFFF;

            return contextId;
        }
    }

    return 0xFFFF;
}

bool InstanceTable::DeleteContext(unsigned int contextId)
{
    if (contextId < m_extraContexts->size())
    {
        delete (*m_extraContexts)[contextId];
        (*m_extraContexts)[contextId] = 0;
        return true;
    }

    SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/InstanceTable.cpp",
                             0x73, 0xE0000003, 0);
    return false;
}

void InstanceTable::AllocateExtraContexts(unsigned int contextCount)
{
    m_extraContexts = new ContextVector(contextCount);
    for (unsigned int contextId = 0; contextId < contextCount; ++contextId)
        (*m_extraContexts)[contextId] = 0;
}

void InstanceTable::FreeExtraContexts()
{
    m_extraContexts->clear();
    delete m_extraContexts;
    m_extraContexts = 0;
}

}
}
