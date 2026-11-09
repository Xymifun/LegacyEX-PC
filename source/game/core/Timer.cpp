#include "game/core/Timer.hpp"

#include "game/core/System.hpp"

Timer::Timer(float ticksPerSecond)
{
    lastHRTime = 0.0;
    elapsedTicks = 0;
    renderPartialTicks = 0.0f;
    timerSpeed = 1.0f;
    elapsedPartialTicks = 0.0f;
    timeSyncCounter = 0;
    timeSyncAdjustment = 1.0;

    this->ticksPerSecond = ticksPerSecond;
    lastSyncSysClock = System::processTimeInMilliSecs();
    lastSyncHRClock = System::processTimeInMilliSecs();
}

void Timer::advanceTime()
{
    long long sysClock = System::processTimeInMilliSecs();
    long long sysClockDelta = sysClock - lastSyncSysClock;
    long long hrClock = System::processTimeInMilliSecs();
    double hrTime = (double)hrClock / 1000.0;

    if (sysClockDelta > 1000)
    {
        lastHRTime = hrTime - 1000.0;
    }
    else if (sysClockDelta < 0)
    {
        lastHRTime = hrTime;
    }
    else
    {
        timeSyncCounter += sysClockDelta;

        if (timeSyncCounter > 1000)
        {
            long long hrClockDelta = hrClock - lastSyncHRClock;
            double measuredRate = (double)timeSyncCounter / (double)hrClockDelta;
            timeSyncAdjustment += (measuredRate - timeSyncAdjustment) * 0.20000000298023224;
            lastSyncHRClock = hrClock;
            timeSyncCounter = 0;
        }

        if (timeSyncCounter < 0)
        {
            lastSyncHRClock = hrClock;
        }
    }

    lastSyncSysClock = sysClock;

    double timePassed = (hrTime - lastHRTime) * timeSyncAdjustment;
    lastHRTime = hrTime;

    if (timePassed < 0.0)
    {
        timePassed = 0.0;
    }

    if (timePassed > 1.0)
    {
        timePassed = 1.0;
    }

    elapsedPartialTicks = (float)((double)elapsedPartialTicks
                                  + timePassed * (double)timerSpeed * (double)ticksPerSecond);
    elapsedTicks = (int)elapsedPartialTicks;
    elapsedPartialTicks -= (float)elapsedTicks;

    if (elapsedTicks > 10)
    {
        elapsedTicks = 10;
    }

    renderPartialTicks = elapsedPartialTicks;
}

void Timer::advanceTimeQuickly()
{
    elapsedTicks = 1;
    elapsedPartialTicks = 0.0f;
    renderPartialTicks = 0.0f;
}

void Timer::skipTime()
{
    long long clock = System::processTimeInMilliSecs();
    lastSyncSysClock = clock;
    lastSyncHRClock = clock;
    lastHRTime = (double)clock / 1000.0;
    timeSyncCounter = 0;
    timeSyncAdjustment = 1.0;
    elapsedTicks = 0;
    elapsedPartialTicks = 0.0f;
    renderPartialTicks = 0.0f;
}
