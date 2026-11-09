#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_INTERFACETABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_INTERFACETABLE_HPP

#include "nn/nex/InterfaceInfo.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class InterfaceTable : public RootObject
{
public:
    InterfaceTable();
    ~InterfaceTable();

    void cleanup();
    bool Query();
    unsigned int GetNbInterface();
    InterfaceInfo& operator[](unsigned int uiIndex);

    unsigned int GetNbLoopbackInterface()
    {
        unsigned int uiCount = 0;
        for (unsigned int i = 0; i < m_uiCount; ++i)
        {
            if ((m_pInterfaces[i].GetFlags() & InterfaceInfo::FLAG_LOOPBACK) != 0)
                ++uiCount;
        }
        return uiCount;
    }

    void Trace(unsigned long long uiFilter)
    {
        for (unsigned int i = 0; i < m_uiCount; ++i)
            m_pInterfaces[i].Trace(uiFilter);
    }

private:
    unsigned int m_uiCount;
    InterfaceInfo* m_pInterfaces;
};

}
}

#endif
