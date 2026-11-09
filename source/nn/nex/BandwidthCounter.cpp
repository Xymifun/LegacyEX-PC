#include "nn/nex/BandwidthCounter.hpp"

#include <algorithm>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/qChain.hpp"

namespace nn
{
namespace nex
{

namespace
{

typedef BandwidthCounter::CounterList CounterChain;
CounterChain g_bandwidthCounters;
CriticalSection g_bandwidthCounterLock;

}

BandwidthCounter::CounterList* BandwidthCounter::GetList()
{
    return &g_bandwidthCounters;
}

CriticalSection* BandwidthCounter::GetCriticalSection()
{
    return &g_bandwidthCounterLock;
}

BandwidthCounter::BandwidthCounter(const String& name)
    : m_link(),
      m_name(name),
      m_minimum(0xffffffff),
      m_maximum(0),
      m_total(0),
      m_count(0)
{
    ScopedCS lock(&g_bandwidthCounterLock);
    g_bandwidthCounters.push_back(this);
}

BandwidthCounter::~BandwidthCounter()
{
    ScopedCS lock(&g_bandwidthCounterLock);
    CounterChain::iterator position = std::find(g_bandwidthCounters.begin(), g_bandwidthCounters.end(), this);
    if (position != g_bandwidthCounters.end())
        g_bandwidthCounters.erase(position);
}

void BandwidthCounter::Reset()
{
    m_minimum = 0xffffffff;
    m_maximum = 0;
    m_total = 0;
    m_count = 0;
}

void BandwidthCounter::ResetAll()
{
    ScopedCS lock(&g_bandwidthCounterLock);
    for (CounterChain::iterator it = g_bandwidthCounters.begin(); it != g_bandwidthCounters.end(); ++it)
        (*it)->Reset();
}

void BandwidthCounter::Log()
{
}

void BandwidthCounter::LogAll(bool activeOnly)
{
    ScopedCS lock(&g_bandwidthCounterLock);
    for (CounterChain::iterator it = g_bandwidthCounters.begin(); it != g_bandwidthCounters.end(); ++it)
    {
        if (!activeOnly || (*it)->m_count != 0)
            (*it)->Log();
    }
}

void BandwidthCounter::operator+=(unsigned int size)
{
    m_total += size;
    ++m_count;
    if (size < m_minimum)
        m_minimum = size;
    if (size > m_maximum)
        m_maximum = size;
}

void BandwidthCounter::Trace(unsigned long long)
{
}

}
}
