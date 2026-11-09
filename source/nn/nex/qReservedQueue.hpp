#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QRESERVEDQUEUE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QRESERVEDQUEUE_HPP

#include "nn/nex/RootObject.hpp"
#include "nn/nex/qVector.hpp"
#include <string.h>

namespace nn
{
namespace nex
{

template <typename T>
class qReservedQueue : public RootObject
{
public:
    class iterator
    {
    public:
        iterator(const qReservedQueue<T>* pQueue, unsigned int uiIndex, bool bEnd)
            : m_pQueue(pQueue),
              m_uiIndex(uiIndex),
              m_bEnd(bEnd)
        {
        }

        bool operator==(const iterator& rOther) const;

        bool operator!=(const iterator& rOther) const { return !(*this == rOther); }

        const T& operator*() const;

        iterator& operator++()
        {
            m_uiIndex = (m_uiIndex + 1) % m_pQueue->m_uiCapacity;
            m_bEnd = m_pQueue->m_uiBack == m_uiIndex;
            return *this;
        }

    private:
        const qReservedQueue<T>* m_pQueue;
        unsigned int m_uiIndex;
        bool m_bEnd;
    };

    explicit qReservedQueue(unsigned int uiCapacity)
        : m_elements(uiCapacity),
          m_uiBack(0),
          m_uiFront(0),
          m_iSize(0),
          m_uiCapacity(uiCapacity)
    {
    }

    bool empty() const { return m_iSize == 0; }

    int size() const { return m_iSize; }

    unsigned int capacity() const { return m_uiCapacity; }

    T& front() { return m_elements[m_uiFront]; }

    void push(const T& rElement)
    {
        m_elements[m_uiBack] = rElement;
        ++m_uiBack;
        m_uiBack %= m_uiCapacity;
        ++m_iSize;
    }

    void push(const T* pElements, unsigned int uiCount);
    void front(T* pElements, unsigned int uiCount);

    void pop()
    {
        ++m_uiFront;
        m_uiFront %= m_uiCapacity;
        --m_iSize;
    }

    void pop(unsigned int uiCount)
    {
        m_iSize -= uiCount;
        m_uiFront = (m_uiFront + uiCount) % m_uiCapacity;
    }

    T& operator[](unsigned int uiIndex) { return m_elements.at((m_uiFront + uiIndex) % m_uiCapacity); }

    iterator begin() const { return iterator(this, m_uiFront, false); }

    iterator end() const { return iterator(this, m_uiBack, true); }

private:
    qVector<T> m_elements;
    unsigned int m_uiBack;
    unsigned int m_uiFront;
    int m_iSize;
    unsigned int m_uiCapacity;
};

template <typename T>
bool qReservedQueue<T>::iterator::operator==(const iterator& rOther) const
{
    if (m_pQueue != rOther.m_pQueue)
        return false;
    if (m_pQueue->m_iSize == 0 && m_pQueue->m_uiBack == m_uiIndex && m_uiIndex == rOther.m_uiIndex)
        return true;
    return m_uiIndex == rOther.m_uiIndex && m_bEnd == rOther.m_bEnd;
}

template <typename T>
const T& qReservedQueue<T>::iterator::operator*() const
{
    return m_pQueue->m_elements.begin()[m_uiIndex];
}

template <>
void qReservedQueue<unsigned char>::push(const unsigned char* pElements, unsigned int uiCount);

template <>
void qReservedQueue<unsigned char>::front(unsigned char* pElements, unsigned int uiCount);

}
}

#endif
