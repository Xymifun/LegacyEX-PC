#include "nn/nex/ComponentState.hpp"

#include <cwchar>

namespace nn
{
namespace nex
{

const wchar_t* ComponentState::GetType() const
{
    return L"ComponentState";
}

bool ComponentState::IsAKindOf(const wchar_t* pType) const
{
    return std::wcscmp(L"ComponentState", pType) == 0 || SystemComponent::IsAKindOf(pType);
}

void ComponentState::EnforceDeclareSysComponentMacro()
{
}

}
}
