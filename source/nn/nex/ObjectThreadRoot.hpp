#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_OBJECTTHREADROOT_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_OBJECTTHREADROOT_HPP

#include <stdint.h>

#include "nn/nex/RootObject.hpp"
#include "nn/nex/String.hpp"

struct OSThread;

namespace nn
{
namespace nex
{

class ObjectThreadRoot : public RootObject
{
public:
    ObjectThreadRoot(const String& name);
    ObjectThreadRoot(const String& name, uint32_t priority);
    virtual ~ObjectThreadRoot();

    static ObjectThreadRoot* GetInstance();
    static uint32_t GetCurrentThreadID();

    static void SetThreadAffinity(uint32_t affinity);
    static void SetDefaultThreadAffinity(uint32_t affinity);
    static void SetDefaultPrio(uint32_t priority);
    static uint32_t GetDefaultPrio();
    static void SetThreadStackSize(uint32_t stackSize);
    static uint32_t GetThreadStackSize();
    static void UseAutoStack(bool useAutoStack);
    static bool IsUsingAutoStack();

    bool Launch(uint32_t stackSize);
    bool Wait(uint32_t timeout);
    void SetPriority(uint32_t priority);
    void SetThreadName(const String& name);
    void CheckStack();

    uint32_t GetPriority() const { return m_priority; }

    bool IsLaunched() const { return m_bLaunched; }

    bool IsFinished() const { return m_bFinished; }

    void SetAffinity(uint32_t affinity) { m_affinity = affinity; }

protected:
    void ReadyToRun();

    virtual void CallObjectMethod() = 0;

private:
    friend class ThreadScrambler;

    struct ThreadData : public RootObject
    {
        ThreadData();

        OSThread* m_pThread;
        char* m_pStack;
        bool m_bJoined;
    };

    static int ObjectThreadProc(int argc, void* argv);

    bool LaunchImpl();
    void Attach(uint32_t threadId);
    void Detach();
    void WrapExceptionHandling();
    void CallObjectMethodWrapper();

    String m_name;
    ThreadData* m_pThreadData;
    uint32_t m_threadId;
    uint32_t m_priority;
    uint32_t m_stackSize;
    bool m_bLaunched;
    bool m_bStarted;
    bool m_bFinished;
    uint32_t m_affinity;
    char m_nameChar8[256];
};

}
}

#endif
