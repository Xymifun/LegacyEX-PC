#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCOMPONENTGROUP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCOMPONENTGROUP_HPP

#include "nn/nex/String.hpp"
#include "nn/nex/SystemComponent.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class SystemComponentGroup : public SystemComponent
{
public:
    typedef qList<SystemComponent*> ComponentList;

    explicit SystemComponentGroup(const String& rName);
    virtual ~SystemComponentGroup();

    virtual const wchar_t* GetType() const;
    virtual bool IsAKindOf(const wchar_t* pType) const;
    virtual void EnforceDeclareSysComponentMacro();
    virtual void OnInitialize();
    virtual void OnTerminate();
    virtual bool BeginInitialization();
    virtual bool EndInitialization();
    virtual bool BeginTermination();
    virtual bool EndTermination();
    virtual _State TestState();
    virtual void DoWork();

    bool RegisterComponent(SystemComponent* pComponent);
    bool UnregisterComponent(SystemComponent* pComponent);
    bool UnregisterAllComponents();

    SystemComponent* FindComponentByState(_State state);
    SystemComponent* FindComponentByName(const String& rName);

    unsigned int GetComponentsStates() const;
    _State GetComponentsState() const;

    static _State ApplyOwnUse(_State state);
    static _State ComputeGroupState(unsigned int uiStates);

private:
    ComponentList m_components;
};

}
}

#endif
