#include "nn/nex/ThreadScrambler.hpp"

#include <cstring>

#include "nn/nex/Platform.hpp"
#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int DEFAULT_THREAD_PRIORITY = 17;
const unsigned int LOWEST_THREAD_PRIORITY = 31;
const unsigned int PRIORITY_CHOICES = 2;
const unsigned int SCRAMBLE_SLEEP_MILLISECONDS = 1;
const unsigned int INFINITE_TIMEOUT = 0xFFFFFFFF;

}

ThreadScrambler* ThreadScrambler::s_instance = 0;

ThreadScrambler::ThreadScrambler() : m_thread(String(L"Thread Scrambler")), m_uiNbThreads(0), m_bStop(false)
{
    m_thread.SetObjectMethod(&ThreadScrambler::ScramblingThread, this, 0);
    m_thread.Launch(0);
}

ThreadScrambler::~ThreadScrambler()
{
    if (m_thread.IsLaunched())
    {
        m_bStop = true;
        m_thread.Wait(INFINITE_TIMEOUT);
    }
}

void ThreadScrambler::CreateInstance()
{
    if (s_instance == 0)
        s_instance = new ThreadScrambler();
}

void ThreadScrambler::DeleteInstance()
{
    if (s_instance != 0)
    {
        delete s_instance;
        s_instance = 0;
    }
}

void ThreadScrambler::Register(ObjectThreadRoot* pThread)
{
    if (s_instance != 0)
    {
        ScopedCS lock(&s_instance->m_criticalSection);
        if (s_instance->m_uiNbThreads == MAX_THREADS)
            return;

        if (pThread->m_threadId != s_instance->m_thread.m_threadId)
        {
            s_instance->m_apThreads[s_instance->m_uiNbThreads] = pThread;
            s_instance->m_uiNbThreads++;
        }
    }
}

void ThreadScrambler::Unregister(ObjectThreadRoot* pThread)
{
    if (s_instance != 0)
    {
        ScopedCS lock(&s_instance->m_criticalSection);
        for (int i = 0; i < s_instance->m_uiNbThreads; ++i)
        {
            if (s_instance->m_apThreads[i] == pThread)
            {
                if (i != MAX_THREADS - 1)
                {
                    std::memcpy(&s_instance->m_apThreads[i], &s_instance->m_apThreads[i + 1],
                                (s_instance->m_uiNbThreads - i - 1) * sizeof(ObjectThreadRoot*));
                }
                --s_instance->m_uiNbThreads;
                return;
            }
        }
    }
}

void ThreadScrambler::ScramblingThread(void* pParameter)
{
    while (!m_bStop)
    {
        ScopedCS lock(&m_criticalSection);
        if (m_uiNbThreads != 0)
        {
            unsigned int priority = DEFAULT_THREAD_PRIORITY;
            if (Platform::GetRandomNumber(PRIORITY_CHOICES) != 0)
                priority = LOWEST_THREAD_PRIORITY;

            unsigned int uiIndex = Platform::GetRandomNumber(m_uiNbThreads);
            m_apThreads[uiIndex]->SetPriority(priority);
            Platform::Sleep(SCRAMBLE_SLEEP_MILLISECONDS);
        }
    }
}

}
}
