#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_COMPONENTSTATE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_COMPONENTSTATE_HPP

#include "nn/nex/String.hpp"
#include "nn/nex/SystemComponent.hpp"

namespace nn
{
namespace nex
{

class ComponentState : public SystemComponent
{
public:
    explicit ComponentState(const String& rName) : SystemComponent(rName) {}

    virtual ~ComponentState() {}

    bool IsTerminated() const { return m_uiState == STATE_TERMINATED; }

    virtual const wchar_t* GetType() const;
    virtual bool IsAKindOf(const wchar_t* pType) const;
    virtual void EnforceDeclareSysComponentMacro();
};

}
}

#endif
