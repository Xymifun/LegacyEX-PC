#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_URLPROBELIST_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_URLPROBELIST_HPP

#include "nn/nex/qList.hpp"
#include "nn/nex/URLProbe.hpp"

namespace nn
{
namespace nex
{

class URLProbeList : public qList<URLProbe>
{
public:
    URLProbeList();
    virtual ~URLProbeList();

    URLProbe* FindProbe(const StationURL& rStationURL);
    URLProbe* FindProbe(unsigned int uiConnectionID);
};

}
}

#endif
