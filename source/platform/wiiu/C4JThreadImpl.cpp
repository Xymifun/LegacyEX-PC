#include "platform/wiiu/C4JThreadImpl.hpp"
#include "game/core/System.hpp"
#include "game/core/CriticalSection.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"
#include <coreinit/memdefaultheap.h>
#include <coreinit/fastmutex.h>
#include <coreinit/atomic64.h>
#include <coreinit/atomic.h>
#include <coreinit/mutex.h>
#include <coreinit/thread.h>
#include <coreinit/systeminfo.h>
#include <stdio.h>
#include <vector>

#ifdef __ghs__
#include <ppc_ghs.h>
#endif

static std::vector<C4JThreadImpl*> s_threads;
static C4JThreadImpl s_mainThread("Main thread");
static OSFastMutex s_createSection;
static OSFastMutex s_threadSection;
static bool s_createSectionInitialized = false;
#if defined(__ghs__) || defined(RECON_NATIVE)
__thread int g_commandBlockId = 0;
#else
static const OSThreadSpecificID COMMAND_BLOCK_ID_SLOT = OS_THREAD_SPECIFIC_12;

class ThreadCommandBlockId
{
public:
    ThreadCommandBlockId& operator=(int commandBlockId)
    {
        OSSetThreadSpecific(COMMAND_BLOCK_ID_SLOT, reinterpret_cast<void*>(static_cast<intptr_t>(commandBlockId)));
        return *this;
    }
};

static ThreadCommandBlockId g_commandBlockId;
#endif
static const int SPECIAL_THREAD_ID_FOR_FS_MILES_STREAM_THREAD = 0x40000000;
static const unsigned char s_processorAffinity[3] = {OS_THREAD_ATTRIB_AFFINITY_CPU0, OS_THREAD_ATTRIB_AFFINITY_CPU1,
                                                     OS_THREAD_ATTRIB_AFFINITY_CPU2};
static const unsigned char s_priorities[] = {18, 17, 16, 15, 14};

void SetThreadName(unsigned int, const char*)
{
}

long long InterlockedCompareExchangeRelease64(volatile long long* destination, long long exchange, long long comparand)
{
    uint64_t previous;
    OSCompareAndSwapAtomicEx64(reinterpret_cast<volatile uint64_t*>(destination), static_cast<uint64_t>(comparand),
                               static_cast<uint64_t>(exchange), &previous);
    return static_cast<long long>(previous);
}

long InterlockedCompareExchangeRelease(volatile long* destination, long exchange, long comparand)
{
#ifdef __ghs__
    unsigned int* word = reinterpret_cast<unsigned int*>(const_cast<long*>(destination));
    unsigned int previous;
    do
    {
        previous = __LWARX(word, 0);
        if (previous != static_cast<unsigned int>(comparand))
            break;
        __DCBST(word, 0);
    } while (!__STWCX(word, 0, static_cast<unsigned int>(exchange)));
    return static_cast<long>(previous);
#elif defined(__powerpc__) || defined(__PPC__)
    unsigned int previous;
    __asm__ __volatile__(
        "1:\n\tlwarx %0, 0, %1\n\tcmplw %0, %2\n\tbne- 2f\n\tdcbst 0, %1\n\tstwcx. %3, 0, %1\n\tbne- 1b\n2:"
        : "=&r"(previous)
        : "r"(destination), "r"(comparand), "r"(exchange)
        : "cr0", "memory");
    return static_cast<long>(previous);
#else
    uint32_t previous = 0;
    OSCompareAndSwapAtomicEx(reinterpret_cast<volatile uint32_t*>(destination), static_cast<uint32_t>(comparand),
                             static_cast<uint32_t>(exchange), &previous);
    return static_cast<long>(previous);
#endif
}

typedef std::vector<C4JThreadImpl*> C4JThreadVector;
typedef C4JThreadVector::iterator C4JThreadVectorIterator;

void Sleep(unsigned int milliseconds)
{
    OSSleepTicks(OSMillisecondsToTicks((OSTime)milliseconds));
}

uint32_t C4JThreadImpl::getCurrentThread()
{
    OSThread* currentThread = OSGetCurrentThread();
    EnterCriticalSection(&s_threadSection);
    for (size_t i = 0; i < s_threads.size(); ++i)
    {
        if (s_threads[i]->m_thread == currentThread)
        {
            LeaveCriticalSection(&s_threadSection);
            return (uint32_t)s_threads[i];
        }
    }
    LeaveCriticalSection(&s_threadSection);
    return 0;
}

bool C4JThreadImpl::isMainThread()
{
    return getCurrentThread() == (uint32_t)&s_mainThread;
}

const char* C4JThreadImpl::getCurrentThreadName()
{
    return ((C4JThreadImpl*)getCurrentThread())->getName();
}

void C4JThreadImpl::threadDeallocator(void*, void* stack)
{
    MEMFreeToDefaultHeap(stack);
}

uint32_t C4JThreadImpl::OSCreateThread4J(OSThread* thread, int (*entry)(int, void*), int argumentCount, void* argument,
                                         void* stack, unsigned int stackSize, int priority, unsigned short attributes)
{
    if (!s_createSectionInitialized)
    {
        InitializeCriticalSection(&s_createSection);
        s_createSectionInitialized = true;
    }
    EnterCriticalSection(&s_createSection);
    uint32_t result = OSCreateThread(thread, reinterpret_cast<OSThreadEntryPointFn>(entry), argumentCount,
                                     static_cast<char*>(argument), stack, stackSize, priority, attributes);
    OSSetThreadDeallocator(thread, reinterpret_cast<OSThreadDeallocatorFn>(threadDeallocator));
    LeaveCriticalSection(&s_createSection);
    return result;
}

