#include "nn/nex/PacketBufferHeap.hpp"

#include <cstring>

namespace nn
{
namespace nex
{

PacketBufferUnitHeap::PacketBufferUnitHeap() : m_pMemory(0), m_uiTotalSize(0), m_uiUnitSize(0), m_pFreeList(0)
{
}

void PacketBufferUnitHeap::ResetDebugInfo()
{
    std::memset(&m_debugInfo, 0, sizeof(m_debugInfo));

    m_debugInfo.m_uiBlockSize = (m_uiUnitSize + 11) & ~3u;
    m_debugInfo.m_uiUnitSize = m_uiUnitSize;
    m_debugInfo.m_uiUnitNum = m_uiTotalSize / m_debugInfo.m_uiBlockSize;
}

}
}
