#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_THREADSCRAMBLER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_THREADSCRAMBLER_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/ObjectThread.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn
{
namespace nex
{

class ObjectThreadRoot;

class ThreadScrambler : public RootObject
{
public:
    enum
    {
        MAX_THREADS = 256
    };

    ThreadScrambler();
    ~ThreadScrambler();

    static void CreateInstance();
    static void DeleteInstance();

    static void Register(ObjectThreadRoot* pThread);
    static void Unregister(ObjectThreadRoot* pThread);

    void ScramblingThread(void* pParameter);

private:
    static ThreadScrambler* s_instance;

    ObjectThread<ThreadScrambler, void*> m_thread;
    unsigned int m_uiNbThreads;
    ObjectThreadRoot* m_apThreads[MAX_THREADS];
    CriticalSection m_criticalSection;
    bool m_bStop;
};

}
}

#endif
