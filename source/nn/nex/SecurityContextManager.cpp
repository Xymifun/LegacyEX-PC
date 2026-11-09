#include "nn/nex/SecurityContextManager.hpp"

#include "nn/nex/Core.hpp"

namespace nn
{
namespace nex
{

SecurityContext::~SecurityContext()
{
}

SecurityContextManager::SecurityContextManager() : m_securityContexts(SecurityContextList())
{
}

SecurityContextManager* SecurityContextManager::GetInstance()
{
    Core* pCore = Core::GetInstance();
    if (pCore != 0)
        return pCore->GetSecurityContextManager();

    return 0;
}

void SecurityContextManager::Push(unsigned int uiPID, unsigned int uiCID, unsigned int uiAddress)
{
    m_securityContexts.GetRef().push_front(SecurityContext(uiPID, uiCID, uiAddress));
}

void SecurityContextManager::Pop()
{
    SecurityContextList& rSecurityContexts = m_securityContexts.GetRef();
    rSecurityContexts.erase(rSecurityContexts.begin());
}

unsigned int SecurityContextManager::GetCurrentPID() const
{
    if (m_securityContexts.GetRef().size() == 0)
        return 0;

    return m_securityContexts.GetRef().front().GetPID();
}

unsigned int SecurityContextManager::GetCurrentCID() const
{
    if (m_securityContexts.GetRef().size() == 0)
        return 0;

    return m_securityContexts.GetRef().front().GetCID();
}

unsigned int SecurityContextManager::GetCurrentAddress() const
{
    if (m_securityContexts.GetRef().size() == 0)
        return 0;

    return m_securityContexts.GetRef().front().GetAddress();
}

unsigned int SecurityContextManager::StaticGetCurrentPID()
{
    SecurityContextManager* pManager = GetInstance();
    if (pManager == 0)
        return 0;

    return pManager->GetCurrentPID();
}

unsigned int SecurityContextManager::StaticGetCurrentCID()
{
    SecurityContextManager* pManager = GetInstance();
    if (pManager == 0)
        return 0;

    return pManager->GetCurrentCID();
}

}
}
