#include "nn/nex/PacketBufferManager.hpp"

#include <algorithm>

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/GlobalVariables.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/PacketBufferMultiUnitHeap.hpp"
#include "nn/nex/RootTransport.hpp"

namespace nn
{
namespace nex
{

namespace
{

CriticalSection s_criticalSection;

const unsigned int PACKET_MANAGEMENT_OBJECT_SIZES[3] = {40, 36, 328};

const unsigned int PACKET_MANAGEMENT_OBJECT_KINDS = 3;

const unsigned int PACKET_MANAGEMENT_OBJECT_MARGIN = 4;

const unsigned int PACKET_BUFFER_HEADER_MARGIN = 0x50;

const unsigned int PACKET_BUFFER_UNIT_HEADER_SIZE = 8;

const unsigned int PACKET_BUFFER_UNIT_ALIGNMENT = 4;

const unsigned long long TRACE_FILTER_PACKET_BUFFER_POOL = 0x0000008000801000ULL;

}

PacketBufferManager::PacketBufferManager()
    : PseudoSingleton(ClassID),
      m_pfnAllocate(0),
      m_pfnAllocateManagementObject(0),
      m_pMemory(0),
      m_uiMemorySize(0),
      m_iInitializeCount(0),
      m_uiUnitHeaderSize(PACKET_BUFFER_UNIT_HEADER_SIZE),
      m_uiUnitAlignment(PACKET_BUFFER_UNIT_ALIGNMENT),
      m_fMemoryUsageRate(0.0f),
      m_bActive(false),
      m_pPacketBufferHeap(0),
      m_pManagementObjectHeap(0)
{
}

PacketBufferManager::~PacketBufferManager()
{
    m_iInitializeCount = 0;
    Destroy();
}

bool PacketBufferManager::IsEnablePacketBufferPool()
{
    ScopedCS lock(&s_criticalSection);
    const bool enabled = g_globalVariables->m_packetBufferPoolEnabled;
    return enabled;
}

bool PacketBufferManager::EnablePacketBufferPool(bool bEnable)
{
    ScopedCS lock(&s_criticalSection);

    if (IsActive())
        return false;

    g_globalVariables->m_packetBufferPoolEnabled = bEnable;
    return true;
}

void PacketBufferManager::GetUnitHeapParam(qVector<UnitHeapParam>& rParams)
{
    ScopedCS lock(&s_criticalSection);
    if (!g_globalVariables->m_packetBufferParams.empty())
    {
        rParams = g_globalVariables->m_packetBufferParams;
    }
    else
    {
        qVector<UnitHeapParam> aParams;
        GetSimpleUnitHeapParam(0x4b000, 0x554, aParams);
        rParams = aParams;
    }
}

bool PacketBufferManager::IsActiveNonStatic()
{
    return m_bActive;
}

bool PacketBufferManager::IsActive()
{
    ScopedCS lock(&s_criticalSection);

    PacketBufferManager* pInstance = GetInstance();
    return pInstance != 0 && pInstance->IsActiveNonStatic();
}

unsigned int PacketBufferManager::GetRequiredMemorySizeImpl(const qVector<UnitHeapParam>& rParams)
{
    unsigned int uiSize = 0;

    for (unsigned int i = 0; i < rParams.size(); ++i)
        uiSize += ((rParams[i].m_uiUnitSize + 11) & ~3u) * rParams[i].m_uiUnitNum;

    return uiSize;
}

bool PacketBufferManager::CheckUnitHeapParam(const qVector<UnitHeapParam>& rParams)
{
    if (rParams.size() == 0)
        return true;

    bool bHasMaxUnit = false;

    for (unsigned int i = 0; i < rParams.size(); ++i)
    {
        const unsigned int uiUnitSize = rParams[i].m_uiUnitSize;

        if ((uiUnitSize & 3) != 0)
            return false;

        if (uiUnitSize == 0)
            return false;

        if (uiUnitSize > MaxUnitSize)
            return false;

        if (rParams[i].m_uiUnitNum == 0)
            return false;

        if (uiUnitSize == MaxUnitSize)
            bHasMaxUnit = true;
    }

    if (!bHasMaxUnit)
        return false;

    return true;
}

void PacketBufferManager::GetPacketManagementUnitHeapParam(qVector<UnitHeapParam>& rManagementParams,
                                                           const qVector<UnitHeapParam>& rParams)
{
    unsigned int uiUnitNum = 0;

    for (unsigned int i = 0; i < rParams.size(); ++i)
        uiUnitNum += rParams[i].m_uiUnitNum;

    qVector<UnitHeapParam> aParams;

    for (unsigned int i = 0; i < PACKET_MANAGEMENT_OBJECT_KINDS; ++i)
    {
        UnitHeapParam param;

        param.m_uiUnitNum = uiUnitNum + PACKET_MANAGEMENT_OBJECT_MARGIN;
        param.m_uiUnitSize = PACKET_MANAGEMENT_OBJECT_SIZES[i];
        aParams.push_back(param);
    }

    std::sort(aParams.begin(), aParams.end());
    rManagementParams = aParams;
}

unsigned int PacketBufferManager::GetRequiredMemorySize(const qVector<UnitHeapParam>& rParams)
{
    if (!CheckUnitHeapParam(rParams))
        return 0;

    unsigned int uiSize = GetRequiredMemorySizeImpl(rParams);

    if (uiSize != 0)
    {
        qVector<UnitHeapParam> aManagementParams;

        GetPacketManagementUnitHeapParam(aManagementParams, rParams);
        uiSize += GetRequiredMemorySizeImpl(aManagementParams);
    }

    return uiSize;
}

bool PacketBufferManager::GetSimpleUnitHeapParam(unsigned int uiPoolSize, unsigned int uiUnitSize,
                                                 qVector<UnitHeapParam>& rParams)
{
    if (uiPoolSize < MinimumPoolSize || uiUnitSize == 0 || uiUnitSize > MaxUnitSize)
        return false;

    uiUnitSize += PACKET_BUFFER_HEADER_MARGIN;

    if (uiUnitSize > MaxUnitSize)
        uiUnitSize = MaxUnitSize;

    const unsigned int uiAlignedUnitSize = (uiUnitSize + 3) & ~3u;

    qVector<UnitHeapParam> aParams;

    if (uiAlignedUnitSize < MaxUnitSize)
    {
        UnitHeapParam param;

        param.m_uiUnitSize = uiAlignedUnitSize;
        param.m_uiUnitNum = 0;
        aParams.push_back(param);
    }

    UnitHeapParam maxParam;

    maxParam.m_uiUnitSize = MaxUnitSize;
    maxParam.m_uiUnitNum = 0;
    aParams.push_back(maxParam);

    unsigned int uiManagementBlockSize = 0;

    for (unsigned int i = 0; i < PACKET_MANAGEMENT_OBJECT_KINDS; ++i)
        uiManagementBlockSize += (PACKET_MANAGEMENT_OBJECT_SIZES[i] + 11) & ~3u;

    for (unsigned int i = 0; i < aParams.size(); ++i)
    {
        const unsigned int uiUnitNum = ((uiPoolSize - uiManagementBlockSize * PACKET_MANAGEMENT_OBJECT_MARGIN)
                                        / static_cast<unsigned int>(aParams.size()))
                                       / (((aParams[i].m_uiUnitSize + 11) & ~3u) + uiManagementBlockSize);

        if (uiUnitNum == 0)
            return false;

        aParams[i].m_uiUnitNum = uiUnitNum;
    }

    std::sort(aParams.begin(), aParams.end());
    rParams = aParams;
    return true;
}

void* PacketBufferManager::PacketBufferAllocate(unsigned int uiSize)
{
    return GetInstance()->m_pPacketBufferHeap->Allocate(uiSize, PacketBufferFree);
}

void PacketBufferManager::PacketBufferFree(void* pBlock)
{
    if (pBlock == 0)
        return;

    GetInstance()->m_pPacketBufferHeap->Free(pBlock);
}

void* PacketBufferManager::PacketManagementObjectAllocate(unsigned int uiSize)
{
    return GetInstance()->m_pManagementObjectHeap->Allocate(uiSize, PacketManagementObjectFree);
}

void PacketBufferManager::PacketManagementObjectFree(void* pBlock)
{
    if (pBlock == 0)
        return;

    GetInstance()->m_pManagementObjectHeap->Free(pBlock);
}

unsigned int PacketBufferManager::GetMaxUnitSize()
{
    if (m_pPacketBufferHeap == 0)
        return 0;

    return m_pPacketBufferHeap->m_uiMaxUnitSize;
}

unsigned int PacketBufferManager::GetProperUnitSize(unsigned int uiSize)
{
    PacketBufferMultiUnitHeap* pHeap = m_pPacketBufferHeap;

    if (pHeap != 0 && pHeap->m_uiMaxUnitSize >= uiSize)
    {
        for (unsigned int i = 0; i < pHeap->m_uiUnitHeapNum; ++i)
        {
            if (uiSize <= pHeap->m_pUnitHeaps[i].m_uiUnitSize)
                return pHeap->m_pUnitHeaps[i].m_uiUnitSize;
        }
    }

    return 0;
}

void* PacketBufferManager::Allocate(unsigned int uiSize)
{
    if (m_pfnAllocate != 0)
        return m_pfnAllocate(uiSize);

    return MemoryManager::Allocate(uiSize);
}

void* PacketBufferManager::AllocateManagementObject(unsigned int uiSize)
{
    if (m_pfnAllocateManagementObject != 0)
        return m_pfnAllocateManagementObject(uiSize);

    return MemoryManager::Allocate(uiSize);
}

bool PacketBufferManager::IsAllocatable(unsigned int uiSize, unsigned int uiUnitNum)
{
    PacketBufferMultiUnitHeap* pHeap = m_pPacketBufferHeap;

    if (pHeap == 0)
        return true;

    bool bAllocatable;

    if (uiSize > pHeap->m_uiMaxUnitSize)
    {
        bAllocatable = true;
    }
    else
    {
        bAllocatable = false;

        pHeap->Lock();

        unsigned int uiFreeUnitNum = 0;

        for (unsigned int i = 0; i < pHeap->m_uiUnitHeapNum; ++i)
        {
            PacketBufferUnitHeap& rUnitHeap = pHeap->m_pUnitHeaps[i];

            if (uiSize > rUnitHeap.m_uiUnitSize)
                continue;

            uiFreeUnitNum += rUnitHeap.m_uiTotalSize / ((rUnitHeap.m_uiUnitSize + 11) & ~3u)
                             - rUnitHeap.m_uiUsedUnitNum;

            if (uiUnitNum <= uiFreeUnitNum)
            {
                bAllocatable = true;
                pHeap->m_bUnitExhausted = false;
                break;
            }
        }

        if (!bAllocatable)
        {
            pHeap->m_bUnitExhausted = true;
            ++pHeap->m_pUnitHeaps[pHeap->m_uiUnitHeapNum - 1].m_debugInfo.m_uiAllocFailedNum;
        }

        pHeap->Unlock();
    }

    return bAllocatable;
}

void PacketBufferManager::GetUnitHeapDebugInfo(qVector<UnitHeapDebugInfo>& rDebugInfo)
{
    ScopedCS lock(&s_criticalSection);

    rDebugInfo.clear();

    PacketBufferManager* pInstance = GetInstance();

    if (pInstance != 0 && pInstance->m_pPacketBufferHeap != 0)
        pInstance->m_pPacketBufferHeap->GetDebugInfo(rDebugInfo);
}

void PacketBufferManager::GetAllUnitHeapDebugInfo(qVector<UnitHeapDebugInfo>& rDebugInfo)
{
    ScopedCS lock(&s_criticalSection);

    PacketBufferManager* pInstance = GetInstance();

    qVector<UnitHeapDebugInfo> aPacketBufferDebugInfo;

    if (pInstance != 0 && pInstance->m_pPacketBufferHeap != 0)
        pInstance->m_pPacketBufferHeap->GetDebugInfo(aPacketBufferDebugInfo);

    qVector<UnitHeapDebugInfo> aManagementObjectDebugInfo;

    if (pInstance != 0 && pInstance->m_pManagementObjectHeap != 0)
        pInstance->m_pManagementObjectHeap->GetDebugInfo(aManagementObjectDebugInfo);

    rDebugInfo.clear();
    rDebugInfo.insert(rDebugInfo.end(), aPacketBufferDebugInfo.begin(), aPacketBufferDebugInfo.end());
    rDebugInfo.insert(rDebugInfo.end(), aManagementObjectDebugInfo.begin(), aManagementObjectDebugInfo.end());
}

void PacketBufferManager::Dump(unsigned long long)
{
    if (m_pPacketBufferHeap != 0)
    {
        qVector<UnitHeapDebugInfo> aDebugInfo;

        m_pPacketBufferHeap->GetDebugInfo(aDebugInfo);

        for (qVector<UnitHeapDebugInfo>::iterator it = aDebugInfo.begin(); it != aDebugInfo.end(); ++it)
        {
        }
    }

    if (m_pManagementObjectHeap != 0)
    {
        qVector<UnitHeapDebugInfo> aDebugInfo;

        m_pManagementObjectHeap->GetDebugInfo(aDebugInfo);

        for (qVector<UnitHeapDebugInfo>::iterator it = aDebugInfo.begin(); it != aDebugInfo.end(); ++it)
        {
        }
    }
}

void PacketBufferManager::Initialize()
{
    ScopedCS lock(&s_criticalSection);
    if (m_iInitializeCount == 0 && IsEnablePacketBufferPool() && !IsActive())
    {
        qVector<UnitHeapParam> aParams;
        GetUnitHeapParam(aParams);
        const unsigned int uiMemorySize = GetRequiredMemorySize(aParams);
        m_pMemory = MemoryManager::Allocate(uiMemorySize);
        m_uiMemorySize = uiMemorySize;

        PacketBufferMultiUnitHeap* pPacketHeap = new PacketBufferMultiUnitHeap();
        void* pPacketMemory = m_pMemory;
        const unsigned int uiPacketMemorySize = GetRequiredMemorySizeImpl(aParams);
        pPacketHeap->Initialize(pPacketMemory, uiPacketMemorySize, aParams);
        m_pPacketBufferHeap = pPacketHeap;

        {
            qVector<UnitHeapParam> aManagementParams;
            GetPacketManagementUnitHeapParam(aManagementParams, aParams);
            PacketBufferMultiUnitHeap* pManagementHeap = new PacketBufferMultiUnitHeap();
            unsigned char* pManagementMemory = static_cast<unsigned char*>(m_pMemory) + uiPacketMemorySize;
            const unsigned int uiManagementMemorySize = GetRequiredMemorySizeImpl(aManagementParams);
            pManagementHeap->Initialize(pManagementMemory, uiManagementMemorySize, aManagementParams);
            m_pManagementObjectHeap = pManagementHeap;
        }
        m_bActive = true;
        m_pfnAllocate = PacketBufferAllocate;
        m_pfnAllocateManagementObject = PacketManagementObjectAllocate;
    }
    m_iInitializeCount = static_cast<int>(static_cast<unsigned int>(m_iInitializeCount) + 1u);
}

void PacketBufferManager::Destroy()
{
    ScopedCS lock(&s_criticalSection);

    if (m_iInitializeCount != 0 || !IsEnablePacketBufferPool() || !IsActive())
        return;

    RootTransport* pTransport = RootTransport::GetInstance();

    if (pTransport != 0)
        pTransport->ClearPacketInPacketBufferPool();

    Dump(TRACE_FILTER_PACKET_BUFFER_POOL);

    if (m_pManagementObjectHeap != 0)
    {
        m_pManagementObjectHeap->~PacketBufferMultiUnitHeap();
        if (m_pManagementObjectHeap != 0)
            RootObject::operator delete(m_pManagementObjectHeap);
        m_pManagementObjectHeap = 0;
    }

    if (m_pPacketBufferHeap != 0)
    {
        m_pPacketBufferHeap->~PacketBufferMultiUnitHeap();
        if (m_pPacketBufferHeap != 0)
            RootObject::operator delete(m_pPacketBufferHeap);
        m_pPacketBufferHeap = 0;
    }

    if (m_pMemory != 0)
        MemoryManager::Free(m_pMemory);

    m_pfnAllocate = 0;
    m_pfnAllocateManagementObject = 0;
    m_pMemory = 0;
    m_bActive = false;
}

void PacketBufferManager::Finalize()
{
    ScopedCS lock(&s_criticalSection);
    --m_iInitializeCount;
}

}
}
