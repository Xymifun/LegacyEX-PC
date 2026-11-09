#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CALL_CONTEXT_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_CALL_CONTEXT_HPP

#include "nn/pia/Common.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class CallContext
{
public:
    enum State
    {
        cStateNotCalled = 0,
        cStateInProgress = 1,
        cStateSuccess = 2,
        cStateFailure = 3,
        cStateCancelled = 4
    };

    typedef void (*CompletionCallback)(Result result, void* param);

    CallContext();
    ~CallContext();
    void Reset();
    void RegisterCompletionCallback(CompletionCallback callback, void* param);
    void SetState(State state);
    bool InitiateCall();
    void SetResult(Result result);
    void Complete(Result result);
    void SignalSuccess(Result result);
    void SignalFailure(Result result);
    void SignalCancel();
    void Cancel();

    State m_state;
    Result m_result;
    CompletionCallback m_completionCallback;
    void* m_completionCallbackParam;
    bool m_cancelRequested;
};

}
}
}

#endif
