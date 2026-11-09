#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALNATDEVICE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALNATDEVICE_HPP

#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class StationURL;

class VirtualNATDevice : public RootObject
{
public:
    virtual ~VirtualNATDevice() {}

    virtual bool ProcessOutgoingAddress(StationURL* pURL, unsigned short uiPort) = 0;
    virtual bool ProcessIncomingAddress(StationURL* pURL, unsigned short uiPort) = 0;
};

}
}

#endif
