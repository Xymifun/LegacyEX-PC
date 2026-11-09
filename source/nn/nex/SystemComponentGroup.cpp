#include "nn/nex/SystemComponentGroup.hpp"

#include <algorithm>
#include <cwchar>

namespace nn
{
namespace nex
{

SystemComponentGroup::SystemComponentGroup(const String& rName) : SystemComponent(rName)
{
}

SystemComponentGroup::~SystemComponentGroup()
{
    UnregisterAllComponents();
}

const wchar_t* SystemComponentGroup::GetType() const
{
    return L"SystemComponentGroup";
}

bool SystemComponentGroup::IsAKindOf(const wchar_t* pType) const
{
    return std::wcscmp(L"SystemComponentGroup", pType) == 0 || SystemComponent::IsAKindOf(pType);
}

void SystemComponentGroup::EnforceDeclareSysComponentMacro()
{
}

void SystemComponentGroup::OnInitialize()
{
}

void SystemComponentGroup::OnTerminate()
{
}

bool SystemComponentGroup::BeginInitialization()
{
    DoWork();
    return GetComponentsState() == STATE_READY;
}

bool SystemComponentGroup::EndInitialization()
{
    DoWork();
    return GetComponentsState() == STATE_READY;
}

bool SystemComponentGroup::BeginTermination()
{
    DoWork();
    return GetComponentsState() == STATE_TERMINATED;
}

bool SystemComponentGroup::EndTermination()
{
    DoWork();
    return GetComponentsState() == STATE_TERMINATED;
}

SystemComponent::_State SystemComponentGroup::TestState()
{
    DoWork();
    return static_cast<_State>(m_uiState);
}

void SystemComponentGroup::DoWork()
{
    if (m_uiState == STATE_TERMINATING)
    {
        if (GetComponentsState() == STATE_TERMINATED || m_components.size() == 0)
        {
            SetState(STATE_TERMINATED, false);
        }
        else
        {
            for (ComponentList::iterator component = m_components.begin(); component != m_components.end(); component++)
            {
                if ((*component)->m_uiState != STATE_TERMINATED)
                    (*component)->Terminate();
            }
        }
    }

    if (m_uiState == STATE_INITIALIZING)
    {
        if (GetComponentsState() == STATE_READY || m_components.size() == 0)
        {
            SetState(STATE_READY, false);
        }
        else
        {
            for (ComponentList::iterator component = m_components.begin(); component != m_components.end(); component++)
            {
                if ((*component)->m_uiState != STATE_READY)
                    (*component)->Initialize();
            }
        }
    }
}

bool SystemComponentGroup::RegisterComponent(SystemComponent* pComponent)
{
    m_components.insert(m_components.end(), pComponent);
    pComponent->SetParent(this);
    TestState();
    return true;
}

bool SystemComponentGroup::UnregisterComponent(SystemComponent* pComponent)
{
    ComponentList::iterator component = std::find(m_components.begin(), m_components.end(), pComponent);
    if (component != m_components.end())
    {
        m_components.erase(component);
        pComponent->ReleaseRef();
        return true;
    }

    return false;
}

bool SystemComponentGroup::UnregisterAllComponents()
{
    while (m_components.size() != 0)
        UnregisterComponent(m_components.front());

    return true;
}

unsigned int SystemComponentGroup::GetComponentsStates() const
{
    unsigned int uiStates = 0;

    for (ComponentList::const_iterator component = m_components.begin(); component != m_components.end(); ++component)
    {
        uiStates |= (*component)->TestState();
    }

    return uiStates;
}

SystemComponent::_State SystemComponentGroup::GetComponentsState() const
{
    const unsigned int uiStates = GetComponentsStates();

    _State state;
    if (m_iInUseCount != 0)
        state = ApplyOwnUse(ComputeGroupState(uiStates));
    else
        state = ComputeGroupState(uiStates);

    return state;
}

SystemComponent::_State SystemComponentGroup::ApplyOwnUse(_State state)
{
    switch (state)
    {
    case STATE_READY:
    case STATE_IN_USE:
        return STATE_IN_USE;
    case STATE_TERMINATING:
    case STATE_TERMINATION_PENDING:
        return STATE_TERMINATION_PENDING;
    default:
        return STATE_TERMINATION_FAILED;
    }
}

SystemComponent::_State SystemComponentGroup::ComputeGroupState(unsigned int uiStates)
{
    if ((uiStates & STATE_INITIALIZATION_FAILED) != 0)
        return STATE_INITIALIZATION_FAILED;

    if ((uiStates & STATE_TERMINATION_FAILED) == 0)
    {
        if (uiStates == STATE_CREATED)
            return STATE_CREATED;

        if (uiStates == STATE_TERMINATED)
            return STATE_TERMINATED;

        if ((uiStates | (STATE_READY | STATE_IN_USE)) == (STATE_READY | STATE_IN_USE))
            return STATE_READY;

        if ((uiStates | (STATE_CREATED | STATE_INITIALIZING | STATE_READY | STATE_IN_USE))
            == (STATE_CREATED | STATE_INITIALIZING | STATE_READY | STATE_IN_USE))
            return STATE_INITIALIZING;

        if ((uiStates | (STATE_READY | STATE_IN_USE | STATE_TERMINATING | STATE_TERMINATION_PENDING | STATE_TERMINATED))
            == (STATE_READY | STATE_IN_USE | STATE_TERMINATING | STATE_TERMINATION_PENDING | STATE_TERMINATED))
            return STATE_TERMINATING;
    }

    return STATE_TERMINATION_FAILED;
}

}
}
