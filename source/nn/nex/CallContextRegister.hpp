#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CALLCONTEXTREGISTER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CALLCONTEXTREGISTER_HPP

#include <functional>
#include <map>
#include <utility>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/SystemComponent.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class CallContext;
class PeriodicJob;

class CallContextRegister : public SystemComponent
{
public:
    typedef std::map<unsigned int, CallContext*, std::less<unsigned int>,
                     MemAllocator<std::pair<const unsigned int, CallContext*> > >
        CallContextMap;
    typedef std::multimap<unsigned int, unsigned int, std::less<unsigned int>,
                          MemAllocator<std::pair<const unsigned int, unsigned int> > >
        AssociatedIdMap;

    CallContextRegister();
    virtual ~CallContextRegister();

    virtual const wchar_t* GetType() const;
    virtual bool IsAKindOf(const wchar_t* pType) const;
    virtual void EnforceDeclareSysComponentMacro();
    virtual bool BeginInitialization();
    virtual bool BeginTermination();

    unsigned int Register(CallContext* pCallContext);
    void Unregister(unsigned int uiCallId);
    CallContext* GetCallContext(unsigned int uiCallId);

    void AssociateCallContext(unsigned int uiCallId, unsigned int uiAssociatedId);
    void GetAssociatedId(unsigned int uiCallId, qVector<unsigned int>* pAssociatedIds);

    void CancelCallContexts(void* pConnection, unsigned int uiConnectionId);
    void CancelExpiredCalls();
    void CheckExpiredCalls(int iExecutionCount);

    void Trace(unsigned long long ullTraceMask);

    void Start();
    void Stop();

private:
    CallContextMap m_callContexts;
    AssociatedIdMap m_associatedIds;
    unsigned int m_uiNextCallId;
    PeriodicJob* m_pCheckExpiredCallsJob;
};

}
}

#endif
