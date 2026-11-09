#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_IOCOMPLETIONNOTIFIER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_IOCOMPLETIONNOTIFIER_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/IOCompletionContext.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class IOCompletionNotifier : public RootObject
{
public:
    enum
    {
        MAX_CONTEXTS = 8
    };

    IOCompletionNotifier();
    ~IOCompletionNotifier();

    IOCompletionContext* CreateIOCompletionContext();
    void DeleteIOCompletionContext(IOCompletionContext* pContext);
    bool WaitForPollIOCompletion(unsigned int uiTimeout);
    bool WaitForIOCompletion(unsigned int uiTimeout);

private:
    CriticalSection m_criticalSection;
    IOCompletionContext* m_aContexts[MAX_CONTEXTS + 1];
};

}
}

#endif
