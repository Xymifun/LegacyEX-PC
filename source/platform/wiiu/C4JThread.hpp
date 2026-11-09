#ifndef PLATFORM_WIIU_C4JTHREAD_HPP
#define PLATFORM_WIIU_C4JTHREAD_HPP

#include <stdint.h>

class C4JThread
{
public:
    virtual ~C4JThread() {}
    virtual void Run() = 0;
    virtual bool isRunning() = 0;
    virtual bool hasStarted() = 0;
    virtual void SetProcessor(int processor) = 0;
    virtual uint32_t GetProcessor() = 0;
    virtual void SetPriority(int priority) = 0;
    virtual uint32_t WaitForCompletion(int milliseconds) = 0;
    virtual int GetExitCode() = 0;
    virtual const char* getName() = 0;
    virtual void SetDeleteOnExit(bool deleteOnExit) = 0;

    static void Sleep(int milliseconds);
    static uint32_t getCurrentThread();
    static bool isMainThread();
    static const char* getCurrentThreadName();
};

#endif
