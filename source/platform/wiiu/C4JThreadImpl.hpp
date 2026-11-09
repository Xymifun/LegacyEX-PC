#ifndef PLATFORM_WIIU_C4JTHREADIMPL_HPP
#define PLATFORM_WIIU_C4JTHREADIMPL_HPP

#include <stdint.h>
#include <coreinit/thread.h>

#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThread.hpp"

void Sleep(unsigned int milliseconds);

class C4JThreadImpl : public C4JThread
{
public:
    C4JThreadImpl(int (*entry)(void*), void* argument, const char* name, int stackSize);
    C4JThreadImpl(const char* name);
    virtual ~C4JThreadImpl();

    static uint32_t getCurrentThread();
    static bool isMainThread();
    static const char* getCurrentThreadName();
    static void threadDeallocator(void* thread, void* stack);
    static uint32_t OSCreateThread4J(OSThread* thread, int (*entry)(int, void*), int argumentCount, void* argument,
                                     void* stack, unsigned int stackSize, int priority, unsigned short attributes);
    static int entryPoint(int argumentCount, void* argument);
    static void Sleep(int milliseconds) { ::Sleep(milliseconds); }

    virtual void Run();
    virtual bool isRunning();
    virtual bool hasStarted();
    virtual void SetProcessor(int processor);
    virtual uint32_t GetProcessor();
    virtual void SetPriority(int priority);
    virtual uint32_t WaitForCompletion(int milliseconds);
    virtual int GetExitCode();
    virtual const char* getName();
    virtual void SetDeleteOnExit(bool deleteOnExit);
    static void SetMainThreadID();

private:
    void* m_argument;
    int (*m_entry)(void*);
    int m_stackSize;
    char m_name[64];
    int m_processor;
    void* m_stack;
    bool m_running;
    bool m_started;
    uint32_t m_exitCode;
    uint64_t m_startTime;
    bool m_deleteOnExit;
    C4JEventImpl* m_completionEvent;
    uint32_t m_priority;
    OSThread* m_thread;
};

#endif
