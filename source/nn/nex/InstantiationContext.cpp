#include "nn/nex/InstantiationContext.hpp"

#include "nn/nex/InstanceControl.hpp"

namespace nn
{
namespace nex
{

InstantiationContext::InstantiationContext() : m_initialized(false)
{
    InitContext();
    m_initialized = true;
}

InstantiationContext::~InstantiationContext()
{
    InitContext();
}

void InstantiationContext::InitContext()
{
    unsigned int classId = 0;
    do
    {
        m_instances[classId] = 0;
        ++classId;
    } while (classId < NumInstances);
}

bool InstantiationContext::AddInstance(InstanceControl* instance, unsigned int classId)
{
    if (classId >= NumInstances)
    {
        SystemError::SignalError(
            L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/InstantiationContext.cpp", 0x3E,
            0xE0000003, 0);
        return false;
    }

    if (m_instances[classId] != 0)
    {
        SystemError::SignalError(
            L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/InstantiationContext.cpp", 0x44,
            0xE000000F, 0);
        return false;
    }

    m_instances[classId] = instance;
    return true;
}

bool InstantiationContext::DelInstance(InstanceControl*, unsigned int classId)
{
    if (classId >= NumInstances)
    {
        SystemError::SignalError(
            L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/InstantiationContext.cpp", 0x55,
            0xE0000003, 0);
        return false;
    }

    m_instances[classId] = 0;
    return true;
}

}
}
