#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_BANDWIDTHCOUNTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_BANDWIDTHCOUNTER_HPP

#include <stdint.h>
#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qChain.hpp"
#include <list>

namespace nn
{
namespace nex
{

class CriticalSection;

class BandwidthCounter : public RootObject
{
public:
    typedef qChain<BandwidthCounter*, DefaultChainPolicy<BandwidthCounter*> > CounterList;

    explicit BandwidthCounter(const String& name);
    virtual ~BandwidthCounter();

    void Reset();
    static void ResetAll();
    void Log();
    static void LogAll(bool activeOnly);
    void operator+=(unsigned int size);
    void Trace(unsigned long long flags);

    static CounterList* GetList();
    static CriticalSection* GetCriticalSection();

private:
    friend class DefaultChainPolicy<BandwidthCounter*>;
    qChainLink<BandwidthCounter*> m_link;
    String m_name;
    unsigned int m_minimum;
    unsigned int m_maximum;
    unsigned int m_total;
    unsigned int m_count;
};

}
}

#endif
