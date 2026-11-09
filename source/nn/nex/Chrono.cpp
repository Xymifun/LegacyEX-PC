#include "nn/nex/Chrono.hpp"

#include "nn/nex/SystemError.hpp"

namespace nn
{
namespace nex
{

namespace
{

const unsigned int CHRONO_ERROR_INVALID_STATE = 0xE000000E;

}

Chrono::Chrono()
{
    Reset();
}

Chrono::~Chrono()
{
}

void Chrono::Reset()
{
    m_state = Stopped;
    m_accumulatedTime = 0;
}

bool Chrono::Start()
{
    if (m_state != Stopped)
    {
        SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Chrono.cpp", 47,
                                 CHRONO_ERROR_INVALID_STATE, 0);
        return false;
    }

    Reset();
    m_startTime = m_clock.GetTime();
    m_state = Running;
    return true;
}

void Chrono::UpdateAccumulatedTime()
{
    m_accumulatedTime += static_cast<int>(m_clock.GetTime() - m_startTime);
}

int Chrono::Elapsed() const
{
    if (m_state == Running)
    {
        Time now = m_clock.GetTime();
        return m_accumulatedTime + static_cast<int>(now - m_startTime);
    }

    return m_accumulatedTime;
}

int Chrono::Check()
{
    return Elapsed();
}

int Chrono::Stop()
{
    if (m_state != Running)
    {
        SystemError::SignalError(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/Chrono.cpp", 97,
                                 CHRONO_ERROR_INVALID_STATE, 0);
        return -1;
    }

    UpdateAccumulatedTime();
    m_state = Stopped;
    return Elapsed();
}

}
}
