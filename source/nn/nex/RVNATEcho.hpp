#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_RVNATECHO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_RVNATECHO_HPP

#include "nn/nex/NATEchoInterface.hpp"
#include "nn/nex/StationURL.hpp"

namespace nn
{
namespace nex
{

class Buffer;

class RVNATEcho : public NATEchoInterface
{
public:
    virtual bool PrepareMessage(Buffer* pRequest, Buffer* pResponse, StationURL* pStationURL);

    void SetStreamURL(const StationURL& rStationURL);

private:
    StationURL m_streamURL;
};

}
}

#endif
