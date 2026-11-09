#include "nn/nex/ObjectThreadRoot.hpp"

#include <coreinit/atomic.h>
#include <coreinit/thread.h>
#include <cstring>

#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/SpecialArray.hpp"
#include "nn/nex/SpinTest.hpp"
#include "nn/nex/StringConversion.hpp"
#include "nn/nex/SystemError.hpp"
#include "nn/nex/ThreadVariableList.hpp"

namespace nn
{
namespace nex
{
namespace
{

const uint32_t INHERIT_THREAD_AFFINITY = 0x80000000u;

uint32_t s_defaultStackSize = 0x8000;
uint32_t s_defaultPriority = 17;
uint32_t s_defaultThreadAffinity = 0;
volatile int32_t s_nbAttachedThreads = 0;

}

ObjectThreadRoot::ThreadData::ThreadData() : m_pThread(0), m_pStack(0), m_bJoined(false)
{
}

ObjectThreadRoot::ObjectThreadRoot(const String& name)
    : m_name(name),
      m_pThreadData(new ThreadData()),
      m_threadId(0),
      m_priority(s_defaultPriority),
      m_stackSize(0),
      m_bLaunched(false),
      m_bFinished(false),
      m_affinity(INHERIT_THREAD_AFFINITY)
{
}

ObjectThreadRoot::ObjectThreadRoot(const String& name, uint32_t priority)
    : m_name(name),
      m_pThreadData(new ThreadData()),
      m_threadId(0),
      m_priority(priority),
      m_stackSize(0),
      m_bLaunched(false),
      m_bFinished(false),
      m_affinity(INHERIT_THREAD_AFFINITY)
{
}

ObjectThreadRoot::~ObjectThreadRoot()
{
    if (m_bLaunched)
        ThreadVariableList::GetInstanceRef()->ClearValue(m_threadId);

    if (m_pThreadData != 0)
    {
        if (m_pThreadData->m_pThread != 0)
        {
            if (!m_pThreadData->m_bJoined)
            {
                OSCancelThread(m_pThreadData->m_pThread);
                OSJoinThread(m_pThreadData->m_pThread, 0);
            }

            SpecialDelete(m_pThreadData->m_pThread);
            m_pThreadData->m_pThread = 0;
        }

        if (m_pThreadData->m_pStack != 0)
        {
            SpecialDeleteArray(m_pThreadData->m_pStack);
            m_pThreadData->m_pStack = 0;
        }

        delete m_pThreadData;
    }

    m_pThreadData = 0;
}

uint32_t ObjectThreadRoot::GetCurrentThreadID()
{
    return reinterpret_cast<uint32_t>(OSGetCurrentThread());
}

void ObjectThreadRoot::SetThreadAffinity(uint32_t affinity)
{
    OSSetThreadAffinity(OSGetCurrentThread(), affinity & 0xffffu);
}

bool ObjectThreadRoot::Launch(uint32_t stackSize)
{
    if (stackSize == 0)
        stackSize = s_defaultStackSize;

    m_stackSize = stackSize;

    if (m_pThreadData->m_pThread != 0)
    {
        SystemError::SignalError(
            L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/ObjectThreadRoot.cpp", 364,
            0xe000000eu, 0);
        return false;
    }

    m_bStarted = false;
    m_bLaunched = true;

    LaunchImpl();

    while (!m_bStarted)
        Platform::Sleep(1);

    return true;
}

bool ObjectThreadRoot::LaunchImpl()
{
    if (m_pThreadData->m_pStack == 0)
        m_pThreadData->m_pStack = SpecialNewArray<char>(m_stackSize, L"", 0);

    if (m_pThreadData->m_pThread == 0)
    {
        OSThread* pThread = SpecialNew<OSThread>(L"", 0);

        m_threadId = reinterpret_cast<uint32_t>(pThread);
        m_pThreadData->m_pThread = pThread;
    }

    m_pThreadData->m_bJoined = false;

    uint32_t affinity = 0;

    if (m_affinity != INHERIT_THREAD_AFFINITY)
        affinity = m_affinity & 0xffffu;

    if (!OSCreateThread(m_pThreadData->m_pThread, reinterpret_cast<OSThreadEntryPointFn>(&ObjectThreadProc), 0,
                        reinterpret_cast<char*>(this), m_pThreadData->m_pStack + m_stackSize, m_stackSize,
                        static_cast<int32_t>(m_priority), static_cast<OSThreadAttributes>(affinity)))
    {
        SpecialDelete(m_pThreadData->m_pThread);
        m_pThreadData->m_pThread = 0;
        return false;
    }

    m_name = String(L"NEX ") + m_name;

    std::memset(m_nameChar8, 0, sizeof(m_nameChar8));
    StringConversion::T2Char8(m_name, m_nameChar8, sizeof(m_nameChar8));

    OSSetThreadName(m_pThreadData->m_pThread, m_nameChar8);
    OSResumeThread(m_pThreadData->m_pThread);
    return true;
}

int ObjectThreadRoot::ObjectThreadProc(int argc, void* argv)
{
    ObjectThreadRoot* pObjectThread = static_cast<ObjectThreadRoot*>(argv);

    GlobalVariables::GetInstancePtr()->GetCurrentThread().SetValue(pObjectThread);
    pObjectThread->CallObjectMethodWrapper();
    return 0;
}

bool ObjectThreadRoot::Wait(uint32_t timeout)
{
    if (m_pThreadData->m_pThread == 0)
        return true;

    if (timeout != 0xffffffffu)
    {
        SpinTest spinTest(100, timeout);

        spinTest.LeaveOnTimeout();

        while (
            !m_bFinished
            && spinTest.SpinOnce(
                   L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/ObjectThreadRoot.cpp",
                   803, L"!m_bFinished")
                   != 0)
        {
        }

        if (!m_bFinished)
        {
            SystemError::SignalError(
                L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/ObjectThreadRoot.cpp",
                806, 0xe000000cu, 0);
            return false;
        }

        int exitValue = 0;

        OSJoinThread(m_pThreadData->m_pThread, &exitValue);
        m_pThreadData->m_bJoined = true;
        SpecialDelete(m_pThreadData->m_pThread);
        m_pThreadData->m_pThread = 0;
        return true;
    }

    while (!m_bFinished)
    {
        SpinTest spinTest(100, 100);

        spinTest.LeaveOnTimeout();

        while (
            !m_bFinished
            && spinTest.SpinOnce(
                   L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Platform/Core/ObjectThreadRoot.cpp",
                   847, L"!m_bFinished")
                   != 0)
        {
        }
    }

    int exitValue = 0;

    OSJoinThread(m_pThreadData->m_pThread, &exitValue);
    m_pThreadData->m_bJoined = true;
    SpecialDelete(m_pThreadData->m_pThread);
    m_pThreadData->m_pThread = 0;
    return true;
}

void ObjectThreadRoot::SetPriority(uint32_t priority)
{
    OSThread* pThread = m_pThreadData->m_pThread;

    m_priority = priority;
    OSSetThreadPriority(pThread, static_cast<int32_t>(priority));
}

void ObjectThreadRoot::ReadyToRun()
{
    m_bStarted = true;
}

void ObjectThreadRoot::Attach(uint32_t threadId)
{
    m_threadId = threadId;
    OSAddAtomic(&s_nbAttachedThreads, 1);
}

void ObjectThreadRoot::Detach()
{
    OSAddAtomic(&s_nbAttachedThreads, -1);
}

void ObjectThreadRoot::WrapExceptionHandling()
{
    CallObjectMethod();
}

void ObjectThreadRoot::CallObjectMethodWrapper()
{
    if (s_defaultThreadAffinity != 0)
        SetThreadAffinity(s_defaultThreadAffinity);

    SetPriority(m_priority);

    m_bFinished = false;
    Attach(GetCurrentThreadID());
    WrapExceptionHandling();
    m_bFinished = true;
    Detach();

    OSExitThread(0);
}

}
}
