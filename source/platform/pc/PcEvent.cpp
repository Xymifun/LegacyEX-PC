#include "platform/pc/PcEvent.hpp"

#include "platform/pc/PcOs.hpp"

#include <errno.h>
#include <time.h>

void PcAbsoluteTimeout(struct timespec* deadline, int timeoutMilliseconds)
{
    clock_gettime(PcOsConditionClock(), deadline);
    deadline->tv_sec += timeoutMilliseconds / 1000;
    deadline->tv_nsec += (long)(timeoutMilliseconds % 1000) * 1000000L;
    if (deadline->tv_nsec >= 1000000000L)
    {
        deadline->tv_sec += 1;
        deadline->tv_nsec -= 1000000000L;
    }
}

PcEvent::PcEvent(bool manualReset) : m_manualReset(manualReset), m_signalled(false), m_waiters(0)
{
    pthread_mutex_init(&m_mutex, 0);
    pthread_condattr_t attrs;
    pthread_condattr_init(&attrs);
    if (PcOsConditionClock() != CLOCK_REALTIME)
        pthread_condattr_setclock(&attrs, PcOsConditionClock());
    pthread_cond_init(&m_condition, &attrs);
    pthread_condattr_destroy(&attrs);
}

PcEvent::~PcEvent()
{
    for (;;)
    {
        pthread_mutex_lock(&m_mutex);
        int waiters = m_waiters;
        pthread_mutex_unlock(&m_mutex);
        if (waiters == 0)
            break;
        PcOsSleepMilliseconds(1);
    }
    pthread_cond_destroy(&m_condition);
    pthread_mutex_destroy(&m_mutex);
}

void PcEvent::Set()
{
    pthread_mutex_lock(&m_mutex);
    m_signalled = true;
    if (m_manualReset)
        pthread_cond_broadcast(&m_condition);
    else
        pthread_cond_signal(&m_condition);
    pthread_mutex_unlock(&m_mutex);
}

void PcEvent::Clear()
{
    pthread_mutex_lock(&m_mutex);
    m_signalled = false;
    pthread_mutex_unlock(&m_mutex);
}

bool PcEvent::IsSet()
{
    pthread_mutex_lock(&m_mutex);
    bool signalled = m_signalled;
    pthread_mutex_unlock(&m_mutex);
    return signalled;
}

bool PcEvent::Wait(int timeoutMilliseconds)
{
    pthread_mutex_lock(&m_mutex);
    ++m_waiters;
    if (timeoutMilliseconds < 0)
    {
        while (!m_signalled)
            pthread_cond_wait(&m_condition, &m_mutex);
    }
    else
    {
        struct timespec deadline;
        PcAbsoluteTimeout(&deadline, timeoutMilliseconds);
        while (!m_signalled)
        {
            if (pthread_cond_timedwait(&m_condition, &m_mutex, &deadline) == ETIMEDOUT)
                break;
        }
    }
    bool signalled = m_signalled;
    if (signalled && !m_manualReset)
        m_signalled = false;
    --m_waiters;
    pthread_mutex_unlock(&m_mutex);
    return signalled;
}
