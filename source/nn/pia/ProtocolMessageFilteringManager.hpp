#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_PROTOCOL_MESSAGE_FILTERING_MANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_PROTOCOL_MESSAGE_FILTERING_MANAGER_HPP

#include <stdint.h>
#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace transport
{


class ProtocolMessageFilteringManager
{
public:
    ProtocolMessageFilteringManager();
    ~ProtocolMessageFilteringManager();
    static Result CreateInstance();
    static void DestroyInstance();
    unsigned int IsFilteringEnabled(unsigned char protocolType) const;
    unsigned int AddNoFilteringProtocolType(unsigned char protocolType);
    unsigned int RemoveNoFilteringProtocolType(unsigned char protocolType);
    void Trace(unsigned long long value) const;

private:
    enum
    {
        cNoFilteringProtocolTypeMax = 32
    };

    unsigned char m_types[cNoFilteringProtocolTypeMax];
    int m_count;
};

extern ProtocolMessageFilteringManager* g_piaProtocolMessageFilteringManager;

}
}
}

#endif
