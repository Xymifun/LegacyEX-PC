#ifndef GAME_CORE_PERFORMANCETIMER_HPP
#define GAME_CORE_PERFORMANCETIMER_HPP

#include <string>

class PerformanceTimer
{
public:
    PerformanceTimer();

    void Reset();
    void PrintElapsedTime(const std::wstring& label);

private:
    unsigned long long m_startTime;
    unsigned long long m_endTime;
};

#endif
