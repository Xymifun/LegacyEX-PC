#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BANDWIDTHCOUNTERMAP_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BANDWIDTHCOUNTERMAP_HPP

#include <map>

#include "nn/nex/BandwidthCounter.hpp"
#include "nn/nex/MemAllocator.hpp"

namespace nn
{
namespace nex
{

class BandwidthCounterMap : public RootObject
{
public:
    class IOBandwidthCounter : public RootObject
    {
    public:
        explicit IOBandwidthCounter(const String& name);

        BandwidthCounter m_incoming;
        BandwidthCounter m_outgoing;
    };

    explicit BandwidthCounterMap(const String& name);
    virtual ~BandwidthCounterMap();
    IOBandwidthCounter* operator[](unsigned int id);

private:
    typedef std::map<unsigned int, IOBandwidthCounter*, std::less<unsigned int>,
                     MemAllocator<std::pair<const unsigned int, IOBandwidthCounter*> > >
        CounterMap;

    String m_name;
    CounterMap m_counters;
};

}
}

#endif
