#include "nn/pia/ProtocolMessageFilteringManager.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/Transport.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>
#include "nn/pia/transport/TransportSupport.hpp"

namespace nn
{
namespace pia
{
namespace transport
{


ProtocolMessageFilteringManager* g_piaProtocolMessageFilteringManager = 0;

namespace
{
unsigned char* g_filteringManagerStorage = 0;
}

ProtocolMessageFilteringManager::ProtocolMessageFilteringManager() : m_types(), m_count(0)
{
}

ProtocolMessageFilteringManager::~ProtocolMessageFilteringManager()
{
}

Result ProtocolMessageFilteringManager::CreateInstance()
{
    if (IsInitialized() == 0)
        return cResultNotInitialized;
    if (IsInSetupMode() == 0)
        return cResultInvalidState;
    if (g_piaProtocolMessageFilteringManager != 0)
        return cResultAlreadyExists;
    g_filteringManagerStorage = common::NewArray<unsigned char>(sizeof(ProtocolMessageFilteringManager), 8);
    if (g_filteringManagerStorage == 0)
        return cResultOutOfMemory;
    g_piaProtocolMessageFilteringManager = new (g_filteringManagerStorage) ProtocolMessageFilteringManager();
    return 0;
}

void ProtocolMessageFilteringManager::DestroyInstance()
{
    if (g_piaProtocolMessageFilteringManager != 0)
    {
        g_piaProtocolMessageFilteringManager->~ProtocolMessageFilteringManager();
        g_piaProtocolMessageFilteringManager = 0;
    }
    if (g_filteringManagerStorage != 0)
    {
        common::DeleteArray<unsigned char>(g_filteringManagerStorage);
        g_filteringManagerStorage = 0;
    }
}

unsigned int ProtocolMessageFilteringManager::IsFilteringEnabled(unsigned char protocolType) const
{
    for (int index = 0; index < m_count; ++index)
    {
        if (protocolType == m_types[index])
            return 0;
    }
    return 1;
}

unsigned int ProtocolMessageFilteringManager::AddNoFilteringProtocolType(unsigned char protocolType)
{
    if (m_count >= cNoFilteringProtocolTypeMax)
        return 0;
    m_types[m_count++] = protocolType;
    return 1;
}

unsigned int ProtocolMessageFilteringManager::RemoveNoFilteringProtocolType(unsigned char protocolType)
{
    if (m_count == 0)
        return 0;
    int index;
    for (index = 0; index < m_count; ++index)
    {
        if (m_types[index] == protocolType)
            break;
    }
    if (index == m_count)
        return 0;
    for (; index < m_count - 1; ++index)
        m_types[index] = m_types[index + 1];
    --m_count;
    return 1;
}


}
}
}
