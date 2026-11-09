#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_WORKERTHREADS_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_WORKERTHREADS_HPP

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/ObjectThread.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class WorkerThreads : public RootObject
{
public:
    enum State
    {
        STATE_RUNNING = 0,
        STATE_STOPPING = 1,
        STATE_STOPPED = 2
    };

    WorkerThreads();
    virtual ~WorkerThreads();

    bool Start(unsigned int nbWorkers);
    bool Stop();

    int GetNbWorkers() const;
    bool IsRunning() const;
    bool IsAWorker(unsigned int threadId) const;
    bool CurrentThreadIsAWorker() const;
    State GetState();

protected:
    virtual void Initialize();
    virtual void Teardown();
    virtual void Work() = 0;

private:
    typedef ObjectThread<WorkerThreads, int> Worker;

    void Run(int);

    CriticalSection m_criticalSection;
    State m_state;
    qVector<Worker*> m_workers;
};

}
}

#endif
