#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_QBUFFER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_QBUFFER_HPP

#include <stdint.h>

#include "nn/nex/RefCountedObject.hpp"

namespace nn
{
namespace nex
{

class qBuffer : public RefCountedObject
{
public:
    explicit qBuffer(unsigned int uiSize = 0, unsigned int uiHeadRoom = 0);
    qBuffer(const qBuffer& rBuffer);
    virtual ~qBuffer();

    qBuffer& operator=(const qBuffer& rBuffer);

    void clear();
    void initialize(unsigned int uiSize);
    void resize(unsigned int uiSize, const unsigned char* pData, unsigned int uiDataSize);
    void push_back(const unsigned char* pData, unsigned int uiDataSize);
    void push_front(const unsigned char* pData, unsigned int uiDataSize);

    void Trace(uint64_t uiFilter, bool bDumpContents);

    unsigned int size() const { return m_pBack != m_pFront ? static_cast<unsigned int>(m_pBack - m_pFront) : 0; }

    unsigned int capacity() const { return m_pBegin != m_pEnd ? static_cast<unsigned int>(m_pEnd - m_pBegin) : 0; }

    bool empty() const { return m_pFront == m_pBack; }

    unsigned char* front() const { return m_pFront; }

    unsigned char* back() const { return m_pBack; }

    bool operator==(const qBuffer& rBuffer) const;
    bool operator<(const qBuffer& rBuffer) const;

private:
    unsigned char* m_pBegin;
    unsigned char* m_pFront;
    unsigned char* m_pBack;
    unsigned char* m_pEnd;
    unsigned int m_uiHeadRoom;
};

}
}

#endif
