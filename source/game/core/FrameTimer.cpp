#include "game/core/FrameTimer.hpp"

FrameTimer::FrameTimer()
{
    mLogStart = 0;
    mLogLength = 0;
    mLogEnd = 0;
}

int FrameTimer::wrapIndex(int index)
{
    return index % 240;
}

void FrameTimer::logFrameDuration(long long duration)
{
    mLoggedTimes[mLogEnd] = duration;
    ++mLogEnd;

    if (mLogEnd == 240)
    {
        mLogEnd = 0;
    }

    if (mLogLength < 240)
    {
        mLogStart = 0;
        ++mLogLength;
    }
    else
    {
        mLogStart = wrapIndex(mLogEnd + 1);
    }
}
