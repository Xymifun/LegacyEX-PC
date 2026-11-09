#include "nn/nex/WorkerThreads.hpp"

#include "nn/nex/String.hpp"

namespace nn
{
namespace nex
{

WorkerThreads::WorkerThreads()
{
    m_state = STATE_STOPPED;
}

WorkerThreads::~WorkerThreads()
{
    Stop();
}

bool WorkerThreads::Start(unsigned int nbWorkers)
{
    ScopedCS lock(&m_criticalSection);

    if (m_state != STATE_STOPPED)
    {
        return false;
    }

    m_state = STATE_RUNNING;

    for (unsigned int index = 0; index < nbWorkers; ++index)
    {
        String name;

        name.Format(L"WorkerThread ID %d", index + 1);

        Worker* pWorker = new Worker(name);

        pWorker->SetObjectMethod(&WorkerThreads::Run, this, 0);
        pWorker->Launch(0);
        m_workers.push_back(pWorker);
    }

    return true;
}

bool WorkerThreads::Stop()
{
    {
        ScopedCS lock(&m_criticalSection);

        if (m_state != STATE_RUNNING)
        {
            return false;
        }

        m_state = STATE_STOPPING;
    }

    for (unsigned int index = 0; index < m_workers.size(); ++index)
    {
        m_workers[index]->Wait(0xffffffffu);
        delete m_workers[index];
    }

    m_workers.clear();

    ScopedCS lock(&m_criticalSection);
    m_state = STATE_STOPPED;
    return true;
}

int WorkerThreads::GetNbWorkers() const
{
    return static_cast<int>(m_workers.size());
}

void WorkerThreads::Run(int)
{
    Initialize();

    while (m_state == STATE_RUNNING)
        Work();

    Teardown();
}

void WorkerThreads::Initialize()
{
}

void WorkerThreads::Teardown()
{
}

}
}
