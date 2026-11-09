#include "nn/nex/qReservedQueue.hpp"

namespace nn
{
namespace nex
{

template <>
void qReservedQueue<unsigned char>::push(const unsigned char* pElements, unsigned int uiCount)
{
    unsigned int uiRoom = m_uiCapacity - m_uiBack;
    if (uiRoom > uiCount)
    {
        memcpy(&m_elements[0] + m_uiBack, pElements, uiCount);
        m_iSize += uiCount;
        m_uiBack += uiCount;
        return;
    }
    memcpy(&m_elements[0] + m_uiBack, pElements, uiRoom);
    m_iSize += uiRoom;
    unsigned int uiBack = m_uiBack + uiRoom;
    uiCount -= uiRoom;
    m_uiBack = uiBack % m_uiCapacity;
    if (uiCount != 0)
    {
        memcpy(&m_elements[0], pElements + uiRoom, uiCount);
        m_uiBack = uiCount;
        m_iSize += uiCount;
    }
}

template <>
void qReservedQueue<unsigned char>::front(unsigned char* pElements, unsigned int uiCount)
{
    unsigned int uiRoom = m_uiCapacity - m_uiFront;
    const unsigned char* pSource = &m_elements[0] + m_uiFront;
    if (uiRoom <= uiCount)
    {
        memcpy(pElements, pSource, uiRoom);
        uiCount -= uiRoom;
        if (uiCount == 0)
            return;
        pSource = &m_elements[0];
        pElements += uiRoom;
    }
    memcpy(pElements, pSource, uiCount);
}

}
}
