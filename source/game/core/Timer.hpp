#ifndef GAME_CORE_TIMER_HPP
#define GAME_CORE_TIMER_HPP

class Timer
{
public:
    Timer(float ticksPerSecond);

    void advanceTime();
    void advanceTimeQuickly();
    void skipTime();

    float ticksPerSecond;
    double lastHRTime;
    int elapsedTicks;
    float renderPartialTicks;
    float timerSpeed;
    float elapsedPartialTicks;

private:
    long long lastSyncSysClock;
    long long lastSyncHRClock;
    long long timeSyncCounter;
    double timeSyncAdjustment;
};

#endif
