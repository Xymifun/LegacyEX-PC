#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_ANYDATAADAPTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_ANYDATAADAPTER_HPP

#include "nn/nex/AnyObjectAdapter.hpp"
#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/Data.hpp"
#include "nn/nex/String.hpp"
#include "nn/nex/qList.hpp"

namespace nn
{
namespace nex
{

class AnyDataAdapter : public AnyObjectAdapter<Data, String>
{
public:
    AnyDataAdapter();
    virtual ~AnyDataAdapter();

    virtual void StreamIn(Message* pMessage, const Data& rData) const;
    virtual void StreamOut(Message* pMessage, Data** ppData) const;

    Data* FindPrototype(const String& rType) const;
    bool RegisterPrototype(Data* pPrototype);
    bool UnregisterPrototype(const String& rType);
    void UnregisterAllPrototypes();
    Data* CreateObject(const String& rType, bool bCompactFormat) const;

    static void Register();
    static void Unregister();

private:
    typedef qList<Data*> PrototypeList;

    static AnyDataAdapter s_instance;
    static unsigned int s_uiUseCount;
    static PrototypeList* s_pPrototypes;
    static CriticalSection s_criticalSection;
};

}
}

#endif
