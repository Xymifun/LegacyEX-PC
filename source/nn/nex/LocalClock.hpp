#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_LOCALCLOCK_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_LOCALCLOCK_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/Time.hpp"

namespace nn
{
namespace nex
{

class LocalClock : public RootObject
{
public:
    LocalClock();
    virtual ~LocalClock();

    static void DeleteInstance();

    Time GetTime() const { return Time(Time::GetTime() - m_startTime); }

private:
    static LocalClock* s_pInstance;

    Time m_startTime;
};

}
}

#endif
