#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALINTERNET_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_VIRTUALINTERNET_HPP

#include "nn/nex/EmulationDevice.hpp"
#include "nn/nex/Platform.hpp"
#include "nn/nex/TimedQueue.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class VirtualInternet : public TimedQueue<T>
{
public:
    explicit VirtualInternet(EmulationDevice* pDevice) : m_pDevice(pDevice) {}

    virtual ~VirtualInternet() {}

    virtual void Queue(const T& rItem, Time time)
    {
        float fDropProbability = m_pDevice->GetPacketDropProbability();
        if (Platform::GetRealRandomNumber(1.0f) >= fDropProbability)
        {
            unsigned int uiJitter = m_pDevice->GetJitter();
            Time delivery = time + static_cast<int>(m_pDevice->GetLatency());
            if (uiJitter == 0)
            {
                TimedQueue<T>::Queue(rItem, delivery);
            }
            else
            {
                int iRandom = static_cast<int>(Platform::GetRandomNumber(uiJitter));
                TimedQueue<T>::Queue(rItem, delivery + iRandom);
            }
        }
    }

private:
    EmulationDevice* m_pDevice;
};

}
}

#endif