int C4JThreadImpl::entryPoint(int, void* argument)
{
    C4JThreadImpl* thread = (C4JThreadImpl*)argument;
    OSSetThreadName(thread->m_thread, thread->m_name);
    thread->m_exitCode = thread->m_entry(thread->m_argument);
    int result = thread->m_exitCode;
    if (thread->m_deleteOnExit)
    {
        delete thread;
    }
    else
    {
        C4JEventImpl* completionEvent = thread->m_completionEvent;
        thread->m_completionEvent = NULL;
        thread->m_running = false;
        completionEvent->Set();
        delete completionEvent;
    }
    return result;
}

C4JThreadImpl::C4JThreadImpl(int (*entry)(void*), void* argument, const char* name, int stackSize)
{
    m_entry = entry;
    m_argument = argument;
    m_processor = -1;
    m_stackSize = stackSize;
    if (m_stackSize == 0)
        m_stackSize = 0x20000;
    if (m_stackSize < 0x4000)
        m_stackSize = 0x4000;
    sprintf(m_name, "(4J) %s", name);
    m_running = false;
    m_started = false;
    m_exitCode = STILL_ACTIVE;
    m_completionEvent = new C4JEventImpl(C4JEvent::Mode1);
    m_startTime = 0;
    m_priority = 16;
    m_stack = MEMAllocFromDefaultHeap(m_stackSize);
    void* stackTop = (char*)m_stack + m_stackSize;
    m_thread = (OSThread*)MEMAllocFromDefaultHeapEx(sizeof(OSThread), 8);
    OSCreateThread4J(m_thread, entryPoint, 0, this, stackTop, m_stackSize, 16,
                     OS_THREAD_ATTRIB_AFFINITY_ANY | OS_THREAD_ATTRIB_DETACHED);
    OSSetThreadRunQuantum(m_thread, 8000);
    EnterCriticalSection(&s_threadSection);
    s_threads.push_back(this);
    LeaveCriticalSection(&s_threadSection);
    m_deleteOnExit = false;
}

C4JThreadImpl::C4JThreadImpl(const char* name)
{
    m_argument = NULL;
    m_entry = NULL;
    m_stackSize = 0;
    sprintf(m_name, "(4J) %s", name);
    m_running = true;
    m_started = true;
    m_startTime = System::processTimeInMilliSecs();
    InitializeCriticalSection(&s_threadSection);
    m_completionEvent = NULL;
    m_thread = OSGetCurrentThread();
    m_processor = 1;
    m_stack = NULL;
    EnterCriticalSection(&s_threadSection);
    s_threads.push_back(this);
    LeaveCriticalSection(&s_threadSection);
}

C4JThreadImpl::~C4JThreadImpl()
{
    if (m_completionEvent != NULL)
    {
        delete m_completionEvent;
        m_completionEvent = NULL;
    }
    EnterCriticalSection(&s_threadSection);
    std::vector<C4JThreadImpl*>::iterator it;
    for (it = s_threads.begin(); it != s_threads.end(); ++it)
    {
        if (*it == this)
        {
            s_threads.erase(it);
            break;
        }
    }
    LeaveCriticalSection(&s_threadSection);
}

void C4JThreadImpl::Run()
{
    m_startTime = System::processTimeInMilliSecs();
    m_running = true;
    m_started = true;
    OSResumeThread(m_thread);
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
    switch (processor)
    {
    case 0:
    case 1:
    case 2:
        OSSetThreadAffinity(m_thread, s_processorAffinity[processor]);
        break;
    default:
        OSSetThreadAffinity(m_thread, OS_THREAD_ATTRIB_AFFINITY_ANY);
        break;
    }
}

uint32_t C4JThreadImpl::GetProcessor()
{
    return m_processor;
}

void C4JThreadImpl::SetPriority(int priority)
{
    m_priority = 16;
    switch (priority)
    {
    case -2:
    case -1:
    case 0:
    case 1:
    case 2:
        m_priority = s_priorities[priority + 2];
        break;
    }
    OSSetThreadPriority(m_thread, m_priority);
}

uint32_t C4JThreadImpl::WaitForCompletion(int milliseconds)
{
    if (m_completionEvent != NULL)
        return m_completionEvent->WaitForSignal(milliseconds);
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
    s_mainThread.m_thread = OSGetCurrentThread();
    OSSetThreadRunQuantum(s_mainThread.m_thread, 8000);
}

void SetSpecialThreadIDForFS_MilesStreamThread()
{
    g_commandBlockId = SPECIAL_THREAD_ID_FOR_FS_MILES_STREAM_THREAD;
}

static void insertThreads(C4JThreadVector& vector, C4JThreadVectorIterator position, size_t count,
                          C4JThreadImpl* const& value)
{
    vector.insert(position, count, value);
}

static C4JThreadVectorIterator insertThread(C4JThreadVector& vector, C4JThreadVectorIterator position,
                                            C4JThreadImpl* const& value)
{
    return vector.insert(position, value);
}

static C4JThreadVectorIterator eraseThread(C4JThreadVector& vector, C4JThreadVectorIterator position)
{
    return vector.erase(position);
}
