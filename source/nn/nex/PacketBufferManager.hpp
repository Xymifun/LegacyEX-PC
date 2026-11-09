#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PACKETBUFFERMANAGER_HPP

#include "nn/nex/InstanceTable.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/PseudoSingleton.hpp"
#include "nn/nex/ThreadVariable.hpp"
#include "nn/nex/qVector.hpp"

namespace nn
{
namespace nex
{

class PacketBufferMultiUnitHeap;

class PacketBufferManager : public PseudoSingleton
{
public:
    enum
    {
        ClassID = 14,
        MaxUnitSize = 0x554,
        MinimumPoolSize = 0x10000
    };

    struct UnitHeapParam
    {
        unsigned int m_uiUnitSize;
        unsigned int m_uiUnitNum;

        bool operator<(const UnitHeapParam& rParam) const { return m_uiUnitSize < rParam.m_uiUnitSize; }
    };

    struct UnitHeapDebugInfo
    {
        unsigned int m_uiMaxUsedUnitNum;
        unsigned int m_uiUsedUnitNum;
        unsigned int m_uiUnitNum;
        unsigned int m_uiUnitSize;
        unsigned int m_uiBlockSize;
        unsigned int m_uiAllocNum;
        unsigned int m_uiFreeNum;
        unsigned int m_uiAllocFailedNum;
    };

    PacketBufferManager();
    virtual ~PacketBufferManager();

    static PacketBufferManager* GetInstance()
    {
        unsigned int contextId = g_usingInstantiationContext;
        if (g_usingInstantiationContext)
            contextId = g_instantiationContextVariable->GetRef();

        return static_cast<PacketBufferManager*>(g_instanceTable.FindInstance(ClassID, contextId));
    }

    static bool IsActive();
    static bool IsEnablePacketBufferPool();
    static bool EnablePacketBufferPool(bool bEnable);

    static bool CheckUnitHeapParam(const qVector<UnitHeapParam>& rParams);
    static unsigned int GetRequiredMemorySizeImpl(const qVector<UnitHeapParam>& rParams);
    static unsigned int GetRequiredMemorySize(const qVector<UnitHeapParam>& rParams);
    static void GetPacketManagementUnitHeapParam(qVector<UnitHeapParam>& rManagementParams,
                                                 const qVector<UnitHeapParam>& rParams);
    static bool GetSimpleUnitHeapParam(unsigned int uiPoolSize, unsigned int uiUnitSize,
                                       qVector<UnitHeapParam>& rParams);
    static void GetUnitHeapParam(qVector<UnitHeapParam>& rParams);
    static bool SetUnitHeapParam(const qVector<UnitHeapParam>& rParams);

    static void GetUnitHeapDebugInfo(qVector<UnitHeapDebugInfo>& rDebugInfo);
    static void GetAllUnitHeapDebugInfo(qVector<UnitHeapDebugInfo>& rDebugInfo);
    static void ResetUnitHeapDebugInfo();

    static void* PacketBufferAllocate(unsigned int uiSize);
    static void PacketBufferFree(void* pBlock);
    static void* PacketManagementObjectAllocate(unsigned int uiSize);
    static void PacketManagementObjectFree(void* pBlock);

    void Initialize();
    void Finalize();
    void Destroy();

    bool IsActiveNonStatic();
    bool IsClean();
    unsigned int GetUnitNum(unsigned int uiUnitSize, unsigned int uiUnitNum);
    bool IsAllocatable(unsigned int uiSize, unsigned int uiUnitNum);
    unsigned int GetMaxUnitSize();

    unsigned int GetUnitHeaderSize() const { return m_uiUnitHeaderSize; }

    unsigned int GetUnitAlignment() const { return m_uiUnitAlignment; }

    unsigned int GetProperUnitSize(unsigned int uiSize);

    void* Allocate(unsigned int uiSize);
    void* AllocateManagementObject(unsigned int uiSize);

    void Dump(unsigned long long uiFilter);

private:
    MemoryManager::Allocator m_pfnAllocate;
    MemoryManager::Allocator m_pfnAllocateManagementObject;
    void* m_pMemory;
    unsigned int m_uiMemorySize;
    int m_iInitializeCount;
    unsigned int m_uiUnitHeaderSize;
    unsigned int m_uiUnitAlignment;
    float m_fMemoryUsageRate;
    bool m_bActive;
    PacketBufferMultiUnitHeap* m_pPacketBufferHeap;
    PacketBufferMultiUnitHeap* m_pManagementObjectHeap;
};

}
}

#endif
