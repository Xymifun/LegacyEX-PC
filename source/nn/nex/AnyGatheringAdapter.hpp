#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ANYGATHERINGADAPTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ANYGATHERINGADAPTER_HPP

#include "nn/nex/AnyObjectAdapter.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Gathering.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class AnyGatheringAdapter : public AnyObjectAdapter<Gathering, String>
{
public:
    AnyGatheringAdapter();
    virtual ~AnyGatheringAdapter();

    virtual void StreamIn(Message* pMessage, const Gathering& rGathering) const;
    virtual void StreamOut(Message* pMessage, Gathering** ppGathering) const;

    Gathering* FindPrototype(const String& rType) const;
    bool RegisterPrototype(Gathering* pPrototype);
    bool UnregisterPrototype(const String& rType);
    void UnregisterAllPrototypes();
    Gathering* CreateObject(const String& rType) const;

    static unsigned int RegisterUse();
    static unsigned int UnregisterUse();

private:
    friend class MatchMakingClient;

    typedef qList<Gathering*> PrototypeList;

    unsigned int m_uiUseCount;

    static AnyGatheringAdapter s_instance;
    static PrototypeList* s_pPrototypes;
    static CriticalSection s_criticalSection;
};

}
}

#endif
