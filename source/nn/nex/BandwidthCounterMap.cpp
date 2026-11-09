#include "nn/nex/BandwidthCounterMap.hpp"

namespace nn
{
namespace nex
{

BandwidthCounterMap::IOBandwidthCounter::IOBandwidthCounter(const String& name)
    : m_incoming(name + L"/Incoming"),
      m_outgoing(name + L"/Outgoing")
{
}

BandwidthCounterMap::BandwidthCounterMap(const String& name) : m_name(name), m_counters()
{
}

BandwidthCounterMap::~BandwidthCounterMap()
{
    for (CounterMap::iterator it = m_counters.begin(); it != m_counters.end(); ++it)
        delete it->second;
    m_counters.clear();
}

BandwidthCounterMap::IOBandwidthCounter* BandwidthCounterMap::operator[](unsigned int id)
{
    CounterMap::iterator found = m_counters.find(id);
    if (found != m_counters.end())
        return found->second;

    String name;
    name.Format(L"%ls %d", static_cast<const wchar_t*>(m_name), id);
    IOBandwidthCounter* counter = new IOBandwidthCounter(name);
    m_counters[id] = counter;
    return counter;
}

}
}
