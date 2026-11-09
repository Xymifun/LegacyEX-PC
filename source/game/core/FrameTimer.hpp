#ifndef GAME_CORE_FRAMETIMER_HPP
#define GAME_CORE_FRAMETIMER_HPP

class FrameTimer
{
public:
    FrameTimer();

    int wrapIndex(int index);
    void logFrameDuration(long long duration);
    int getAverageDuration(int startIndex);
    int scaleSampleTo(long sample, int scale);
    int scaleAverageDurationTo(int startIndex, int scale);
    int getLogStart();
    int getLogEnd();
    long long* getLog();

private:
    long long mLoggedTimes[240];
    int mLogStart;
    int mLogLength;
    int mLogEnd;
};

#endif
