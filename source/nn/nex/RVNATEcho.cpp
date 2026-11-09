#include "nn/nex/RVNATEcho.hpp"

#include "nn/nex/NATEchoStream.hpp"

namespace nn
{
namespace nex
{

bool RVNATEcho::PrepareMessage(Buffer* pRequest, Buffer* pResponse, StationURL* pStationURL)
{
    NATEchoStream::PrepareMessage(pRequest, pResponse);
    *pStationURL = m_streamURL;
    return true;
}

void RVNATEcho::SetStreamURL(const StationURL& rStationURL)
{
    m_streamURL.SetURLType(StationURL::Udp);
    m_streamURL.SetInetAddress(rStationURL.GetInetAddress());
}

}
}
