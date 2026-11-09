#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_NATTRANSPORTADAPTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_NATTRANSPORTADAPTER_HPP

namespace nn
{
namespace nex
{

class StationURL;

class NATTransportAdapter
{
public:
    virtual void Receive(const StationURL& rStationURL, const unsigned char* pData, unsigned int uiSize) = 0;
    virtual void Send(const StationURL& rStationURL, const unsigned char* pData, unsigned int uiSize,
                      unsigned char uiTTL) = 0;
};

}
}

#endif
