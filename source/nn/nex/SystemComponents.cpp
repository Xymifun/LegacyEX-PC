#include "nn/nex/SystemComponents.hpp"

namespace nn
{
namespace nex
{

SystemComponents::SystemComponents() : SystemComponentGroup(String(L"System Components"))
{
    m_reserved0 = 0;
    m_reserved1 = 0;
    m_reserved2 = 0;
    m_pCoreGroup = new SystemComponentGroup(String(L"Core"));
    RegisterComponent(m_pCoreGroup);
}

SystemComponents::~SystemComponents()
{
}

}
}
