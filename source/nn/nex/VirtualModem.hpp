#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALMODEM_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALMODEM_HPP

#include "nn/nex/Buffer.hpp"
#include "nn/nex/EmulationDevice.hpp"
#include "nn/nex/TimedQueue.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class VirtualModem : public TimedQueue<T>
{
public:
    explicit VirtualModem(EmulationDevice* pDevice) : m_pDevice(pDevice), m_lastTime(Time::GetTime()) {}

    virtual ~VirtualModem() {}

    virtual void Queue(const T& rItem, Time time)
    {
        Time queueTime = m_lastTime;
        if (static_cast<unsigned long long>(m_lastTime) < static_cast<unsigned long long>(time))
        {
            queueTime = time;
            m_lastTime = time;
        }

        TimedQueue<T>::Queue(rItem, queueTime);
        unsigned int uiBandwidth = m_pDevice->GetBandwidth();
        if (uiBandwidth != 0xffffffff)
        {
            unsigned int uiDelay = (rItem.m_pBuffer->GetLength() * 8000) / uiBandwidth;
            m_lastTime = Time(static_cast<unsigned long long>(m_lastTime) + uiDelay);
        }
    }

private:
    EmulationDevice* m_pDevice;
    Time m_lastTime;
};

}
}

#endif
