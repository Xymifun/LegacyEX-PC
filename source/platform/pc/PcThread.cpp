#include "platform/wiiu/C4JThreadImpl.hpp"

#include "platform/pc/PcCrashHandler.hpp"
#include "platform/pc/PcEvent.hpp"
#include "platform/pc/PcOs.hpp"
#include "game/core/CriticalSection.hpp"
#include "game/core/System.hpp"
#include "platform/wiiu/C4JEventArrayImpl.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThread.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <vector>

static OSFastMutex threadSection;

static std::vector<C4JThreadImpl*>& Threads()
{
    static std::vector<C4JThreadImpl*>* s_threads = 0;
    if (s_threads == 0)
        s_threads = new std::vector<C4JThreadImpl*>;
    return *s_threads;
}

static C4JThreadImpl& MainThread()
{
    static C4JThreadImpl* s_mainThread = 0;
    if (s_mainThread == 0)
        s_mainThread = new C4JThreadImpl("Main thread");
    return *s_mainThread;
}

void Sleep(unsigned int ms)
{
    PcOsSleepMilliseconds(ms);
}

void SetThreadName(unsigned int, const char*)
{
}

long long InterlockedCompareExchangeRelease64(volatile long long* dst, long long exchange, long long comparand)
{
    return __sync_val_compare_and_swap(dst, comparand, exchange);
}

long InterlockedCompareExchangeRelease(volatile long* dst, long exchange, long comparand)
{
    return __sync_val_compare_and_swap(dst, comparand, exchange);
}

void* InterlockedCompareExchangePointerRelease(void* volatile* dst, void* exchange, void* comparand)
{
    return __sync_val_compare_and_swap(dst, comparand, exchange);
}

void C4JThread::Sleep(int ms)
{
    C4JThreadImpl::Sleep(ms);
}

uint32_t C4JThread::getCurrentThread()
{
    return C4JThreadImpl::getCurrentThread();
}

bool C4JThread::isMainThread()
{
    return C4JThreadImpl::isMainThread();
}

const char* C4JThread::getCurrentThreadName()
{
    return C4JThreadImpl::getCurrentThreadName();
}

uint32_t C4JThreadImpl::getCurrentThread()
{
    pthread_t self = pthread_self();
    uint32_t found = 0;
    EnterCriticalSection(&threadSection);
    for (size_t i = 0; i < Threads().size(); ++i)
    {
        if (Threads()[i]->m_started && pthread_equal(Threads()[i]->m_thread, self))
        {
            found = (uint32_t)(reinterpret_cast<uintptr_t>(Threads()[i]));
            break;
        }
    }
    LeaveCriticalSection(&threadSection);
    return found;
}

bool C4JThreadImpl::isMainThread()
{
    return pthread_equal(pthread_self(), MainThread().m_thread) != 0;
}

const char* C4JThreadImpl::getCurrentThreadName()
{
    pthread_t self = pthread_self();
    const char* name = "(unknown)";
    EnterCriticalSection(&threadSection);
    for (size_t i = 0; i < Threads().size(); ++i)
    {
        if (Threads()[i]->m_started && pthread_equal(Threads()[i]->m_thread, self))
        {
            name = Threads()[i]->getName();
            break;
        }
    }
    LeaveCriticalSection(&threadSection);
    return name;
}

void* C4JThreadImpl::entryPoint(void* arg)
{
    C4JThreadImpl* t = static_cast<C4JThreadImpl*>(arg);
    EnterCriticalSection(&threadSection);
    LeaveCriticalSection(&threadSection);
    PcOsSetCurrentThreadName(t->m_name + 5);
    PcCrashHandlerPrepareThread();
    t->m_exitCode = t->m_entry(t->m_argument);
    if (t->m_deleteOnExit)
    {
        delete t;
    }
    else
    {
        t->m_running = false;
        t->m_completionEvent->Set();
    }
    return 0;
}

