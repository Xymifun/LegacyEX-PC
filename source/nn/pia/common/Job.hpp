#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_JOB_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_JOB_HPP

#include "nn/pia/ListBase.hpp"
#include "nn/pia/common/Time.hpp"

namespace nn
{
namespace pia
{
namespace common
{

class Job
{
public:
    enum InnerState
    {
        cInnerStateNotEntried = 0,
        cInnerStateForegroundWaiting = 1,
        cInnerStateForegroundReady = 2,
        cInnerStateForegroundRunning = 3,
        cInnerStateBackgroundWaiting = 4,
        cInnerStateBackgroundReady = 5,
        cInnerStateBackgroundRunning = 6,
        cInnerStateSuspended = 7,
        cInnerStateCompleted = 8
    };

    enum State
    {
        cStateNotEntried = 0,
        cStateWaiting = 1,
        cStateSuspended = 2,
        cStateReady = 3,
        cStateRunning = 4,
        cStateCompleted = 5
    };

    enum ExecuteKind
    {
        cExecuteKindContinue = 0,
        cExecuteKindFinished = 1,
        cExecuteKindCancelled = 2,
        cExecuteKindSuspend = 3,
        cExecuteKindWait = 4,
        cExecuteKindNextStep = 5,
        cExecuteKindEntryForeground = 6,
        cExecuteKindEntryBackground = 7
    };

    struct ExecuteResult
    {
        unsigned char m_kind;
        unsigned char m_unset;
        unsigned short m_value;
    };

    static ExecuteResult MakeExecuteResult(ExecuteKind kind, unsigned short value = 0)
    {
        ExecuteResult result;
        result.m_kind = kind;
        result.m_value = value;
        return result;
    }

    Job();
    virtual ~Job();
    virtual void Reset(bool isBackground);
    virtual ExecuteResult ExecuteCore() = 0;
    void Ready(bool isBackground);
    void Resume(bool isBackground);
    State GetState() const;
    bool IsRunning() const;
    bool IsForeground() const;
    bool IsBackground() const;
    void Execute(bool isBackground);
    static const char* GetInnerStateString(InnerState innerState);

    InnerState m_innerState;
    unsigned int m_executeCount;
    Time m_time;
    ListNode m_listNode;
};

}
}
}

#endif
