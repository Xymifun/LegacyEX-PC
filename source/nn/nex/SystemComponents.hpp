#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCOMPONENTS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_SYSTEMCOMPONENTS_HPP

#include "nn/nex/SystemComponentGroup.hpp"

namespace nn
{
namespace nex
{

class SystemComponents : public SystemComponentGroup
{
public:
    SystemComponents();
    virtual ~SystemComponents();

    SystemComponentGroup* GetCoreGroup() const { return m_pCoreGroup; }

private:
    unsigned int m_reserved0;
    unsigned int m_reserved1;
    unsigned int m_reserved2;
    SystemComponentGroup* m_pCoreGroup;
};

}
}

#endif