C4JThreadImpl::C4JThreadImpl(int (*e)(void*), void* arg, const char* name, int stackSize)
{
    m_entry = e;
    m_argument = arg;
    m_processor = -1;
    m_stackSize = stackSize;
    if (m_stackSize == 0)
        m_stackSize = 0x20000;
    if (m_stackSize < 0x4000)
        m_stackSize = 0x4000;
    snprintf(m_name, sizeof(m_name), "(4J) %s", name);
    m_running = false;
    m_started = false;
    m_exitCode = STILL_ACTIVE;
    m_completionEvent = new C4JEventImpl(C4JEvent::Mode1);
    m_startTime = 0;
    m_priority = 16;
    m_deleteOnExit = false;
    memset(&m_thread, 0, sizeof(m_thread));
    EnterCriticalSection(&threadSection);
    Threads().push_back(this);
    LeaveCriticalSection(&threadSection);
}

C4JThreadImpl::C4JThreadImpl(const char* name)
{
    m_argument = 0;
    m_entry = 0;
    m_stackSize = 0;
    snprintf(m_name, sizeof(m_name), "(4J) %s", name);
    m_running = true;
    m_started = true;
    m_exitCode = STILL_ACTIVE;
    m_startTime = 0;
    m_deleteOnExit = false;
    m_completionEvent = 0;
    m_thread = pthread_self();
    m_processor = 1;
    m_priority = 16;
    EnterCriticalSection(&threadSection);
    Threads().push_back(this);
    LeaveCriticalSection(&threadSection);
}

C4JThreadImpl::~C4JThreadImpl()
{
    EnterCriticalSection(&threadSection);
    for (std::vector<C4JThreadImpl*>::iterator it = Threads().begin(); it != Threads().end(); ++it)
    {
        if (*it == this)
        {
            Threads().erase(it);
            break;
        }
    }
    LeaveCriticalSection(&threadSection);
    delete m_completionEvent;
    m_completionEvent = 0;
}

void C4JThreadImpl::Run()
{
    m_startTime = System::processTimeInMilliSecs();
    m_running = true;
    pthread_attr_t attrs;
    pthread_attr_init(&attrs);
    size_t stackSize = (size_t)m_stackSize * 4;
    if (stackSize < 1024 * 1024)
        stackSize = 1024 * 1024;
    pthread_attr_setstacksize(&attrs, stackSize);
    pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    EnterCriticalSection(&threadSection);
    m_started = true;
    pthread_create(&m_thread, &attrs, entryPoint, this);
    LeaveCriticalSection(&threadSection);
    pthread_attr_destroy(&attrs);
}

bool C4JThreadImpl::isRunning()
{
    return m_running;
}

bool C4JThreadImpl::hasStarted()
{
    return m_started;
}

void C4JThreadImpl::SetProcessor(int processor)
{
    m_processor = processor;
}

uint32_t C4JThreadImpl::GetProcessor()
{
    return m_processor;
}

void C4JThreadImpl::SetPriority(int priority)
{
    m_priority = (uint32_t)(16 - priority);
}

uint32_t C4JThreadImpl::WaitForCompletion(int ms)
{
    if (m_completionEvent != 0)
        return m_completionEvent->WaitForSignal(ms);
    return 0;
}

int C4JThreadImpl::GetExitCode()
{
    return m_exitCode;
}

const char* C4JThreadImpl::getName()
{
    return m_name;
}

void C4JThreadImpl::SetDeleteOnExit(bool deleteOnExit)
{
    m_deleteOnExit = deleteOnExit;
}

void C4JThreadImpl::SetMainThreadID()
{
    MainThread().m_thread = pthread_self();
}

void SetSpecialThreadIDForFS_MilesStreamThread()
{
}

C4JEventImpl::C4JEventImpl(C4JEvent::EMode mode) : m_mode(mode), m_event(mode == C4JEvent::Mode1)
{
}

