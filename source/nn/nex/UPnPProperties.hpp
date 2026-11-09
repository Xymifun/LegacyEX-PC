#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_UPNPPROPERTIES_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_UPNPPROPERTIES_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class UPnPProperties : public RootObject
{
public:
    UPnPProperties();
    virtual ~UPnPProperties();

    void SetUPnPPortOpen(bool open);
    bool IsUPnPPortOpen() const;
    void SetMappedPort(unsigned short port);
    unsigned short GetMappedPort() const;

private:
    bool m_portOpen;
    unsigned short m_mappedPort;
};

}
}

#endif
