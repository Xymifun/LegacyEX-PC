#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_CHRONO_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_CHRONO_HPP

#include "nn/nex/LocalClock.hpp"
#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class Chrono : public RootObject
{
public:
    enum State
    {
        Running = 0,
        Stopped = 2
    };

    Chrono();
    virtual ~Chrono();

    void Reset();
    bool Start();
    int Stop();
    bool Pause();
    bool Resume();
    int Check();
    int Elapsed() const;
    State GetState() const;

private:
    void UpdateAccumulatedTime();

    LocalClock m_clock;
    Time m_startTime;
    int m_accumulatedTime;
    State m_state;
};

}
}

#endif
