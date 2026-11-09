#include "nn/nex/qBuffer.hpp"

#include <cstring>

#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/StringStream.hpp"

namespace nn
{
namespace nex
{

qBuffer::qBuffer(unsigned int uiSize, unsigned int uiHeadRoom) : RefCountedObject()
{
    m_uiHeadRoom = uiHeadRoom;
    m_pBegin = 0;
    m_pFront = 0;
    m_pBack = 0;
    m_pEnd = 0;
    initialize(uiSize);
}

qBuffer::qBuffer(const qBuffer& rBuffer) : RefCountedObject()
{
    m_pBegin = 0;
    m_pFront = 0;
    m_pBack = 0;
    m_pEnd = 0;
    m_uiHeadRoom = 0;
    initialize(rBuffer.capacity());
    *this = rBuffer;
}

qBuffer::~qBuffer()
{
    if (m_pBegin != 0)
    {
        MemoryManager::Free(m_pBegin);
    }
}

qBuffer& qBuffer::operator=(const qBuffer& rBuffer)
{
    if (&rBuffer != this)
    {
        clear();
        push_back(rBuffer.m_pFront, rBuffer.size());
    }
    return *this;
}

void qBuffer::clear()
{
    unsigned char* pStart = m_pBegin;
    if (m_uiHeadRoom != 0)
    {
        pStart += m_uiHeadRoom;
    }
    m_pBack = pStart;
    m_pFront = pStart;
}

void qBuffer::initialize(unsigned int uiSize)
{
    if (uiSize == 0)
    {
        m_pBegin = 0;
        m_pEnd = 0;
        clear();
    }
    else
    {
        if (uiSize < 10)
        {
            uiSize = 10;
        }
        m_pBegin = static_cast<unsigned char*>(MemoryManager::Allocate(uiSize));
        m_pEnd = m_pBegin + uiSize;
        clear();
    }
}

void qBuffer::resize(unsigned int uiSize, const unsigned char* pData, unsigned int uiDataSize)
{
    unsigned char* pOldBuffer = m_pBegin;
    int nGranularity = static_cast<int>(capacity()) / 2;
    if (nGranularity != 0)
    {
        uiSize = (uiSize / static_cast<unsigned int>(nGranularity) + 1) * static_cast<unsigned int>(nGranularity);
    }
    if (uiSize < 10)
    {
        uiSize = 10;
    }

    unsigned int uiOldSize = size();
    initialize(uiSize);
    if (pData != 0)
    {
        push_back(pData, uiDataSize);
    }
    push_back(pOldBuffer, uiOldSize);
    MemoryManager::Free(pOldBuffer);
}

void qBuffer::push_back(const unsigned char* pData, unsigned int uiDataSize)
{
    if (uiDataSize == 0)
    {
        return;
    }

    for (;;)
    {
        unsigned int uiFree = 0;
        if (m_pEnd != m_pBack)
        {
            uiFree = static_cast<unsigned int>(m_pEnd - m_pBack);
        }
        if (uiDataSize <= uiFree)
        {
            break;
        }
        unsigned int uiUsed = 0;
        if (m_pBack != m_pBegin)
        {
            uiUsed = static_cast<unsigned int>(m_pBack - m_pBegin);
        }
        resize(uiUsed + uiDataSize, 0, 0);
    }

    std::memcpy(m_pBack, pData, uiDataSize);
    m_pBack += uiDataSize;
}

void qBuffer::Trace(uint64_t uiFilter, bool bDumpContents)
{
    if (bDumpContents)
    {
        if (size() == 0)
        {
            return;
        }

        StringStream stream;
        unsigned int uiIndex = 0;
        for (unsigned char* pByte = m_pFront; pByte != m_pBack; ++pByte, ++uiIndex)
        {
            if (uiIndex != 0 && (uiIndex % 32) == 0)
            {
                stream.Clear();
            }
            hex(stream).StreamNumber(*pByte) << L", ";
        }
    }
}

}
}
