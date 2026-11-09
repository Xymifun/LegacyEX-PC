#include "nn/nex/URLProbeList.hpp"

namespace nn
{
namespace nex
{

URLProbeList::URLProbeList()
{
}

URLProbeList::~URLProbeList()
{
}

URLProbe* URLProbeList::FindProbe(const StationURL& rStationURL)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        URLProbe* pProbe = &*it;
        if (String::IsEqual(pProbe->m_stationURL.GetAddress(), rStationURL.GetAddress())
            && pProbe->m_stationURL.GetRVConnectionID() == rStationURL.GetRVConnectionID())
            return pProbe;
    }
    return 0;
}

URLProbe* URLProbeList::FindProbe(unsigned int uiConnectionID)
{
    for (iterator it = begin(); it != end(); ++it)
    {
        URLProbe* pProbe = &*it;
        if (pProbe->m_stationURL.GetRVConnectionID() == uiConnectionID)
            return pProbe;
    }
    return 0;
}

}
}