C4JEventImpl::~C4JEventImpl()
{
}

void C4JEventImpl::Set()
{
    m_event.Set();
}

void C4JEventImpl::Clear()
{
    m_event.Clear();
}

unsigned int C4JEventImpl::WaitForSignal(int timeout)
{
    return m_event.Wait(timeout) ? WaitSignalled : WaitTimeout;
}

C4JEventArrayImpl::C4JEventArrayImpl(int n, C4JEventArray::EMode mode) : m_count(n), m_mode(mode)
{
    m_signalled = new bool[(unsigned int)n];
    for (int i = 0; i < n; ++i)
        m_signalled[i] = false;
    pthread_mutex_init(&m_mutex, 0);
    pthread_condattr_t attrs;
    pthread_condattr_init(&attrs);
    if (PcOsConditionClock() != CLOCK_REALTIME)
        pthread_condattr_setclock(&attrs, PcOsConditionClock());
    pthread_cond_init(&m_condition, &attrs);
    pthread_condattr_destroy(&attrs);
}

C4JEventArrayImpl::~C4JEventArrayImpl()
{
    pthread_cond_destroy(&m_condition);
    pthread_mutex_destroy(&m_mutex);
    delete[] m_signalled;
}

void C4JEventArrayImpl::Set(int i)
{
    pthread_mutex_lock(&m_mutex);
    m_signalled[i] = true;
    pthread_cond_broadcast(&m_condition);
    pthread_mutex_unlock(&m_mutex);
}

void C4JEventArrayImpl::Clear(int i)
{
    pthread_mutex_lock(&m_mutex);
    m_signalled[i] = false;
    pthread_mutex_unlock(&m_mutex);
}

void C4JEventArrayImpl::SetAll()
{
    pthread_mutex_lock(&m_mutex);
    for (int i = 0; i < m_count; ++i)
        m_signalled[i] = true;
    pthread_cond_broadcast(&m_condition);
    pthread_mutex_unlock(&m_mutex);
}

void C4JEventArrayImpl::ClearAll()
{
    pthread_mutex_lock(&m_mutex);
    for (int i = 0; i < m_count; ++i)
        m_signalled[i] = false;
    pthread_mutex_unlock(&m_mutex);
}

bool C4JEventArrayImpl::WaitUntil(int timeout, int index, bool all)
{
    struct timespec deadline;
    if (timeout >= 0)
        PcAbsoluteTimeout(&deadline, timeout);
    pthread_mutex_lock(&m_mutex);
    bool done = false;
    for (;;)
    {
        if (index >= 0)
        {
            done = m_signalled[index];
        }
        else
        {
            done = all;
            for (int i = 0; i < m_count; ++i)
            {
                if (all && !m_signalled[i])
                    done = false;
                if (!all && m_signalled[i])
                    done = true;
            }
        }
        if (done)
            break;
        if (timeout < 0)
            pthread_cond_wait(&m_condition, &m_mutex);
        else if (pthread_cond_timedwait(&m_condition, &m_mutex, &deadline) == ETIMEDOUT)
            break;
    }
    if (done && m_mode != C4JEventArray::Mode1)
    {
        for (int i = 0; i < m_count; ++i)
        {
            if (index < 0 || i == index)
                m_signalled[i] = false;
        }
    }
    pthread_mutex_unlock(&m_mutex);
    return done;
}

unsigned int C4JEventArrayImpl::WaitForAll(int timeout)
{
    return WaitUntil(timeout, -1, true) ? 0 : 0x102;
}

unsigned int C4JEventArrayImpl::WaitForAny(int timeout)
{
    return WaitUntil(timeout, -1, false) ? 0 : 0x102;
}

unsigned int C4JEventArrayImpl::WaitForSingle(int i, int timeout)
{
    return WaitUntil(timeout, i, false) ? 0 : 0x102;
}

void C4JEventArrayImpl::Cancel()
{
}
