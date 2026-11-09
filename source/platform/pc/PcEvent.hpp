#ifndef PLATFORM_PC_PCEVENT_HPP
#define PLATFORM_PC_PCEVENT_HPP

#include <pthread.h>

class PcEvent
{
public:
    PcEvent(bool manualReset);
    ~PcEvent();
    void Set();
    void Clear();
    bool Wait(int timeoutMilliseconds);
    bool IsSet();

private:
    PcEvent(const PcEvent&);
    PcEvent& operator=(const PcEvent&);

    pthread_mutex_t m_mutex;
    pthread_cond_t m_condition;
    bool m_manualReset;
    bool m_signalled;
    int m_waiters;
};

void PcAbsoluteTimeout(struct timespec* deadline, int timeoutMilliseconds);

#endif
