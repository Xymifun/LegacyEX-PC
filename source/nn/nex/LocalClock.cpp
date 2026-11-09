#include "nn/nex/LocalClock.hpp"

namespace nn
{
namespace nex
{

LocalClock* LocalClock::s_pInstance = 0;

LocalClock::LocalClock()
{
    m_startTime = Time(Time::GetTime());
}

LocalClock::~LocalClock()
{
}

void LocalClock::DeleteInstance()
{
    if (s_pInstance != 0)
    {
        delete s_pInstance;
        s_pInstance = 0;
    }
}

}
}
